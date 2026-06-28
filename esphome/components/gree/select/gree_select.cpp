#include "gree_select.h"
#include "esphome/core/log.h"

namespace esphome::gree {

static const char *const TAG = "gree.select";

// ── Vane select ──────────────────────────────────────────────────────────────

void GreeYb1faVaneSelect::setup() {
  const auto &opts = this->traits.get_options();
  uint8_t idx = static_cast<uint8_t>(this->parent_->yb1fa_get_vane());
  if (idx < opts.size()) {
    this->publish_state(opts[idx]);
  }
}

void GreeYb1faVaneSelect::dump_config() { LOG_SELECT("", "Gree YB1FA Vane Select", this); }

void GreeYb1faVaneSelect::control(const std::string &value) {
  if (auto idx = this->index_of(value)) {
    this->parent_->yb1fa_set_vane(static_cast<Yb1faVane>(idx.value()));
  }
  this->publish_state(value);
}

// ── TempDisp select ──────────────────────────────────────────────────────────

void GreeYb1faTempDispSelect::setup() {
  const auto &opts = this->traits.get_options();
  uint8_t idx = static_cast<uint8_t>(this->parent_->yb1fa_get_tempdisp());
  if (idx < opts.size()) {
    this->publish_state(opts[idx]);
  }
}

void GreeYb1faTempDispSelect::dump_config() { LOG_SELECT("", "Gree YB1FA Temp Display Select", this); }

void GreeYb1faTempDispSelect::control(const std::string &value) {
  if (auto idx = this->index_of(value)) {
    this->parent_->yb1fa_set_tempdisp(static_cast<Yb1faTempDisp>(idx.value()));
  }
  this->publish_state(value);
}

}  // namespace esphome::gree
