#pragma once

#include "esphome/core/component.h"
#include "esphome/components/select/select.h"
#include "esphome/components/gree/gree.h"

namespace esphome::gree {

class GreeYb1faVaneSelect final : public select::Select, public Component, public Parented<GreeClimate> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void control(const std::string &value) override;
};

class GreeYb1faTempDispSelect final : public select::Select, public Component, public Parented<GreeClimate> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void control(const std::string &value) override;
};

}  // namespace esphome::gree
