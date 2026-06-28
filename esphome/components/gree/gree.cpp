#include "gree.h"
#include "esphome/components/remote_base/remote_base.h"
#include "esphome/core/log.h"

namespace esphome::gree {

static const char *const TAG = "gree.climate";

climate::ClimateTraits GreeClimate::traits() {
  auto t = climate_ir::ClimateIR::traits();
  // ClimateIR unconditionally includes HEAT_COOL in the base mode set; remove it when heat is not supported.
  if (!this->supports_heat_) {
    auto modes = t.get_supported_modes();
    modes.erase(climate::CLIMATE_MODE_HEAT_COOL);
    t.set_supported_modes(modes);
  }
  return t;
}

void GreeClimate::set_model(Model model) {
  if (model == GREE_YAN) {
    // YAN only has a vertical vane; horizontal swing IR bytes are not defined for this model.
    this->swing_modes_.erase(climate::CLIMATE_SWING_HORIZONTAL);
    this->swing_modes_.erase(climate::CLIMATE_SWING_BOTH);
  }
  if (model == GREE_YX1FF) {
    this->fan_modes_.insert(climate::CLIMATE_FAN_QUIET);
    this->presets_.insert(climate::CLIMATE_PRESET_NONE);
    this->presets_.insert(climate::CLIMATE_PRESET_SLEEP);
  }
  if (model == GREE_YB1FA) {
    // YB1FA has vertical swing only (no horizontal vanes on C&H units).
    this->swing_modes_.erase(climate::CLIMATE_SWING_HORIZONTAL);
    this->swing_modes_.erase(climate::CLIMATE_SWING_BOTH);
  }
  this->model_ = model;
}

void GreeClimate::set_mode_bit(uint8_t bit_mask, bool enabled) {
  if (enabled) {
    this->mode_bits_ |= bit_mask;
  } else {
    this->mode_bits_ &= ~bit_mask;
  }
  this->transmit_state();
}

// ── YB1FA setters ────────────────────────────────────────────────────────────

void GreeClimate::yb1fa_set_turbo(bool on) {
  this->yb1fa_turbo_ = on;
  this->transmit_state();
}
void GreeClimate::yb1fa_set_xfan(bool on) {
  this->yb1fa_xfan_ = on;
  this->transmit_state();
}
void GreeClimate::yb1fa_set_light(bool on) {
  this->yb1fa_light_ = on;
  this->transmit_state();
}
void GreeClimate::yb1fa_set_sleep(bool on) {
  this->yb1fa_sleep_ = on;
  this->transmit_state();
}

void GreeClimate::yb1fa_set_vane(Yb1faVane pos) {
  this->yb1fa_vane_ = pos;
  this->transmit_state();
}

void GreeClimate::yb1fa_set_tempdisp(Yb1faTempDisp mode) {
  this->yb1fa_tempdisp_ = mode;
  this->transmit_state();
}

// ── Transmit ─────────────────────────────────────────────────────────────────

void GreeClimate::transmit_state() {
  if (this->model_ == GREE_YB1FA) {
    this->transmit_yb1fa_();
    return;
  }

  uint8_t remote_state[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00};

  remote_state[0] = this->fan_speed_() | this->operation_mode_();
  remote_state[1] = this->temperature_();

  if (this->model_ == GREE_YAN) {
    remote_state[2] = 0x20;
    remote_state[3] = 0x50;
    remote_state[4] = this->vertical_swing_();
  }

  if (this->model_ == GREE_YX1FF || this->model_ == GREE_YAG) {
    remote_state[2] = 0x60;
    remote_state[3] = 0x50;
    remote_state[4] = this->vertical_swing_();
  }

  if (this->model_ == GREE_YAG) {
    remote_state[5] = 0x40;
    if (this->vertical_swing_() == GREE_VDIR_SWING || this->horizontal_swing_() == GREE_HDIR_SWING)
      remote_state[0] |= (1 << 6);
  }

  if (this->model_ == GREE_YAC || this->model_ == GREE_YAG)
    remote_state[4] |= (this->horizontal_swing_() << 4);

  if (this->model_ == GREE_YAA || this->model_ == GREE_YAC || this->model_ == GREE_YAC1FB9) {
    remote_state[2] = 0x20;  // bits 0..3 always 0000, bits 4..7 TURBO, LIGHT, HEALTH, X-FAN
    remote_state[3] = 0x50;  // bits 4..7 always 0101
    remote_state[6] = 0x20;  // YAA1FB, FAA1FB1, YB1F2 bits 4..7 always 0010

    if (this->vertical_swing_() == GREE_VDIR_SWING) {
      remote_state[0] |= (1 << 6);
    } else if (this->vertical_swing_() != GREE_VDIR_AUTO) {
      remote_state[5] = this->vertical_swing_();
    }
  }

  if (this->model_ == GREE_YAN || this->model_ == GREE_YAA || this->model_ == GREE_YAC ||
      this->model_ == GREE_YAC1FB9) {
    remote_state[2] = (remote_state[2] & 0x0F) | this->mode_bits_;
  }

  if (this->model_ == GREE_YX1FF) {
    if (this->fan_speed_() == GREE_FAN_TURBO)
      remote_state[2] |= GREE_FAN_TURBO_BIT;
    if (this->preset_() == GREE_PRESET_SLEEP)
      remote_state[0] |= GREE_PRESET_SLEEP_BIT;
  }

  // Calculate the checksum
  if (this->model_ == GREE_YAN || this->model_ == GREE_YX1FF) {
    remote_state[7] = ((remote_state[0] << 4) + (remote_state[1] << 4) + 0xC0);
  } else {
    remote_state[7] =
        ((((remote_state[0] & 0x0F) + (remote_state[1] & 0x0F) + (remote_state[2] & 0x0F) + (remote_state[3] & 0x0F) +
           ((remote_state[4] & 0xF0) >> 4) + ((remote_state[5] & 0xF0) >> 4) + ((remote_state[6] & 0xF0) >> 4) + 0x0A) &
          0x0F)
         << 4);
  }

  this->send_ir_(remote_state, /*yac1fb9_timing=*/this->model_ == GREE_YAC1FB9);
}

// ── YB1FA-specific transmit ──────────────────────────────────────────────────

void GreeClimate::transmit_yb1fa_() {
  uint8_t s[8] = {0x00, 0x00, YB1FA_B2_DEFAULT, YB1FA_B3, YB1FA_VANE_OFF, 0x00, YB1FA_B6, 0x00};

  // b[0]: sleep | swing | fan | power | mode
  s[0] = this->fan_speed_() | this->operation_mode_();
  if (this->yb1fa_sleep_)
    s[0] |= YB1FA_SLEEP_BIT;

  // b[1]: temperature (temp - 16); 0 in Auto mode since AC manages temperature
  if (this->mode != climate::CLIMATE_MODE_HEAT_COOL)
    s[1] = this->temperature_() - GREE_TEMP_MIN;

  // b[2]: xfan | light | health | turbo  (health follows light: both on or both off)
  s[2] = 0x00;
  if (this->yb1fa_xfan_)
    s[2] |= YB1FA_B2_XFAN;
  if (this->yb1fa_light_)
    s[2] |= YB1FA_B2_LIGHT | YB1FA_B2_HEALTH;
  if (this->yb1fa_turbo_)
    s[2] |= YB1FA_B2_TURBO;

  // b[4] + b[0] bit6: vane position
  switch (this->yb1fa_vane_) {
    case YB1FA_VANE_SEL_OFF:
      s[4] = YB1FA_VANE_OFF;
      break;
    case YB1FA_VANE_SEL_UP:
      s[4] = YB1FA_VANE_POS1_UP;
      break;
    case YB1FA_VANE_SEL_MIDUP:
      s[4] = YB1FA_VANE_POS2_MIDUP;
      break;
    case YB1FA_VANE_SEL_MIDDLE:
      s[4] = YB1FA_VANE_POS3_MID;
      break;
    case YB1FA_VANE_SEL_MIDDOWN:
      s[4] = YB1FA_VANE_POS4_MIDDN;
      break;
    case YB1FA_VANE_SEL_DOWN:
      s[4] = YB1FA_VANE_POS5_DOWN;
      break;
    case YB1FA_VANE_SEL_SWING:
      s[4] = YB1FA_VANE_SWING_ALL;
      s[0] |= YB1FA_SWING_BIT;
      break;
    case YB1FA_VANE_SEL_SWING_B3:
      s[4] = YB1FA_VANE_SWING_BOT3;
      s[0] |= YB1FA_SWING_BIT;
      break;
    case YB1FA_VANE_SEL_SWING_M3:
      s[4] = YB1FA_VANE_SWING_MID3;
      s[0] |= YB1FA_SWING_BIT;
      break;
    case YB1FA_VANE_SEL_SWING_T3:
      s[4] = YB1FA_VANE_SWING_TOP3;
      s[0] |= YB1FA_SWING_BIT;
      break;
  }

  // b[5]: temperature display mode
  switch (this->yb1fa_tempdisp_) {
    case YB1FA_TEMPDISP_SEL_HOUSE:
      s[5] = YB1FA_TEMPDISP_HOUSE;
      break;
    case YB1FA_TEMPDISP_SEL_INSIDE:
      s[5] = YB1FA_TEMPDISP_INSIDE;
      break;
    case YB1FA_TEMPDISP_SEL_OUTSIDE:
      s[5] = YB1FA_TEMPDISP_OUTSIDE;
      break;
    default:
      s[5] = YB1FA_TEMPDISP_OFF;
      break;
  }

  // b[7]: checksum = ((b[0]+b[1]) % 2) << 7
  s[7] = static_cast<uint8_t>(((s[0] + s[1]) & 1) << 7);

  ESP_LOGD(TAG, "YB1FA TX: %02X %02X %02X %02X %02X %02X %02X %02X", s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);

  this->send_ir_(s, /*yac1fb9_timing=*/true);
}

// ── Shared IR send helper ────────────────────────────────────────────────────

void GreeClimate::send_ir_(const uint8_t remote_state[GREE_STATE_FRAME_SIZE], bool yac1fb9_timing) {
  auto transmit = this->transmitter_->transmit();
  auto *data = transmit.get_data();
  data->set_carrier_frequency(GREE_IR_FREQUENCY);

  data->mark(GREE_HEADER_MARK);
  data->space(yac1fb9_timing ? GREE_YAC1FB9_HEADER_SPACE : GREE_HEADER_SPACE);

  for (int i = 0; i < 4; i++) {
    for (uint8_t mask = 1; mask > 0; mask <<= 1) {
      data->mark(GREE_BIT_MARK);
      data->space((remote_state[i] & mask) ? GREE_ONE_SPACE : GREE_ZERO_SPACE);
    }
  }

  // Footer: 0-bit, 1-bit, 0-bit
  data->mark(GREE_BIT_MARK);
  data->space(GREE_ZERO_SPACE);
  data->mark(GREE_BIT_MARK);
  data->space(GREE_ONE_SPACE);
  data->mark(GREE_BIT_MARK);
  data->space(yac1fb9_timing ? GREE_YAC1FB9_MESSAGE_SPACE : GREE_MESSAGE_SPACE);

  for (int i = 4; i < 8; i++) {
    for (uint8_t mask = 1; mask > 0; mask <<= 1) {
      data->mark(GREE_BIT_MARK);
      data->space((remote_state[i] & mask) ? GREE_ONE_SPACE : GREE_ZERO_SPACE);
    }
  }

  data->mark(GREE_BIT_MARK);
  data->space(0);
  transmit.perform();
}

// ── Field encoders ───────────────────────────────────────────────────────────

uint8_t GreeClimate::operation_mode_() {
  uint8_t operating_mode = GREE_MODE_ON;

  switch (this->mode) {
    case climate::CLIMATE_MODE_COOL:
      operating_mode |= GREE_MODE_COOL;
      break;
    case climate::CLIMATE_MODE_DRY:
      operating_mode |= GREE_MODE_DRY;
      break;
    case climate::CLIMATE_MODE_HEAT:
      operating_mode |= GREE_MODE_HEAT;
      break;
    case climate::CLIMATE_MODE_HEAT_COOL:
      operating_mode |= GREE_MODE_AUTO;
      break;
    case climate::CLIMATE_MODE_FAN_ONLY:
      operating_mode |= GREE_MODE_FAN;
      break;
    case climate::CLIMATE_MODE_OFF:
    default:
      operating_mode = GREE_MODE_OFF;
      break;
  }

  return operating_mode;
}

uint8_t GreeClimate::fan_speed_() {
  // YX1FF has 4 fan speeds -- we treat low as quiet and turbo as high
  if (this->model_ == GREE_YX1FF) {
    switch (this->fan_mode.value_or(climate::CLIMATE_FAN_ON)) {
      case climate::CLIMATE_FAN_QUIET:
        return GREE_FAN_1;
      case climate::CLIMATE_FAN_LOW:
        return GREE_FAN_2;
      case climate::CLIMATE_FAN_MEDIUM:
        return GREE_FAN_3;
      case climate::CLIMATE_FAN_HIGH:
        return GREE_FAN_TURBO;
      case climate::CLIMATE_FAN_AUTO:
      default:
        return GREE_FAN_AUTO;
    }
  }

  switch (this->fan_mode.value_or(climate::CLIMATE_FAN_ON)) {
    case climate::CLIMATE_FAN_LOW:
      return GREE_FAN_1;
    case climate::CLIMATE_FAN_MEDIUM:
      return GREE_FAN_2;
    case climate::CLIMATE_FAN_HIGH:
      return GREE_FAN_3;
    case climate::CLIMATE_FAN_AUTO:
    default:
      return GREE_FAN_AUTO;
  }
}

uint8_t GreeClimate::horizontal_swing_() {
  switch (this->swing_mode) {
    case climate::CLIMATE_SWING_HORIZONTAL:
    case climate::CLIMATE_SWING_BOTH:
      return GREE_HDIR_SWING;
    default:
      return GREE_HDIR_MANUAL;
  }
}

uint8_t GreeClimate::vertical_swing_() {
  switch (this->swing_mode) {
    case climate::CLIMATE_SWING_VERTICAL:
    case climate::CLIMATE_SWING_BOTH:
      return GREE_VDIR_SWING;
    default:
      return GREE_VDIR_MANUAL;
  }
}

uint8_t GreeClimate::temperature_() {
  return (uint8_t) roundf(clamp<float>(this->target_temperature, GREE_TEMP_MIN, GREE_TEMP_MAX));
}

uint8_t GreeClimate::preset_() {
  // YX1FF has sleep preset
  if (this->model_ == GREE_YX1FF) {
    switch (this->preset.value_or(climate::CLIMATE_PRESET_NONE)) {
      case climate::CLIMATE_PRESET_NONE:
        return GREE_PRESET_NONE;
      case climate::CLIMATE_PRESET_SLEEP:
        return GREE_PRESET_SLEEP;
      default:
        return GREE_PRESET_NONE;
    }
  }

  return GREE_PRESET_NONE;
}

// ── IR Receiver ──────────────────────────────────────────────────────────────

bool GreeClimate::on_receive(remote_base::RemoteReceiveData data) {
  if (data.size() < 73)
    return false;

  // Try YAC1FB9/YB1FA header space first, then standard
  if (!data.expect_item(GREE_HEADER_MARK, GREE_YAC1FB9_HEADER_SPACE, GREE_IR_TOLERANCE)) {
    data.reset();
    if (!data.expect_item(GREE_HEADER_MARK, GREE_HEADER_SPACE, GREE_IR_TOLERANCE))
      return false;
  }

  uint8_t frame[GREE_STATE_FRAME_SIZE] = {};

  // Block 1 (bytes 0-3)
  for (int b = 0; b < 4; b++) {
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (!data.expect_mark(GREE_BIT_MARK, GREE_IR_TOLERANCE))
        return false;
      if (data.expect_space(GREE_ONE_SPACE, GREE_IR_TOLERANCE))
        frame[b] |= (1 << bit);
      else if (!data.expect_space(GREE_ZERO_SPACE, GREE_IR_TOLERANCE))
        return false;
    }
  }

