#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/gree/gree.h"

namespace esphome::gree {

class GreeModeBitSwitch final : public switch_::Switch, public Component, public Parented<GreeClimate> {
 public:
  GreeModeBitSwitch(const char *name, uint8_t bit_mask) : name_(name), bit_mask_(bit_mask) {}

  void setup() override;
  void dump_config() override;
  void write_state(bool state) override;

 protected:
  const char *name_;
  uint8_t bit_mask_;
};

// Feature types for YB1FA boolean switches
enum class Yb1faSwitchFeature : uint8_t { TURBO, XFAN, LIGHT, SLEEP };

// YB1FA switch — same schema type as GreeModeBitSwitch (subclass), different write_state.
class GreeYb1faBoolSwitch final : public GreeModeBitSwitch {
 public:
  GreeYb1faBoolSwitch(const char *name, Yb1faSwitchFeature feature) : GreeModeBitSwitch(name, 0), feature_(feature) {}

  void write_state(bool state) override;

 protected:
  Yb1faSwitchFeature feature_;
};

}  // namespace esphome::gree
