# Copyright 2025 Minh Hoang
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate_ir, select
from esphome.const import CONF_ID, CONF_NAME, CONF_DISABLED_BY_DEFAULT

AUTO_LOAD = ['climate_ir','select']

panaac_ns = cg.esphome_ns.namespace('panaac')
PanaACClimate = panaac_ns.class_('PanaACClimate', climate_ir.ClimateIR)
PanaACFanLevel = panaac_ns.class_('PanaACFanLevel', select.Select, cg.Component)
PanaACSwingV = panaac_ns.class_('PanaACSwingV', select.Select, cg.Component)
PanaACSwingH = panaac_ns.class_('PanaACSwingH', select.Select, cg.Component)

CONF_SUPPORT_FAN_ONLY = "supports_fan_only"
CONF_SWING_HORIZONTAL = "swing_horizontal"
CONF_TEMP_STEP = "temp_step"
CONF_SUPPORT_QUIET = "supports_quiet"
CONF_FAN_5LEVEL = "fan_5level"
CONF_IR_CONTROL = "ir_control"

CONF_SWINGV_ID = "swingv_id"
CONF_SWINGH_ID = "swingh_id"
CONF_FANLEVEL_ID = "fanlevel_id"

CONFIG_SCHEMA = climate_ir.climate_ir_with_receiver_schema(PanaACClimate).extend({
    cv.GenerateID(): cv.declare_id(PanaACClimate),
    cv.GenerateID(CONF_SWINGV_ID): cv.declare_id(PanaACSwingV),
    cv.GenerateID(CONF_SWINGH_ID): cv.declare_id(PanaACSwingH),
    cv.GenerateID(CONF_FANLEVEL_ID): cv.declare_id(PanaACFanLevel),
    cv.Optional(CONF_SWING_HORIZONTAL, default=False): cv.boolean,
    cv.Optional(CONF_TEMP_STEP, default=1.0): cv.float_,
    cv.Optional(CONF_SUPPORT_QUIET, default=False): cv.boolean,
    cv.Optional(CONF_SUPPORT_FAN_ONLY, default=False): cv.boolean,
    cv.Optional(CONF_FAN_5LEVEL, default=False): cv.boolean,
    cv.Optional(CONF_IR_CONTROL, default=False): cv.boolean,
})

async def to_code(config):
    var = await climate_ir.new_climate_ir(config)
    # var = cg.new_Pvariable(config[CONF_ID])
    # await climate_ir.register_climate_ir(var, config)
    cg.add(var.set_swing_horizontal(config[CONF_SWING_HORIZONTAL]))
    cg.add(var.set_temp_step(config[CONF_TEMP_STEP]))
    cg.add(var.set_supports_fan_only(config[CONF_SUPPORT_FAN_ONLY]))
    cg.add(var.set_supports_quiet(config[CONF_SUPPORT_QUIET]))
    cg.add(var.set_fan_5level(config[CONF_FAN_5LEVEL]))
    cg.add(var.set_ir_control(config[CONF_IR_CONTROL]))

    # Fan level select
    fanlevel_default_config = { CONF_ID: config[CONF_FANLEVEL_ID],
                                CONF_NAME: "- Fan Level",
                                CONF_DISABLED_BY_DEFAULT: False}
    fanlevel = cg.new_Pvariable(config[CONF_FANLEVEL_ID])
    await select.register_select(fanlevel, fanlevel_default_config, options=[])
    await cg.register_component(fanlevel, fanlevel_default_config)
    cg.add(fanlevel.set_parent_climate(var))
    cg.add(var.set_fanlevel(fanlevel))
    
    # SwingV select
    swingv_default_config = {   CONF_ID: config[CONF_SWINGV_ID],
                                CONF_NAME: "- Swing Vertical",
                                CONF_DISABLED_BY_DEFAULT: False}
    swingv = cg.new_Pvariable(config[CONF_SWINGV_ID])
    await select.register_select(swingv, swingv_default_config, options=[])
    await cg.register_component(swingv, swingv_default_config)
    cg.add(swingv.set_parent_climate(var))
    cg.add(var.set_swingv(swingv))

    # SwingH select
    swingh_default_config = {   CONF_ID: config[CONF_SWINGH_ID],
                                CONF_NAME: "- Swing Horizontal",
                                CONF_DISABLED_BY_DEFAULT: False}
    swingh = cg.new_Pvariable(config[CONF_SWINGH_ID])
    await select.register_select(swingh, swingh_default_config, options=[])
    await cg.register_component(swingh, swingh_default_config)
    cg.add(swingh.set_parent_climate(var))
    cg.add(var.set_swingh(swingh))