  // Footer (0, 1, 0) + inter-block gap
  if (!data.expect_item(GREE_BIT_MARK, GREE_ZERO_SPACE, GREE_IR_TOLERANCE))
    return false;
  if (!data.expect_item(GREE_BIT_MARK, GREE_ONE_SPACE, GREE_IR_TOLERANCE))
    return false;
  if (!data.expect_mark(GREE_BIT_MARK, GREE_IR_TOLERANCE))
    return false;
  if (!data.expect_space(GREE_YAC1FB9_MESSAGE_SPACE, GREE_IR_TOLERANCE)) {
    if (!data.expect_space(GREE_MESSAGE_SPACE, GREE_IR_TOLERANCE))
      return false;
  }

  // Block 2 (bytes 4-7)
  for (int b = 4; b < 8; b++) {
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (!data.expect_mark(GREE_BIT_MARK, GREE_IR_TOLERANCE))
        return false;
      if (data.expect_space(GREE_ONE_SPACE, GREE_IR_TOLERANCE))
        frame[b] |= (1 << bit);
      else if (!data.expect_space(GREE_ZERO_SPACE, GREE_IR_TOLERANCE))
        return false;
    }
  }
  if (!data.expect_mark(GREE_BIT_MARK, GREE_IR_TOLERANCE))
    return false;

  ESP_LOGD(TAG, "on_receive: %02X %02X %02X %02X | %02X %02X %02X %02X", frame[0], frame[1], frame[2], frame[3],
           frame[4], frame[5], frame[6], frame[7]);

  if (this->model_ == GREE_YB1FA)
    return this->parse_yb1fa_frame_(frame);
  return this->parse_state_frame_(frame);
}

