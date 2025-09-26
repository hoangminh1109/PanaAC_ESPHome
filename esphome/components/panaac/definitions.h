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

#pragma once

#include "esphome/components/climate_ir/climate_ir.h"
#include "esphome/components/select/select.h"
#include "esphome/components/remote_base/remote_base.h"
#include "esphome/core/log.h"
#include <cstdlib>
#include <cinttypes>

namespace esphome
{
    namespace panaac
    {
        static const char *TAG = "panaac.climate";

        // Temperature
        static const uint8_t PANAAC_TEMP_MIN = 16; // Celsius
        static const uint8_t PANAAC_TEMP_MAX = 30; // Celsius

        // Pulse parameters in usec
        const uint16_t PANAAC_BIT_MARK = 550;
        const uint16_t PANAAC_ONE_SPACE = 1200;
        const uint16_t PANAAC_ZERO_SPACE = 350;
        const uint16_t PANAAC_HEADER_MARK = 3650;
        const uint16_t PANAAC_HEADER_SPACE = 1600;
        const uint16_t PANAAC_FRAME_END = 10000;

        // IR transmit frequency
        const uint16_t PANAAC_IR_TRANSMIT_FREQ = 38000;
        
        // byte position
        const uint8_t PANAAC_BYTEPOS_POWER = 5;
        const uint8_t PANAAC_BYTEPOS_MODE = 5;
        const uint8_t PANAAC_BYTEPOS_TEMP = 6;
        const uint8_t PANAAC_BYTEPOS_FAN = 8;
        const uint8_t PANAAC_BYTEPOS_SWINGV = 8;
        const uint8_t PANAAC_BYTEPOS_SWINGH = 9;
        const uint8_t PANAAC_BYTEPOS_QUIET = 13;
        
        // byte values
        const uint8_t PANAAC_POWER_OFF = 0x08;
        const uint8_t PANAAC_POWER_ON = 0x09;

        const uint8_t PANAAC_MODE_DRY = 0x20;
        const uint8_t PANAAC_MODE_COOL = 0x30;
        const uint8_t PANAAC_MODE_HEAT = 0x40;
        const uint8_t PANAAC_MODE_FAN_ONLY = 0x60;
        const uint8_t PANAAC_MODE_AUTO = 0x00;

        enum FanLevel {
            PANAAC_FAN_AUTO = 0xA0,
            PANAAC_FAN_LEVEL_1 = 0x30,
            PANAAC_FAN_LEVEL_2 = 0x40,
            PANAAC_FAN_LEVEL_3 = 0x50,
            PANAAC_FAN_LEVEL_4 = 0x60,
            PANAAC_FAN_LEVEL_5 = 0x70,
            PANAAC_FAN_QUIET = 0x20,
        };
        
        enum SwingVPos {
            PANAAC_SWINGV_AUTO = 0x0F,
            PANAAC_SWINGV_HIGHEST = 0x01,
            PANAAC_SWINGV_HIGH = 0x02,
            PANAAC_SWINGV_MIDDLE = 0x03,
            PANAAC_SWINGV_LOWEST = 0x05,
            PANAAC_SWINGV_LOW = 0x04,
        };

        enum SwingHPos {
            PANAAC_SWINGH_NONE = 0x00,
            PANAAC_SWINGH_MIDDLE = 0x06,
            PANAAC_SWINGH_LEFTMAX = 0x09,
            PANAAC_SWINGH_LEFT = 0x0A,
            PANAAC_SWINGH_RIGHT = 0x0B,
            PANAAC_SWINGH_RIGHTMAX = 0x0C,
            PANAAC_SWINGH_AUTO = 0x0D,
        };

        struct ClimateState {
            climate::ClimateMode mode;
            float temp;
            climate::ClimateFanMode fan_mode;
            FanLevel fan_level;
            climate::ClimateSwingMode swing_mode;
            SwingVPos swing_v_pos;
            SwingHPos swing_h_pos;
            SwingVPos last_swing_v_pos;
            SwingHPos last_swing_h_pos;
        };

        static const char *STR_FAN_AUTO = "Auto";
        static const char *STR_FAN_L1 = "Level 1";
        static const char *STR_FAN_L2 = "Level 2";
        static const char *STR_FAN_L3 = "Level 3";
        static const char *STR_FAN_L4 = "Level 4";
        static const char *STR_FAN_L5 = "Level 5";
        static const char *STR_FAN_QUIET = "Quiet";

        static const char *STR_SWINGV_AUTO = "Auto";
        static const char *STR_SWINGV_HIGHEST = "Highest";
        static const char *STR_SWINGV_HIGH = "High";
        static const char *STR_SWINGV_MIDDLE = "Middle";
        static const char *STR_SWINGV_LOW = "Low";
        static const char *STR_SWINGV_LOWEST = "Lowest";

        static const char *STR_SWINGH_AUTO = "Auto";
        static const char *STR_SWINGH_LEFTMAX = "Left Max";
        static const char *STR_SWINGH_LEFT = "Left";
        static const char *STR_SWINGH_MIDDLE = "Middle";
        static const char *STR_SWINGH_RIGHT = "Right";
        static const char *STR_SWINGH_RIGHTMAX = "Right Max";

        class PanaACClimate;

    } // namespace panaac
} // namespace esphome
