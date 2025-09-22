/*
 * Copyright 2025 Hoang Minh
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 #include "panaac.h"

namespace esphome
{
    namespace panaac
    {
        // ---------------- PanaACClimate -----------------------
        void PanaACClimate::setup()
        {
            ClimateIR::setup();

            // state
            ac_state.mode = climate::CLIMATE_MODE_OFF;
            ac_state.temp = 26.0;
            ac_state.fan_mode = climate::CLIMATE_FAN_AUTO;
            ac_state.fan_level = PANAAC_FAN_AUTO;
            ac_state.swing_mode = climate::CLIMATE_SWING_VERTICAL;
            ac_state.swing_v_pos = PANAAC_SWINGV_AUTO;
            ac_state.swing_h_pos = PANAAC_SWINGH_AUTO;
            ac_state.last_swing_v_pos = PANAAC_SWINGV_MIDDLE;
            ac_state.last_swing_h_pos = PANAAC_SWINGH_MIDDLE;

            // fan level options
            if (this->fan_5level_)
            {
                this->fanlevel_->traits.set_options({STR_FAN_AUTO, STR_FAN_L1, STR_FAN_L2, STR_FAN_L3, STR_FAN_L4, STR_FAN_L5});
            }
            else
            {
                this->fanlevel_->traits.set_options({STR_FAN_AUTO, STR_FAN_L1, STR_FAN_L3, STR_FAN_L5});
            }

            // support quiet
            if (this->supports_quiet_)
            {
                std::vector<std::string> fanlevel_options = this->fanlevel_->traits.get_options();
                fanlevel_options.push_back(STR_FAN_QUIET);
                this->fanlevel_->traits.set_options(fanlevel_options);
            }

            // swing v options
            this->swingv_->traits.set_options({STR_SWINGV_AUTO, STR_SWINGV_HIGHEST, STR_SWINGV_HIGH, STR_SWINGV_MIDDLE, STR_SWINGV_LOW, STR_SWINGV_LOWEST});

            if (this->swing_horizontal_)
            {
                this->swingh_->traits.set_options({STR_SWINGH_AUTO, STR_SWINGH_LEFTMAX, STR_SWINGH_LEFT, STR_SWINGH_MIDDLE, STR_SWINGH_RIGHT, STR_SWINGH_RIGHTMAX});
                this->swingh_->set_internal(false);
            }
            else
            {
                this->swingh_->traits.set_options({});
                this->swingh_->set_internal(true);
            }

            // initial state
            this->mode = climate::CLIMATE_MODE_OFF;
            this->target_temperature = 26.0;
            this->fan_mode = climate::CLIMATE_FAN_AUTO;
            if (this->swing_horizontal_)
            {
                this->swing_mode = climate::CLIMATE_SWING_BOTH;
            }
            else
            {
                this->swing_mode = climate::CLIMATE_SWING_VERTICAL;
            }

            transmit_state();

        }

        climate::ClimateTraits PanaACClimate::traits() {
            
            auto traits = climate::ClimateTraits();
            traits.set_supports_current_temperature(this->sensor_ != nullptr);
            traits.set_supports_action(false);
            traits.set_visual_min_temperature(PANAAC_TEMP_MIN);
            traits.set_visual_max_temperature(PANAAC_TEMP_MAX);
            traits.set_visual_temperature_step(this->temp_step_);
            traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_AUTO, climate::CLIMATE_MODE_DRY});
            
            if (this->supports_cool_)
                traits.add_supported_mode(climate::CLIMATE_MODE_COOL);
            if (this->supports_heat_)
                traits.add_supported_mode(climate::CLIMATE_MODE_HEAT);
            
            // Default to only 3 levels in ESPHome
            traits.set_supported_fan_modes(
                {   climate::CLIMATE_FAN_AUTO,
                    climate::CLIMATE_FAN_LOW,       // level 1
                    climate::CLIMATE_FAN_MEDIUM,    // level 3
                    climate::CLIMATE_FAN_HIGH       // level 5
                });

            if (this->supports_quiet_)
                traits.add_supported_fan_mode(climate::CLIMATE_FAN_QUIET);

            traits.set_supported_swing_modes({climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_VERTICAL});
            
            if (this->swing_horizontal_)
            {
                traits.add_supported_swing_mode(climate::CLIMATE_SWING_HORIZONTAL);
                traits.add_supported_swing_mode(climate::CLIMATE_SWING_BOTH);
            }
            
            return traits;
        }
        
        bool PanaACClimate::decode_data(remote_base::RemoteReceiveData data, std::vector<uint8_t>& state_bytes)
        {
            auto raw_data = data.get_raw_data();

            // process full frame or 2nd frame only, will ignore the fixed 1st frame
            if (raw_data.size() != 308 && raw_data.size() != 440)
            {
                return false;
            }
            
            if (!data.expect_item(PANAAC_HEADER_MARK, PANAAC_HEADER_SPACE))
            {
                ESP_LOGV(TAG, "Invalid data - expected header");
                return false;
            }

            state_bytes.clear();
            while (data.get_index() + 2 < raw_data.size())
            {
                uint8_t byte = 0;
                for (uint8_t a_bit = 0; a_bit < 8; a_bit++)
                {
                    if (data.expect_item(PANAAC_BIT_MARK, PANAAC_FRAME_END))
                    {
                        // expect new header if there are remain data
                        if (!data.expect_item(PANAAC_HEADER_MARK, PANAAC_HEADER_SPACE))
                        {
                            ESP_LOGV(TAG, "Invalid data - expected header at index = %d", data.get_index());
                            return false;
                        }
                    }
                    
                    // bit 1
                    if (data.expect_item(PANAAC_BIT_MARK, PANAAC_ONE_SPACE))
                    {
                        byte |= 1 << a_bit;
                    }
                    // bit 0
                    else if (data.expect_item(PANAAC_BIT_MARK, PANAAC_ZERO_SPACE))
                    {
                        // 0 already initialized, hence do nothingg here
                    }
                    else
                    {
                        ESP_LOGV(TAG, "Invalid bit %d of byte %d, index = %d", a_bit, state_bytes.size(), data.get_index());
                        return false;
                    }
                }
                state_bytes.push_back(byte);
            }

#if(DEBUG)
            std::string hex_str = "";
            for (uint8_t i = 0; i < state_bytes.size(); i++)
            {
                char buf[6];
                snprintf(buf, sizeof(buf), "%02X ", state_bytes[i]);
                hex_str += buf;
            }
            
            ESP_LOGV(TAG, "Full packet received: len = %d, data = [ %s]", state_bytes.size(), hex_str.c_str());
#endif

            // in case of full frame, just crop the first 8 fixed bytes
            if (state_bytes.size() == 27)
            {
                state_bytes.erase(state_bytes.begin(), state_bytes.begin() + 8);
            }

            return true;
            
        }
        
        bool PanaACClimate::decode_state(std::vector<uint8_t> state_bytes, ClimateState& ac_state)
        {
            // check length
            if (state_bytes.size() != 19) return false;
            
            // check protocol
            if ( !( state_bytes[0] == 0x02 &&
                    state_bytes[1] == 0x20 &&
                    state_bytes[2] == 0xE0 &&
                    state_bytes[3] == 0x04 &&
                    state_bytes[4] == 0x00 ) )
            {
                ESP_LOGV(TAG, "Invalid protocol");
                return false;
            }

            // verify checksum
            uint8_t checksum = 0;
            for (uint8_t i = 0; i < 18; i++) {
                checksum += state_bytes[i];
            }
            if (checksum != state_bytes[18])
            {
                ESP_LOGV(TAG, "Invalid checksum");
                return false;
            }
            
            // operation mode
            if ((state_bytes[PANAAC_BYTEPOS_POWER] & 0x0F) == PANAAC_POWER_OFF)
            {
                ac_state.mode = climate::CLIMATE_MODE_OFF;
            }
            else
            {
                switch (state_bytes[PANAAC_BYTEPOS_MODE] & 0xF0)
                {
                    case PANAAC_MODE_DRY:
                        ac_state.mode = climate::CLIMATE_MODE_DRY;
                        break;
                    case PANAAC_MODE_COOL:
                        ac_state.mode = climate::CLIMATE_MODE_COOL;
                        break;
                    case PANAAC_MODE_HEAT:
                        ac_state.mode = climate::CLIMATE_MODE_HEAT;
                        break;
                    case PANAAC_MODE_AUTO:
                    default:
                        ac_state.mode = climate::CLIMATE_MODE_AUTO;
                        break;
                }
            }
            
            // temperature
            ac_state.temp = ((state_bytes[PANAAC_BYTEPOS_TEMP] & 0x1E) >> 1) + PANAAC_TEMP_MIN;
            if ((state_bytes[PANAAC_BYTEPOS_TEMP] & 0x01) == 0x01)
            {
                ac_state.temp += 0.5;
            }
            
            // fan
            switch (state_bytes[PANAAC_BYTEPOS_FAN] & 0xF0)
            {
                case PANAAC_FAN_LEVEL_1:
                    ac_state.fan_mode = climate::CLIMATE_FAN_LOW;
                    ac_state.fan_level = PANAAC_FAN_LEVEL_1;
                    break;
                case PANAAC_FAN_LEVEL_2:
                    ac_state.fan_mode = climate::CLIMATE_FAN_LOW;
                    ac_state.fan_level = PANAAC_FAN_LEVEL_2;
                    break;
                case PANAAC_FAN_LEVEL_3:
                    ac_state.fan_mode = climate::CLIMATE_FAN_MEDIUM;
                    ac_state.fan_level = PANAAC_FAN_LEVEL_3;
                    break;
                case PANAAC_FAN_LEVEL_4:
                    ac_state.fan_mode = climate::CLIMATE_FAN_MEDIUM;
                    ac_state.fan_level = PANAAC_FAN_LEVEL_4;
                    break;
                case PANAAC_FAN_LEVEL_5:
                    ac_state.fan_mode = climate::CLIMATE_FAN_HIGH;
                    ac_state.fan_level = PANAAC_FAN_LEVEL_5;
                    break;
                case PANAAC_FAN_AUTO:
                default:
                    ac_state.fan_mode = climate::CLIMATE_FAN_AUTO;
                    ac_state.fan_level = PANAAC_FAN_AUTO;
                    break;
            }

            // quiet
            if (this->supports_quiet_)
            {
                if ((state_bytes[PANAAC_BYTEPOS_QUIET] & 0xF0) == PANAAC_FAN_QUIET)
                {
                    ac_state.fan_mode = climate::CLIMATE_FAN_QUIET;
                    ac_state.fan_level = PANAAC_FAN_QUIET;
                }
            }
            
            //swing
            uint8_t swing_v = state_bytes[PANAAC_BYTEPOS_SWINGV] & 0x0F;
            uint8_t swing_h = state_bytes[PANAAC_BYTEPOS_SWINGH] & 0x0F;
            
            ac_state.swing_v_pos = static_cast<SwingVPos>(swing_v);
            ac_state.swing_h_pos = static_cast<SwingHPos>(swing_h);
            
            if (!this->swing_horizontal_) swing_h = PANAAC_SWINGH_NONE;

            if (swing_v == PANAAC_SWINGV_AUTO && swing_h == PANAAC_SWINGH_AUTO)
            {
                ac_state.swing_mode = climate::CLIMATE_SWING_BOTH;
            }
            else if (swing_v == PANAAC_SWINGV_AUTO)
            {
                ac_state.swing_mode = climate::CLIMATE_SWING_VERTICAL;
            }
            else if (swing_h == PANAAC_SWINGH_AUTO)
            {
                ac_state.swing_mode = climate::CLIMATE_SWING_HORIZONTAL;
            }
            else
            {
                ac_state.swing_mode = climate::CLIMATE_SWING_OFF;
            }

            return true;
        }

        bool PanaACClimate::on_receive(remote_base::RemoteReceiveData data) {
            ESP_LOGV(TAG, "Received some bytes");

            auto raw_data = data.get_raw_data();

#if(DEBUG)
            for (uint32_t i = 0; i < raw_data.size(); i++)
            {
                ESP_LOGV(TAG, "Raw data index = %d, data = %d", i, raw_data[i]);
            }
#endif
            
            // process full frame or 2nd frame only, will ignore the fixed 1st frame
            if (raw_data.size() != 308 && raw_data.size() != 440)
            {
                if (raw_data.size() == 132) // fixed 1st frame
                {
                    ESP_LOGV(TAG, "Ignore fixed 1st frame");
                }
                ESP_LOGV(TAG, "Ir data length not expected");
                return false;
            }
            
            std::vector<uint8_t> state_bytes;
            if (!decode_data(data, state_bytes))
            {
                ESP_LOGV(TAG, "Decode ir data failed");
                return false;
            }

#if(DEBUG)            
            std::string hex_str = "";
            for (uint8_t i = 0; i < state_bytes.size(); i++)
            {
                char buf[6];
                snprintf(buf, sizeof(buf), "%02X ", state_bytes[i]);
                hex_str += buf;
            }
            
            ESP_LOGD(TAG, "Received Panasonic AC IR state: len = %d, data = [ %s]", state_bytes.size(), hex_str.c_str());
            ESP_LOGV(TAG, "Finished receiving");
#endif            
            
            if (!decode_state(state_bytes, ac_state))
            {
                ESP_LOGV(TAG, "Decode state failed");
                return false;
            }
            
            // receiving HEAT but doesn't support HEAT
            if (!this->supports_heat_ && ac_state.mode == climate::CLIMATE_MODE_HEAT)
            {
                ESP_LOGV(TAG, "Heat mode not supported");
                return false;
            }
            
            this->mode = ac_state.mode;
            this->target_temperature = ac_state.temp;
            this->fan_mode = ac_state.fan_mode;
            this->swing_mode = ac_state.swing_mode;
            this->publish_state();

            this->fanlevel_->set_fanlevel(ac_state.fan_level);
            this->swingv_->set_swingvpos(ac_state.swing_v_pos);
            if (this->swing_horizontal_)
            {
                this->swingh_->set_swinghpos(ac_state.swing_h_pos);
            }
            
            return true;
        }

        void PanaACClimate::transmit_data()
        {
            std::vector<uint8_t> first_frame =  {   0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x06 };
            std::vector<uint8_t> second_frame = {   0x02, 0x20, 0xE0, 0x04, 0x00, 0x00,
                                                    0x00, 0x80, 0x00, 0x00, 0x00, 0x0E,
                                                    0xE0, 0x00, 0x00, 0x89, 0x00, 0x00,
                                                    0x00  };

            // power & mode
            switch (ac_state.mode)
            {
                case climate::CLIMATE_MODE_COOL:
                    second_frame[PANAAC_BYTEPOS_POWER] |= PANAAC_POWER_ON;
                    second_frame[PANAAC_BYTEPOS_MODE]  |= PANAAC_MODE_COOL;
                    break;
                case climate::CLIMATE_MODE_HEAT:
                    second_frame[PANAAC_BYTEPOS_POWER] |= PANAAC_POWER_ON;
                    second_frame[PANAAC_BYTEPOS_MODE]  |= PANAAC_MODE_HEAT;
                    break;
                case climate::CLIMATE_MODE_DRY:
                    second_frame[PANAAC_BYTEPOS_POWER] |= PANAAC_POWER_ON;
                    second_frame[PANAAC_BYTEPOS_MODE]  |= PANAAC_MODE_DRY;
                    break;
                case climate::CLIMATE_MODE_AUTO:
                    second_frame[PANAAC_BYTEPOS_POWER] |= PANAAC_POWER_ON;
                    second_frame[PANAAC_BYTEPOS_MODE]  |= PANAAC_MODE_AUTO;
                    break;
                case climate::CLIMATE_MODE_OFF:
                default:
                    second_frame[PANAAC_BYTEPOS_POWER] |= PANAAC_POWER_OFF;
                    second_frame[PANAAC_BYTEPOS_MODE]  |= PANAAC_MODE_COOL;
            }

            // temperature
            uint8_t encoded_temp = static_cast<uint8_t>(ac_state.temp) - PANAAC_TEMP_MIN;
            encoded_temp &= 0x0F;
            second_frame[PANAAC_BYTEPOS_TEMP] = 0x20 | (encoded_temp << 1);
            
            if (static_cast<uint8_t>(ac_state.temp) < ac_state.temp) // if x.5 degree in some models
            {
                second_frame[PANAAC_BYTEPOS_TEMP] |= 0x01;
            }

            // fan
            switch (ac_state.fan_mode)
            {
                case climate::CLIMATE_FAN_LOW:
                    if (ac_state.fan_level != PANAAC_FAN_LEVEL_1 && ac_state.fan_level != PANAAC_FAN_LEVEL_2)
                        ac_state.fan_level = PANAAC_FAN_LEVEL_1;
                    second_frame[PANAAC_BYTEPOS_FAN] |= ac_state.fan_level;
                    break;
                case climate::CLIMATE_FAN_MEDIUM:
                    if (ac_state.fan_level != PANAAC_FAN_LEVEL_3 && ac_state.fan_level != PANAAC_FAN_LEVEL_4)
                        ac_state.fan_level = PANAAC_FAN_LEVEL_3;
                    second_frame[PANAAC_BYTEPOS_FAN] |= ac_state.fan_level;
                    break;
                case climate::CLIMATE_FAN_HIGH:
                    if (ac_state.fan_level != PANAAC_FAN_LEVEL_5)
                        ac_state.fan_level = PANAAC_FAN_LEVEL_5;
                    second_frame[PANAAC_BYTEPOS_FAN] |= ac_state.fan_level;
                    break;
                case climate::CLIMATE_FAN_QUIET:
                    if (this->supports_quiet_)
                    {
                        if (ac_state.fan_level != PANAAC_FAN_QUIET)
                            ac_state.fan_level = PANAAC_FAN_QUIET;
                        second_frame[PANAAC_BYTEPOS_QUIET] |= PANAAC_FAN_QUIET;
                        second_frame[PANAAC_BYTEPOS_FAN] |= ac_state.fan_level;
                    }
                    else
                    {
                        second_frame[PANAAC_BYTEPOS_FAN] |= PANAAC_FAN_AUTO;
                        ac_state.fan_mode = climate::CLIMATE_FAN_AUTO;
                        ac_state.fan_level = PANAAC_FAN_AUTO;
                    }
                    break;
                case climate::CLIMATE_FAN_AUTO:
                default:
                    if (ac_state.fan_level != PANAAC_FAN_AUTO)
                        ac_state.fan_level = PANAAC_FAN_AUTO;
                    second_frame[PANAAC_BYTEPOS_FAN] |= ac_state.fan_level;
            }

            // swing
            switch (ac_state.swing_mode)
            {
                case climate::CLIMATE_SWING_OFF:
                    ac_state.swing_v_pos = ac_state.last_swing_v_pos;
                    second_frame[PANAAC_BYTEPOS_SWINGV] |= ac_state.swing_v_pos;
                    if (this->swing_horizontal_)
                    {
                        ac_state.swing_h_pos = ac_state.last_swing_h_pos;
                        second_frame[PANAAC_BYTEPOS_SWINGH] |= ac_state.swing_h_pos;
                    }
                    break;
                case climate::CLIMATE_SWING_VERTICAL:
                    second_frame[PANAAC_BYTEPOS_SWINGV] |= PANAAC_SWINGV_AUTO;
                    ac_state.swing_v_pos = PANAAC_SWINGV_AUTO;
                    if (this->swing_horizontal_)
                    {
                        ac_state.swing_h_pos = ac_state.last_swing_h_pos;
                        second_frame[PANAAC_BYTEPOS_SWINGH] |= ac_state.swing_h_pos;
                    }
                    break;
                case climate::CLIMATE_SWING_HORIZONTAL:
                    ac_state.swing_v_pos = ac_state.last_swing_v_pos;
                    second_frame[PANAAC_BYTEPOS_SWINGV] |= ac_state.swing_v_pos;
                    if (this->swing_horizontal_)
                    {
                        second_frame[PANAAC_BYTEPOS_SWINGH] |= PANAAC_SWINGH_AUTO;
                        ac_state.swing_h_pos = PANAAC_SWINGH_AUTO;
                    }   
                    break;
                case climate::CLIMATE_SWING_BOTH:
                default:
                    second_frame[PANAAC_BYTEPOS_SWINGV] |= PANAAC_SWINGV_AUTO;
                    ac_state.swing_v_pos = PANAAC_SWINGV_AUTO;
                    if (this->swing_horizontal_)
                    {
                        second_frame[PANAAC_BYTEPOS_SWINGH] |= PANAAC_SWINGH_AUTO;
                        ac_state.swing_h_pos = PANAAC_SWINGH_AUTO;
                    }
            }

            // checksum
            for (uint8_t i = 0; i < 18; i++) {
                second_frame[18] += second_frame[i];
            }

#if(DEBUG)            
            std::string hex_str = "";
            for (uint8_t i = 0; i < second_frame.size(); i++)
            {
                char buf[6];
                snprintf(buf, sizeof(buf), "%02X ", second_frame[i]);
                hex_str += buf;
            }
            
            ESP_LOGD(TAG, "Sending Panasonic AC IR state: len = %d, data = [ %s]", second_frame.size(), hex_str.c_str());
#endif            

            auto transmit = this->transmitter_->transmit();
            auto *data = transmit.get_data();

            //set transmit frequency
            if (this->ir_control_)
            {
                data->set_carrier_frequency(PANAAC_IR_TRANSMIT_FREQ);
            }
          
            // First frame
            data->mark(PANAAC_HEADER_MARK);
            data->space(PANAAC_HEADER_SPACE);
            for (uint8_t i_byte = 0; i_byte < first_frame.size(); i_byte++)
            {
                for (uint8_t i_bit = 0; i_bit < 8; i_bit++)
                {
                    data->mark(PANAAC_BIT_MARK);
                    bool bit = first_frame[i_byte] & (1 << i_bit);
                    data->space(bit ? PANAAC_ONE_SPACE : PANAAC_ZERO_SPACE);
                }
            }
            data->mark(PANAAC_BIT_MARK);
            data->space(PANAAC_FRAME_END);

            // 2nd frame
            data->mark(PANAAC_HEADER_MARK);
            data->space(PANAAC_HEADER_SPACE);
            for (uint8_t i_byte = 0; i_byte < second_frame.size(); i_byte++)
            {
                for (uint8_t i_bit = 0; i_bit < 8; i_bit++)
                {
                    data->mark(PANAAC_BIT_MARK);
                    bool bit = second_frame[i_byte] & (1 << i_bit);
                    data->space(bit ? PANAAC_ONE_SPACE : PANAAC_ZERO_SPACE);
                }
            }
            data->mark(PANAAC_BIT_MARK);
            data->space(PANAAC_FRAME_END);
          
            // transmit
            transmit.perform();
        }
        
        void PanaACClimate::transmit_state() {
            // power & mode
            ac_state.mode = this->mode;

            // temperature
            ac_state.temp = this->target_temperature;

            // fan
            ac_state.fan_mode = this->fan_mode.value();
            switch (ac_state.fan_mode)
            {
                case climate::CLIMATE_FAN_LOW:
                    ac_state.fan_level = PANAAC_FAN_LEVEL_1;
                    break;
                case climate::CLIMATE_FAN_MEDIUM:
                    ac_state.fan_level = PANAAC_FAN_LEVEL_3;
                    break;
                case climate::CLIMATE_FAN_HIGH:
                    ac_state.fan_level = PANAAC_FAN_LEVEL_5;
                    break;
                case climate::CLIMATE_FAN_QUIET:
                    if (this->supports_quiet_)
                    {
                        ac_state.fan_level = PANAAC_FAN_QUIET;
                    }
                    else
                    {
                        ac_state.fan_mode = climate::CLIMATE_FAN_AUTO;
                        ac_state.fan_level = PANAAC_FAN_AUTO;
                    }
                    break;
                case climate::CLIMATE_FAN_AUTO:
                default:
                    ac_state.fan_level = PANAAC_FAN_AUTO;
            }

            // swing
            ac_state.swing_mode = this->swing_mode;
            switch (ac_state.swing_mode)
            {
                case climate::CLIMATE_SWING_OFF:
                    if (ac_state.swing_v_pos == PANAAC_SWINGV_AUTO)
                        ac_state.swing_v_pos = PANAAC_SWINGV_MIDDLE;
                    if (this->swing_horizontal_)
                    {
                        if (ac_state.swing_h_pos == PANAAC_SWINGH_AUTO)
                            ac_state.swing_h_pos = PANAAC_SWINGH_MIDDLE;
                    }
                    else
                    {
                        ac_state.swing_h_pos = PANAAC_SWINGH_NONE;
                    }
                    break;
                case climate::CLIMATE_SWING_VERTICAL:
                    ac_state.swing_v_pos = PANAAC_SWINGV_AUTO;
                    if (this->swing_horizontal_)
                    {
                        if (ac_state.swing_h_pos == PANAAC_SWINGH_AUTO)
                            ac_state.swing_h_pos = PANAAC_SWINGH_MIDDLE;
                    }
                    else
                    {
                        ac_state.swing_h_pos = PANAAC_SWINGH_NONE;
                    }
                    break;
                case climate::CLIMATE_SWING_HORIZONTAL:
                    if (ac_state.swing_v_pos == PANAAC_SWINGV_AUTO)
                        ac_state.swing_v_pos = PANAAC_SWINGV_MIDDLE;
                    if (this->swing_horizontal_)
                    {
                        ac_state.swing_h_pos = PANAAC_SWINGH_AUTO;
                    }
                    else
                    {
                        ac_state.swing_h_pos = PANAAC_SWINGH_NONE;
                        ac_state.swing_mode = climate::CLIMATE_SWING_OFF;
                    }
                    break;
                case climate::CLIMATE_SWING_BOTH:
                default:
                    ac_state.swing_v_pos = PANAAC_SWINGV_AUTO;
                    if (this->swing_horizontal_)
                    {
                        ac_state.swing_h_pos = PANAAC_SWINGH_AUTO;
                    }
                    else
                    {
                        ac_state.swing_h_pos = PANAAC_SWINGH_NONE;
                        ac_state.swing_mode = climate::CLIMATE_SWING_VERTICAL;
                    }
            }

            transmit_data();

            this->mode = ac_state.mode;
            this->target_temperature = ac_state.temp;
            this->fan_mode = ac_state.fan_mode;
            this->swing_mode = ac_state.swing_mode;
            this->publish_state();

            this->fanlevel_->set_fanlevel(ac_state.fan_level);
            this->swingv_->set_swingvpos(ac_state.swing_v_pos);
            if (this->swing_horizontal_)
            {
                this->swingh_->set_swinghpos(ac_state.swing_h_pos);
            }
        }

        void PanaACClimate::update_state()
        {
            // this->fan_mode = ac_state.fan_mode;
            this->mode = ac_state.mode;
            this->target_temperature = ac_state.temp;
            this->fan_mode = ac_state.fan_mode;
            this->swing_mode = ac_state.swing_mode;
            transmit_data();

            this->publish_state();

        }

    } // namespace panaac
} // namespace esphome