// ── YB1FA frame parser ───────────────────────────────────────────────────────

bool GreeClimate::parse_yb1fa_frame_(const uint8_t f[GREE_STATE_FRAME_SIZE]) {
  // Checksum: b[7] bit7 = (b[0]+b[1]) % 2
  uint8_t expected_cs = static_cast<uint8_t>(((f[0] + f[1]) & 1) << 7);
  if ((f[7] & 0x80) != expected_cs) {
    ESP_LOGD(TAG, "YB1FA checksum fail: got 0x%02X expected bit7=0x%02X", f[7], expected_cs);
    return false;
  }

  // Power / mode
  if (!(f[0] & GREE_MODE_ON)) {
    this->mode = climate::CLIMATE_MODE_OFF;
  } else {
    switch (f[0] & 0x07) {
      case GREE_MODE_COOL:
        this->mode = climate::CLIMATE_MODE_COOL;
        break;
      case GREE_MODE_DRY:
        this->mode = climate::CLIMATE_MODE_DRY;
        break;
      case GREE_MODE_HEAT:
        this->mode = climate::CLIMATE_MODE_HEAT;
        break;
      case GREE_MODE_AUTO:
        this->mode = climate::CLIMATE_MODE_HEAT_COOL;
        break;
      case GREE_MODE_FAN:
        this->mode = climate::CLIMATE_MODE_FAN_ONLY;
        break;
      default:
        this->mode = climate::CLIMATE_MODE_COOL;
        break;
    }
  }

  // Fan speed (b[0] bits 7:4)
  switch (f[0] & 0xF0) {
    case GREE_FAN_1:
      this->fan_mode = climate::CLIMATE_FAN_LOW;
      break;
    case GREE_FAN_2:
      this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
      break;
    case GREE_FAN_3:
      this->fan_mode = climate::CLIMATE_FAN_HIGH;
      break;
    default:
      this->fan_mode = climate::CLIMATE_FAN_AUTO;
      break;
  }

  // Temperature: b[1] + 16; irrelevant in Auto mode
  if (this->mode != climate::CLIMATE_MODE_HEAT_COOL) {
    this->target_temperature =
        static_cast<float>(clamp<uint8_t>((f[1] & 0x1F) + GREE_TEMP_MIN, GREE_TEMP_MIN, GREE_TEMP_MAX));
  }

  // Vane / swing
  bool swing_bit = (f[0] & YB1FA_SWING_BIT) != 0;
  uint8_t b4 = f[4];
  if (!swing_bit && b4 == YB1FA_VANE_OFF) {
    this->swing_mode = climate::CLIMATE_SWING_OFF;
    this->yb1fa_vane_ = YB1FA_VANE_SEL_OFF;
  } else if (!swing_bit) {
    this->swing_mode = climate::CLIMATE_SWING_VERTICAL;
    switch (b4) {
      case YB1FA_VANE_POS1_UP:
        this->yb1fa_vane_ = YB1FA_VANE_SEL_UP;
        break;
      case YB1FA_VANE_POS2_MIDUP:
        this->yb1fa_vane_ = YB1FA_VANE_SEL_MIDUP;
        break;
      case YB1FA_VANE_POS3_MID:
        this->yb1fa_vane_ = YB1FA_VANE_SEL_MIDDLE;
        break;
      case YB1FA_VANE_POS4_MIDDN:
        this->yb1fa_vane_ = YB1FA_VANE_SEL_MIDDOWN;
        break;
      case YB1FA_VANE_POS5_DOWN:
        this->yb1fa_vane_ = YB1FA_VANE_SEL_DOWN;
        break;
      default:
        this->yb1fa_vane_ = YB1FA_VANE_SEL_OFF;
        break;
    }
  } else {
    this->swing_mode = climate::CLIMATE_SWING_VERTICAL;
    switch (b4) {
      case YB1FA_VANE_SWING_ALL:
        this->yb1fa_vane_ = YB1FA_VANE_SEL_SWING;
        break;
      case YB1FA_VANE_SWING_BOT3:
        this->yb1fa_vane_ = YB1FA_VANE_SEL_SWING_B3;
        break;
      case YB1FA_VANE_SWING_MID3:
        this->yb1fa_vane_ = YB1FA_VANE_SEL_SWING_M3;
        break;
      case YB1FA_VANE_SWING_TOP3:
        this->yb1fa_vane_ = YB1FA_VANE_SEL_SWING_T3;
        break;
      default:
        this->yb1fa_vane_ = YB1FA_VANE_SEL_SWING;
        break;
    }
  }

  // b[2] flags
  this->yb1fa_xfan_ = (f[2] & YB1FA_B2_XFAN) != 0;
  this->yb1fa_light_ = (f[2] & YB1FA_B2_LIGHT) != 0;
  this->yb1fa_turbo_ = (f[2] & YB1FA_B2_TURBO) != 0;
  // health follows light — not exposed separately

  // b[0] bit 7: sleep
  this->yb1fa_sleep_ = (f[0] & YB1FA_SLEEP_BIT) != 0;

  // b[5] temp display (bits 4:3)
  switch ((f[5] >> 3) & 0x03) {
    case 1:
      this->yb1fa_tempdisp_ = YB1FA_TEMPDISP_SEL_HOUSE;
      break;
    case 2:
      this->yb1fa_tempdisp_ = YB1FA_TEMPDISP_SEL_INSIDE;
      break;
    case 3:
      this->yb1fa_tempdisp_ = YB1FA_TEMPDISP_SEL_OUTSIDE;
      break;
    default:
      this->yb1fa_tempdisp_ = YB1FA_TEMPDISP_SEL_OFF;
      break;
  }

  this->publish_state();
  return true;
}

