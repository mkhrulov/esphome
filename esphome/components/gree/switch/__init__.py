import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import CONF_LIGHT, DEVICE_CLASS_SWITCH, ENTITY_CATEGORY_CONFIG
import esphome.final_validate as fv

from .. import get_climate_model, gree_ns
from ..climate import CONF_MODEL, GreeClimate

CODEOWNERS = ["@nagyrobi", "@mknjc"]

GreeModeBitSwitch = gree_ns.class_("GreeModeBitSwitch", switch.Switch, cg.Component)
GreeYb1faBoolSwitch = gree_ns.class_("GreeYb1faBoolSwitch", GreeModeBitSwitch)
Yb1faSwitchFeature = gree_ns.enum("Yb1faSwitchFeature")

CONF_TURBO = "turbo"
CONF_HEALTH = "health"
CONF_XFAN = "xfan"
CONF_SLEEP = "sleep"
CONF_GREE_ID = "gree_id"

STANDARD_MODELS = {"yan", "yaa", "yac", "yac1fb9"}
YB1FA_MODEL = "yb1fa"

# Standard model: turbo/light/health/xfan — mapped to bit masks in b[2] high nibble
STANDARD_SWITCH_CONFIGS = (
    (CONF_TURBO, "Gree Turbo Switch", 0x10, "mdi:car-turbocharger"),
    (CONF_LIGHT, "Gree Light Switch", 0x20, "mdi:led-outline"),
    (CONF_HEALTH, "Gree Health Switch", 0x40, "mdi:pine-tree"),
    (CONF_XFAN, "Gree X-FAN Switch", 0x80, "mdi:wall-sconce-flat"),
)

# YB1FA model: turbo/xfan/light/sleep — mapped to dedicated setters (health follows light)
YB1FA_SWITCH_CONFIGS = (
    (
        CONF_TURBO,
        "Gree YB1FA Turbo Switch",
        Yb1faSwitchFeature.TURBO,
        "mdi:car-turbocharger",
    ),
    (
        CONF_XFAN,
        "Gree YB1FA X-FAN Switch",
        Yb1faSwitchFeature.XFAN,
        "mdi:wall-sconce-flat",
    ),
    (
        CONF_LIGHT,
        "Gree YB1FA Light Switch",
        Yb1faSwitchFeature.LIGHT,
        "mdi:led-outline",
    ),
    (
        CONF_SLEEP,
        "Gree YB1FA Sleep Switch",
        Yb1faSwitchFeature.SLEEP,
        "mdi:weather-night",
    ),
)

_SWITCH_ICONS = {
    CONF_TURBO: "mdi:car-turbocharger",
    CONF_LIGHT: "mdi:led-outline",
    CONF_HEALTH: "mdi:pine-tree",
    CONF_XFAN: "mdi:wall-sconce-flat",
    CONF_SLEEP: "mdi:weather-night",
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_GREE_ID): cv.use_id(GreeClimate),
        **{
            cv.Optional(key): switch.switch_schema(
                GreeModeBitSwitch,
                icon=_SWITCH_ICONS[key],
                default_restore_mode="RESTORE_DEFAULT_OFF",
                device_class=DEVICE_CLASS_SWITCH,
                entity_category=ENTITY_CATEGORY_CONFIG,
            )
            for key in (CONF_TURBO, CONF_LIGHT, CONF_HEALTH, CONF_XFAN, CONF_SLEEP)
        },
    }
)


def _validate_model(config):
    full_config = fv.full_config.get()
    climate_path = full_config.get_path_for_id(config[CONF_GREE_ID])[:-1]
    climate_conf = full_config.get_config_for_path(climate_path)
    model = climate_conf[CONF_MODEL]

    if model in STANDARD_MODELS:
        if CONF_SLEEP in config:
            raise cv.Invalid(
                f"'sleep' switch is only supported for the {YB1FA_MODEL} model"
            )
    elif model == YB1FA_MODEL:
        if CONF_HEALTH in config:
            raise cv.Invalid(
                f"'health' switch is not supported for the {YB1FA_MODEL} model "
                "(health follows light automatically)"
            )
    else:
        raise cv.Invalid(
            "Gree switches are only supported for the "
            + ", ".join(sorted(STANDARD_MODELS) + [YB1FA_MODEL])
            + " models"
        )


FINAL_VALIDATE_SCHEMA = _validate_model


async def to_code(config):
    parent = await cg.get_variable(config[CONF_GREE_ID])
    model = get_climate_model(config[CONF_GREE_ID])

    if model == YB1FA_MODEL:
        for conf_key, name, feature, _ in YB1FA_SWITCH_CONFIGS:
            if switch_conf := config.get(conf_key):
                sw = cg.new_Pvariable(switch_conf[cv.CONF_ID], name, feature)
                await switch.register_switch(sw, switch_conf)
                await cg.register_component(sw, switch_conf)
                await cg.register_parented(sw, parent)
    else:
        for conf_key, name, bit_mask, _ in STANDARD_SWITCH_CONFIGS:
            if switch_conf := config.get(conf_key):
                sw = cg.new_Pvariable(switch_conf[cv.CONF_ID], name, bit_mask)
                await switch.register_switch(sw, switch_conf)
                await cg.register_component(sw, switch_conf)
                await cg.register_parented(sw, parent)
