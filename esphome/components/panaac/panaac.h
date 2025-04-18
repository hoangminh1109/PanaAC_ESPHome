#pragma once

#include "definitions.h"
#include "extra.h"
#include <cinttypes>

namespace esphome
{
    namespace panaac
    {
        class PanaACClimate : public climate_ir::ClimateIR
        {
        public:
            PanaACClimate() : climate_ir::ClimateIR(
                                  PANAAC_TEMP_MIN, PANAAC_TEMP_MAX, 1.0f, true, true,
                                  {climate::CLIMATE_FAN_AUTO,
                                   climate::CLIMATE_FAN_LOW,
                                   climate::CLIMATE_FAN_MEDIUM,
                                   climate::CLIMATE_FAN_HIGH,
                                   climate::CLIMATE_FAN_QUIET},
                                  {climate::CLIMATE_SWING_OFF,
                                   climate::CLIMATE_SWING_BOTH,
                                   climate::CLIMATE_SWING_VERTICAL,
                                   climate::CLIMATE_SWING_HORIZONTAL},
                                  {})
                                  {}

            void set_swing_horizontal(bool swing_horizontal) { this->swing_horizontal_ = swing_horizontal; }
            void set_temp_step(float temp_step) { this->temp_step_ = temp_step; }
            void set_supports_quiet(bool supports_quiet) { this->supports_quiet_ = supports_quiet; }
            void set_fan_5level(bool fan_5level) { this->fan_5level_ = fan_5level; }

            void set_fanlevel(PanaACFanLevel *fanlevel) { this->fanlevel_ = fanlevel; }
            void set_swingv(PanaACSwingV *swingv) { this->swingv_ = swingv; }
            void set_swingh(PanaACSwingH *swingh) { this->swingh_ = swingh; }

            void update_state();
            void transmit_data();

            ClimateState ac_state;
            bool swing_horizontal_;

        protected:
            void setup() override;
            void transmit_state() override;
            bool on_receive(remote_base::RemoteReceiveData data) override;
            climate::ClimateTraits traits() override;

            bool decode_data(remote_base::RemoteReceiveData data, std::vector<uint8_t>& state_bytes);
            bool decode_state(std::vector<uint8_t> state_bytes, ClimateState& state);
            
            float temp_step_;
            bool supports_quiet_;
            bool fan_5level_;

            PanaACFanLevel *fanlevel_{nullptr};
            PanaACSwingV *swingv_{nullptr};
            PanaACSwingH *swingh_{nullptr};
        };


    } // namespace panaac
} // namespace esphome
