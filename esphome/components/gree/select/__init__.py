import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_CONFIG
import esphome.final_validate as fv

from .. import gree_ns
from ..climate import CONF_MODEL, GreeClimate

CODEOWNERS = ["@mknjc"]

GreeYb1faVaneSelect = gree_ns.class_("GreeYb1faVaneSelect", select.Select, cg.Component)
GreeYb1faTempDispSelect = gree_ns.class_(
    "GreeYb1faTempDispSelect", select.Select, cg.Component
)

CONF_VANE = "vane"
CONF_TEMP_DISPLAY = "temp_display"
CONF_GREE_ID = "gree_id"

# Options must match Yb1faVane enum order (0-9)
VANE_OPTIONS = [
    "Off",
    "Up",
    "Mid-Up",
    "Middle",
    "Mid-Down",
    "Down",
    "Swing (All)",
    "Swing (Bottom 3)",
    "Swing (Middle 3)",
    "Swing (Top 3)",
]

# Options must match Yb1faTempDisp enum order (0-3)
TEMPDISP_OPTIONS = ["Off", "House", "Inside", "Outside"]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_GREE_ID): cv.use_id(GreeClimate),
        cv.Optional(CONF_VANE): select.select_schema(
            GreeYb1faVaneSelect,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:angle-acute",
        ),
        cv.Optional(CONF_TEMP_DISPLAY): select.select_schema(
            GreeYb1faTempDispSelect,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:thermometer",
        ),
    }
)


def _validate_model(config):
    full_config = fv.full_config.get()
    climate_path = full_config.get_path_for_id(config[CONF_GREE_ID])[:-1]
    climate_conf = full_config.get_config_for_path(climate_path)
    if climate_conf[CONF_MODEL] != "yb1fa":
        raise cv.Invalid("Gree selects are only supported for the yb1fa model")


FINAL_VALIDATE_SCHEMA = _validate_model


async def to_code(config):
    parent = await cg.get_variable(config[CONF_GREE_ID])

    if vane_conf := config.get(CONF_VANE):
        var = cg.new_Pvariable(vane_conf[cv.CONF_ID])
        cg.add(var.traits.set_options(VANE_OPTIONS))
        await select.register_select(var, vane_conf, options=VANE_OPTIONS)
        await cg.register_component(var, vane_conf)
        await cg.register_parented(var, parent)

    if td_conf := config.get(CONF_TEMP_DISPLAY):
        var = cg.new_Pvariable(td_conf[cv.CONF_ID])
        cg.add(var.traits.set_options(TEMPDISP_OPTIONS))
        await select.register_select(var, td_conf, options=TEMPDISP_OPTIONS)
        await cg.register_component(var, td_conf)
        await cg.register_parented(var, parent)
