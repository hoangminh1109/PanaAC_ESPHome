import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate_ir, select
from esphome.const import CONF_ID

AUTO_LOAD = ['climate_ir','select']

panaac_ns = cg.esphome_ns.namespace('panaac')
PanaACClimate = panaac_ns.class_('PanaACClimate', climate_ir.ClimateIR)
PanaACFanLevel = panaac_ns.class_('PanaACFanLevel', select.Select, cg.Component)
PanaACSwingV = panaac_ns.class_('PanaACSwingV', select.Select, cg.Component)
PanaACSwingH = panaac_ns.class_('PanaACSwingH', select.Select, cg.Component)

CONF_SWING_HORIZONTAL = "swing_horizontal"
CONF_TEMP_STEP = "temp_step"
CONF_SUPPORT_QUIET = "supports_quiet"
CONF_FAN_5LEVEL = "fan_5level"

CONF_SWINGV_ID = "swingv_id"
CONF_SWINGH_ID = "swingh_id"
CONF_FANLEVEL_ID = "fanlevel_id"

CONFIG_SCHEMA = climate_ir.CLIMATE_IR_WITH_RECEIVER_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(PanaACClimate),
    cv.GenerateID(CONF_SWINGV_ID): cv.declare_id(PanaACSwingV),
    cv.GenerateID(CONF_SWINGH_ID): cv.declare_id(PanaACSwingH),
    cv.GenerateID(CONF_FANLEVEL_ID): cv.declare_id(PanaACFanLevel),
    cv.Optional(CONF_SWING_HORIZONTAL, default=False): cv.boolean,
    cv.Optional(CONF_TEMP_STEP, default=1.0): cv.float_,
    cv.Optional(CONF_SUPPORT_QUIET, default=False): cv.boolean,
    cv.Optional(CONF_FAN_5LEVEL, default=False): cv.boolean,
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await climate_ir.register_climate_ir(var, config)
    cg.add(var.set_swing_horizontal(config[CONF_SWING_HORIZONTAL]))
    cg.add(var.set_temp_step(config[CONF_TEMP_STEP]))
    cg.add(var.set_supports_quiet(config[CONF_SUPPORT_QUIET]))
    cg.add(var.set_fan_5level(config[CONF_FAN_5LEVEL]))

    # Fan level select
    fanlevel = cg.new_Pvariable(config[CONF_FANLEVEL_ID])
    cg.add(fanlevel.set_name("- Fan Level"))
    cg.add(fanlevel.set_object_id("fanlevel"))
    cg.add(fanlevel.set_internal(False))    
    cg.add(fanlevel.set_parent_climate(var))
    cg.add(cg.App.register_component(fanlevel))
    cg.add(cg.App.register_select(fanlevel))
    cg.add(var.set_fanlevel(fanlevel))
    
    # SwingV select
    swingv = cg.new_Pvariable(config[CONF_SWINGV_ID])
    cg.add(swingv.set_name("- Swing Vertical"))
    cg.add(swingv.set_object_id("swingv"))
    cg.add(swingv.set_internal(False))    
    cg.add(swingv.set_parent_climate(var))
    cg.add(cg.App.register_component(swingv))
    cg.add(cg.App.register_select(swingv))
    cg.add(var.set_swingv(swingv))

    # SwingH select
    swingh = cg.new_Pvariable(config[CONF_SWINGH_ID])
    cg.add(swingh.set_name("- Swing Horizontal"))
    cg.add(swingh.set_object_id("swingh"))
    cg.add(swingh.set_internal(False))    
    cg.add(swingh.set_parent_climate(var))
    cg.add(cg.App.register_component(swingh))
    cg.add(cg.App.register_select(swingh))
    cg.add(var.set_swingh(swingh))