// ── Standard Gree frame parser ───────────────────────────────────────────────

bool GreeClimate::parse_state_frame_(const uint8_t f[GREE_STATE_FRAME_SIZE]) {
  // Checksum
  uint8_t expected;
  if (this->model_ == GREE_YAN || this->model_ == GREE_YX1FF) {
    expected = ((f[0] << 4) + (f[1] << 4) + 0xC0);
  } else {
    expected = ((((f[0] & 0x0F) + (f[1] & 0x0F) + (f[2] & 0x0F) + (f[3] & 0x0F) + ((f[4] & 0xF0) >> 4) +
                  ((f[5] & 0xF0) >> 4) + ((f[6] & 0xF0) >> 4) + 0x0A) &
                 0x0F)
                << 4);
  }
  if (f[7] != expected) {
    ESP_LOGD(TAG, "checksum mismatch (got 0x%02X, expected 0x%02X)", f[7], expected);
    return false;
  }

  // Power / mode
  uint8_t mode_raw = f[0] & 0x0F;
  if (!(mode_raw & GREE_MODE_ON)) {
    this->mode = climate::CLIMATE_MODE_OFF;
  } else {
    switch (mode_raw & 0x07) {
      case GREE_MODE_COOL:
        this->mode = climate::CLIMATE_MODE_COOL;
        break;
      case GREE_MODE_DRY:
        this->mode = climate::CLIMATE_MODE_DRY;
        break;
      case GREE_MODE_HEAT:
        this->mode = climate::CLIMATE_MODE_HEAT;
        break;
      case GREE_MODE_AUTO:
        this->mode = climate::CLIMATE_MODE_HEAT_COOL;
        break;
      case GREE_MODE_FAN:
        this->mode = climate::CLIMATE_MODE_FAN_ONLY;
        break;
      default:
        this->mode = climate::CLIMATE_MODE_COOL;
        break;
    }
  }

  // Fan speed
  if (this->model_ == GREE_YX1FF) {
    bool turbo = (f[2] & GREE_FAN_TURBO_BIT) != 0;
    uint8_t fan = f[0] & 0xF0;
    if (turbo)
      this->fan_mode = climate::CLIMATE_FAN_HIGH;
    else if (fan == GREE_FAN_1)
      this->fan_mode = climate::CLIMATE_FAN_QUIET;
    else if (fan == GREE_FAN_2)
      this->fan_mode = climate::CLIMATE_FAN_LOW;
    else if (fan == GREE_FAN_3)
      this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
    else
      this->fan_mode = climate::CLIMATE_FAN_AUTO;
  } else {
    switch (f[0] & 0xF0) {
      case GREE_FAN_1:
        this->fan_mode = climate::CLIMATE_FAN_LOW;
        break;
      case GREE_FAN_2:
        this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
        break;
      case GREE_FAN_3:
        this->fan_mode = climate::CLIMATE_FAN_HIGH;
        break;
      default:
        this->fan_mode = climate::CLIMATE_FAN_AUTO;
        break;
    }
  }

  // Temperature — standard Gree stores the raw value directly (not offset)
  this->target_temperature = static_cast<float>(clamp<uint8_t>(f[1] & 0x1F, GREE_TEMP_MIN, GREE_TEMP_MAX));

  // Swing
  bool swing_bit = (f[0] >> 6) & 0x01;
  if (this->model_ == GREE_YAN) {
    this->swing_mode =
        ((f[4] & 0x0F) == GREE_VDIR_SWING) ? climate::CLIMATE_SWING_VERTICAL : climate::CLIMATE_SWING_OFF;
  } else if (this->model_ == GREE_YAC || this->model_ == GREE_YAG) {
    bool h = ((f[4] >> 4) == GREE_HDIR_SWING);
    if (swing_bit && h)
      this->swing_mode = climate::CLIMATE_SWING_BOTH;
    else if (swing_bit)
      this->swing_mode = climate::CLIMATE_SWING_VERTICAL;
    else if (h)
      this->swing_mode = climate::CLIMATE_SWING_HORIZONTAL;
    else
      this->swing_mode = climate::CLIMATE_SWING_OFF;
  } else {
    this->swing_mode = swing_bit ? climate::CLIMATE_SWING_VERTICAL : climate::CLIMATE_SWING_OFF;
  }

  // Mode bits (for switch components)
  if (this->model_ != GREE_YX1FF)
    this->mode_bits_ = f[2] & 0xF0;

  // Sleep preset (YX1FF)
  if (this->model_ == GREE_YX1FF) {
    this->preset = (f[0] & GREE_PRESET_SLEEP_BIT) ? climate::CLIMATE_PRESET_SLEEP : climate::CLIMATE_PRESET_NONE;
  }

  this->publish_state();
  return true;
}

}  // namespace esphome::gree
