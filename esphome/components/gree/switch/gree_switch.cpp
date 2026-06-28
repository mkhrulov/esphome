#include "gree_switch.h"
#include "esphome/core/log.h"

namespace esphome::gree {

static const char *const TAG = "gree.switch";

void GreeModeBitSwitch::setup() {
  auto initial = this->get_initial_state_with_restore_mode();
  if (initial.has_value()) {
    this->write_state(*initial);
  }
}

void GreeModeBitSwitch::dump_config() { log_switch(TAG, "  ", this->name_, this); }

void GreeModeBitSwitch::write_state(bool state) {
  this->parent_->set_mode_bit(this->bit_mask_, state);
  this->publish_state(state);
}

void GreeYb1faBoolSwitch::write_state(bool state) {
  switch (this->feature_) {
    case Yb1faSwitchFeature::TURBO:
      this->parent_->yb1fa_set_turbo(state);
      break;
    case Yb1faSwitchFeature::XFAN:
      this->parent_->yb1fa_set_xfan(state);
      break;
    case Yb1faSwitchFeature::LIGHT:
      this->parent_->yb1fa_set_light(state);
      break;
    case Yb1faSwitchFeature::SLEEP:
      this->parent_->yb1fa_set_sleep(state);
      break;
  }
  this->publish_state(state);
}

}  // namespace esphome::gree
