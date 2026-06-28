#pragma once

#include "esphome/components/climate_ir/climate_ir.h"

namespace esphome::gree {

// Temperature
static constexpr uint8_t GREE_TEMP_MIN = 16;  // Celsius
static constexpr uint8_t GREE_TEMP_MAX = 30;  // Celsius

// Modes
static constexpr uint8_t GREE_MODE_AUTO = 0x00;
static constexpr uint8_t GREE_MODE_COOL = 0x01;
static constexpr uint8_t GREE_MODE_DRY = 0x02;
static constexpr uint8_t GREE_MODE_FAN = 0x03;
static constexpr uint8_t GREE_MODE_HEAT = 0x04;
static constexpr uint8_t GREE_MODE_OFF = 0x00;
static constexpr uint8_t GREE_MODE_ON = 0x08;

// Fan Speed
static constexpr uint8_t GREE_FAN_AUTO = 0x00;
static constexpr uint8_t GREE_FAN_1 = 0x10;
static constexpr uint8_t GREE_FAN_2 = 0x20;
static constexpr uint8_t GREE_FAN_3 = 0x30;

// IR Transmission
static constexpr uint32_t GREE_IR_FREQUENCY = 38000;
static constexpr uint32_t GREE_HEADER_MARK = 9000;
static constexpr uint32_t GREE_HEADER_SPACE = 4000;
static constexpr uint32_t GREE_BIT_MARK = 620;
static constexpr uint32_t GREE_ONE_SPACE = 1600;
static constexpr uint32_t GREE_ZERO_SPACE = 540;
static constexpr uint32_t GREE_MESSAGE_SPACE = 19000;

// Timing specific for YAC features (I-Feel mode)
static constexpr uint32_t GREE_YAC_HEADER_MARK = 6000;
static constexpr uint32_t GREE_YAC_HEADER_SPACE = 3000;
static constexpr uint32_t GREE_YAC_BIT_MARK = 650;

// Timing specific to YAC1FB9 and YB1FA
static constexpr uint32_t GREE_YAC1FB9_HEADER_SPACE = 4500;
static constexpr uint32_t GREE_YAC1FB9_MESSAGE_SPACE = 19980;

// IR receive tolerance
static constexpr uint8_t GREE_IR_TOLERANCE = 25;  // %

// State Frame size
static constexpr uint8_t GREE_STATE_FRAME_SIZE = 8;

// Vertical air directions (YAN, and YB1FA b[4] high nibble)
static constexpr uint8_t GREE_VDIR_AUTO = 0x00;
static constexpr uint8_t GREE_VDIR_MANUAL = 0x00;
static constexpr uint8_t GREE_VDIR_SWING = 0x01;
static constexpr uint8_t GREE_VDIR_UP = 0x02;
static constexpr uint8_t GREE_VDIR_MUP = 0x03;
static constexpr uint8_t GREE_VDIR_MIDDLE = 0x04;
static constexpr uint8_t GREE_VDIR_MDOWN = 0x05;
static constexpr uint8_t GREE_VDIR_DOWN = 0x06;

// Horizontal air directions (YAC/YAG)
static constexpr uint8_t GREE_HDIR_AUTO = 0x00;
static constexpr uint8_t GREE_HDIR_MANUAL = 0x00;
static constexpr uint8_t GREE_HDIR_SWING = 0x01;
static constexpr uint8_t GREE_HDIR_LEFT = 0x02;
static constexpr uint8_t GREE_HDIR_MLEFT = 0x03;
static constexpr uint8_t GREE_HDIR_MIDDLE = 0x04;
static constexpr uint8_t GREE_HDIR_MRIGHT = 0x05;
static constexpr uint8_t GREE_HDIR_RIGHT = 0x06;

// YX1FF turbo/sleep
static constexpr uint8_t GREE_FAN_TURBO = 0x80;
static constexpr uint8_t GREE_FAN_TURBO_BIT = 0x10;
static constexpr uint8_t GREE_PRESET_NONE = 0x00;
static constexpr uint8_t GREE_PRESET_SLEEP = 0x01;
static constexpr uint8_t GREE_PRESET_SLEEP_BIT = 0x80;

// ── YB1FA-specific constants ─────────────────────────────────────────────────
// b[0] bit assignments
static constexpr uint8_t YB1FA_SLEEP_BIT = 0x80;  // b[0] bit 7
static constexpr uint8_t YB1FA_SWING_BIT = 0x40;  // b[0] bit 6

// b[2] flag bits (YB1FA b[2] bit layout differs from other Gree models)
static constexpr uint8_t YB1FA_B2_XFAN = 0x80;    // bit 7
static constexpr uint8_t YB1FA_B2_LIGHT = 0x40;   // bit 6 (default ON; toggles together with HEALTH)
static constexpr uint8_t YB1FA_B2_HEALTH = 0x20;  // bit 5 (default ON; follows LIGHT)
static constexpr uint8_t YB1FA_B2_TURBO = 0x10;   // bit 4

// b[2] default: health+light ON
static constexpr uint8_t YB1FA_B2_DEFAULT = 0x60;

// b[4] vane low-nibble tags
static constexpr uint8_t YB1FA_VANE_LO_FIXED = 0x02;
static constexpr uint8_t YB1FA_VANE_LO_SWING = 0x0A;

// b[4] full vane-position bytes (high nibble = index, low nibble = fixed/swing tag)
static constexpr uint8_t YB1FA_VANE_OFF = 0x02;
static constexpr uint8_t YB1FA_VANE_POS1_UP = 0x12;
static constexpr uint8_t YB1FA_VANE_POS2_MIDUP = 0x1A;
static constexpr uint8_t YB1FA_VANE_POS3_MID = 0x22;
static constexpr uint8_t YB1FA_VANE_POS4_MIDDN = 0x2A;
static constexpr uint8_t YB1FA_VANE_POS5_DOWN = 0x32;
static constexpr uint8_t YB1FA_VANE_SWING_ALL = 0x0A;   // b[0] bit6=1
static constexpr uint8_t YB1FA_VANE_SWING_BOT3 = 0x3A;  // b[0] bit6=1
static constexpr uint8_t YB1FA_VANE_SWING_MID3 = 0x4A;  // b[0] bit6=1
static constexpr uint8_t YB1FA_VANE_SWING_TOP3 = 0x5A;  // b[0] bit6=1

// b[5] temperature display mode (bits 4:3)
static constexpr uint8_t YB1FA_TEMPDISP_OFF = 0x00;
static constexpr uint8_t YB1FA_TEMPDISP_HOUSE = 0x08;
static constexpr uint8_t YB1FA_TEMPDISP_INSIDE = 0x10;
static constexpr uint8_t YB1FA_TEMPDISP_OUTSIDE = 0x18;

// b[3] and b[6] are constant for YB1FA
static constexpr uint8_t YB1FA_B3 = 0x50;
static constexpr uint8_t YB1FA_B6 = 0x01;

// Model codes
enum Model {
  GREE_GENERIC,
  GREE_YAN,
  GREE_YAA,
  GREE_YAC,
  GREE_YAC1FB9,
  GREE_YB1FA,
  GREE_YX1FF,
  GREE_YAG,
};

// YB1FA vane position select values (used by select component)
enum Yb1faVane : uint8_t {
  YB1FA_VANE_SEL_OFF = 0,
  YB1FA_VANE_SEL_UP = 1,
  YB1FA_VANE_SEL_MIDUP = 2,
  YB1FA_VANE_SEL_MIDDLE = 3,
  YB1FA_VANE_SEL_MIDDOWN = 4,
  YB1FA_VANE_SEL_DOWN = 5,
  YB1FA_VANE_SEL_SWING = 6,
  YB1FA_VANE_SEL_SWING_B3 = 7,
  YB1FA_VANE_SEL_SWING_M3 = 8,
  YB1FA_VANE_SEL_SWING_T3 = 9,
};

// YB1FA temperature display select values (used by select component)
enum Yb1faTempDisp : uint8_t {
  YB1FA_TEMPDISP_SEL_OFF = 0,
  YB1FA_TEMPDISP_SEL_HOUSE = 1,
  YB1FA_TEMPDISP_SEL_INSIDE = 2,
  YB1FA_TEMPDISP_SEL_OUTSIDE = 3,
};

class GreeClimate final : public climate_ir::ClimateIR {
 public:
  GreeClimate()
      : climate_ir::ClimateIR(GREE_TEMP_MIN, GREE_TEMP_MAX, 1.0f, true, true,
                              {climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW, climate::CLIMATE_FAN_MEDIUM,
                               climate::CLIMATE_FAN_HIGH},
                              {climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_VERTICAL,
                               climate::CLIMATE_SWING_HORIZONTAL, climate::CLIMATE_SWING_BOTH}) {}

