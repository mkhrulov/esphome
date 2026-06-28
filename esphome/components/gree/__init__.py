from dataclasses import dataclass, field

import esphome.codegen as cg
from esphome.core import CORE

gree_ns = cg.esphome_ns.namespace("gree")

DOMAIN = "gree"


@dataclass
class GreeData:
    model_by_id: dict = field(default_factory=dict)


def _get_data() -> GreeData:
    if DOMAIN not in CORE.data:
        CORE.data[DOMAIN] = GreeData()
    return CORE.data[DOMAIN]


def set_climate_model(climate_id, model: str) -> None:
    _get_data().model_by_id[str(climate_id)] = model


def get_climate_model(climate_id) -> str | None:
    return _get_data().model_by_id.get(str(climate_id))