  void set_model(Model model);

  // Generic Gree mode-bit switch (YAN/YAA/YAC/YAC1FB9)
  void set_mode_bit(uint8_t bit_mask, bool enabled);

  // YB1FA-specific setters called from switch/select components
  void yb1fa_set_turbo(bool on);
  void yb1fa_set_xfan(bool on);
  void yb1fa_set_light(bool on);
  void yb1fa_set_sleep(bool on);
  void yb1fa_set_vane(Yb1faVane pos);
  void yb1fa_set_tempdisp(Yb1faTempDisp mode);

  // Getters for state restore
  bool yb1fa_get_turbo() const { return this->yb1fa_turbo_; }
  bool yb1fa_get_xfan() const { return this->yb1fa_xfan_; }
  bool yb1fa_get_light() const { return this->yb1fa_light_; }
  bool yb1fa_get_sleep() const { return this->yb1fa_sleep_; }
  Yb1faVane yb1fa_get_vane() const { return this->yb1fa_vane_; }
  Yb1faTempDisp yb1fa_get_tempdisp() const { return this->yb1fa_tempdisp_; }

 protected:
  void transmit_state() override;
  bool on_receive(remote_base::RemoteReceiveData data) override;
  climate::ClimateTraits traits() override;

  uint8_t operation_mode_();
  uint8_t fan_speed_();
  uint8_t horizontal_swing_();
  uint8_t vertical_swing_();
  uint8_t temperature_();
  uint8_t preset_();

  // YB1FA transmit/receive helpers
  void transmit_yb1fa_();
  bool parse_yb1fa_frame_(const uint8_t frame[GREE_STATE_FRAME_SIZE]);

  // Standard Gree receive helper
  bool parse_state_frame_(const uint8_t frame[GREE_STATE_FRAME_SIZE]);

  // Shared IR send helper
  void send_ir_(const uint8_t remote_state[GREE_STATE_FRAME_SIZE], bool yac1fb9_timing);

  Model model_{};
  uint8_t mode_bits_{0};  // for YAN/YAA/YAC/YAC1FB9

  // YB1FA state
  bool yb1fa_turbo_{false};
  bool yb1fa_xfan_{false};
  bool yb1fa_light_{true};  // default: display ON
  bool yb1fa_sleep_{false};
  Yb1faVane yb1fa_vane_{YB1FA_VANE_SEL_OFF};
  Yb1faTempDisp yb1fa_tempdisp_{YB1FA_TEMPDISP_SEL_OFF};
};

}  // namespace esphome::gree
