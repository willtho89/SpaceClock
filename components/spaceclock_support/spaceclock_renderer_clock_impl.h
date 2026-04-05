#include "spaceclock_renderer.h"

namespace spaceclock {

inline esphome::Color Renderer::palette_color_(const DrawContext &ctx, int r1, int g1, int b1, int r2, int g2,
                                               int b2) const {
  if (ctx.palette_mode == 0) {
    return scaled_color_(r1, g1, b1, ctx.brightness);
  }
  if (ctx.palette_mode == 1) {
    return scaled_color_(r2, g2, b2, ctx.brightness);
  }

  const int luma1 = luminance_(r1, g1, b1);
  const int luma2 = luminance_(r2, g2, b2);
  const int peak1 = r1 > g1 ? (r1 > b1 ? r1 : b1) : (g1 > b1 ? g1 : b1);
  const int peak2 = r2 > g2 ? (r2 > b2 ? r2 : b2) : (g2 > b2 ? g2 : b2);
  const int energy = peak1 > peak2 ? peak1 : peak2;
  const int level = clamp_channel_((((luma1 + luma2) / 2) + energy) / 2);
  int r = r1;
  int g = g1;
  int b = b1;

  if (ctx.palette_mode == 2) {
    r = level;
    g = clamp_channel_((level * 170) / 255);
    b = clamp_channel_((level * 36) / 255);
  } else if (ctx.palette_mode == 3) {
    const bool greenish = (g1 + g2) >= (r1 + r2) && (g1 + g2) >= (b1 + b2);
    const bool blueish = (b1 + b2) > (r1 + r2);
    if (greenish) {
      r = 0;
      g = clamp_channel_((level * 170) / 255);
      b = level;
    } else if (blueish) {
      r = clamp_channel_((level * 220) / 255);
      g = 0;
      b = level;
    } else {
      r = level;
      g = clamp_channel_((level * 30) / 255);
      b = clamp_channel_((level * 180) / 255);
    }
  } else if (ctx.palette_mode == 4) {
    r = level;
    g = level;
    b = level;
  } else if (ctx.palette_mode == 5) {
    const bool cool = (b1 + b2) > (g1 + g2);
    r = level;
    g = clamp_channel_((level * (cool ? 48 : 110)) / 255);
    b = clamp_channel_((level * (cool ? 120 : 22)) / 255);
  } else if (ctx.palette_mode == 6) {
    r = clamp_channel_((level * 24) / 255);
    g = level;
    b = clamp_channel_((level * 170) / 255);
  }

  return scaled_color_(r, g, b, ctx.brightness);
}

inline esphome::Color Renderer::clock_section_color_(const DrawContext &ctx, int section, bool lit) const {
  int r = 0;
  int g = 0;
  int b = 0;

  switch (ctx.palette_mode) {
    case 1:
      if (section == 0) {
        r = 180;
        g = 0;
        b = 200;
      } else if (section == 1) {
        r = 0;
        g = 150;
        b = 200;
      } else if (section == 2) {
        r = 0;
        g = 190;
        b = 160;
      } else {
        r = 220;
        g = 220;
        b = 255;
      }
      break;
    case 2:
      if (section == 0) {
        r = 210;
        g = 70;
        b = 20;
      } else if (section == 1) {
        r = 230;
        g = 140;
        b = 20;
      } else if (section == 2) {
        r = 170;
        g = 120;
        b = 10;
      } else {
        r = 255;
        g = 210;
        b = 70;
      }
      break;
    case 3:
      if (section == 0) {
        r = 255;
        g = 30;
        b = 160;
      } else if (section == 1) {
        r = 0;
        g = 210;
        b = 255;
      } else if (section == 2) {
        r = 60;
        g = 255;
        b = 110;
      } else {
        r = 255;
        g = 170;
        b = 20;
      }
      break;
    case 4:
      if (section == 0) {
        r = 80;
        g = 80;
        b = 80;
      } else if (section == 1) {
        r = 130;
        g = 130;
        b = 130;
      } else if (section == 2) {
        r = 180;
        g = 180;
        b = 180;
      } else {
        r = 235;
        g = 235;
        b = 235;
      }
      break;
    case 5:
      if (section == 0) {
        r = 255;
        g = 90;
        b = 70;
      } else if (section == 1) {
        r = 255;
        g = 150;
        b = 50;
      } else if (section == 2) {
        r = 220;
        g = 80;
        b = 140;
      } else {
        r = 140;
        g = 70;
        b = 210;
      }
      break;
    case 6:
      if (section == 0) {
        r = 20;
        g = 220;
        b = 160;
      } else if (section == 1) {
        r = 30;
        g = 170;
        b = 230;
      } else if (section == 2) {
        r = 110;
        g = 230;
        b = 120;
      } else {
        r = 180;
        g = 255;
        b = 210;
      }
      break;
    default:
      if (section == 0) {
        r = 200;
        g = 20;
        b = 20;
      } else if (section == 1) {
        r = 50;
        g = 50;
        b = 160;
      } else if (section == 2) {
        r = 10;
        g = 180;
        b = 10;
      } else {
        r = 200;
        g = 180;
        b = 10;
      }
      break;
  }

  if (!lit) {
    r = (r * 3) / 10;
    g = (g * 3) / 10;
    b = (b * 3) / 10;
  }

  return scaled_color_(r, g, b, ctx.brightness);
}

inline void Renderer::draw_active_index_(const DrawContext &ctx, int active_index, esphome::Color color) const {
  if (ctx.active_width <= 0 || ctx.active_height <= 0 || active_index < 0) {
    return;
  }

  const int local_y = active_index / ctx.active_width;
  const int local_col = active_index % ctx.active_width;
  if (local_y < 0 || local_y >= ctx.active_height) {
    return;
  }

  const int physical_y = local_y + ctx.config.active_y_offset;
  if (physical_y < 0 || physical_y >= ctx.config.matrix_height) {
    return;
  }

  const bool direct = ((physical_y + ctx.config.serpentine_phase) % 2) == 0;
  const int x = direct ? local_col : (ctx.active_width - 1 - local_col);
  ctx.display.draw_pixel_at(x, physical_y, color);
}

inline void Renderer::draw_active_xy_(const DrawContext &ctx, int x, int y, esphome::Color color) const {
  const int physical_y = y + ctx.config.active_y_offset;
  if (x < 0 || x >= ctx.active_width || y < 0 || y >= ctx.active_height) {
    return;
  }
  if (physical_y < 0 || physical_y >= ctx.config.matrix_height) {
    return;
  }
  ctx.display.draw_pixel_at(x, physical_y, color);
}

inline void Renderer::draw_matrix_xy_(const DrawContext &ctx, int x, int y, esphome::Color color) const {
  if (x < 0 || x >= ctx.animation_width || y < 0 || y >= ctx.animation_height) {
    return;
  }
  ctx.display.draw_pixel_at(x, y, color);
}

inline void Renderer::draw_animation_xy_(const DrawContext &ctx, int x, int y, esphome::Color color) const {
  if (x < 0 || x >= ctx.animation_width || y < 0 || y >= ctx.animation_height) {
    return;
  }

  // Mirror animation rows to match the installed panel orientation.
  // The clock renderers already compensate on their own paths.
  ctx.display.draw_pixel_at(x, (ctx.animation_height - 1) - y, color);
}

inline void Renderer::draw_binary_row_(const DrawContext &ctx, int value, int bits, int y, esphome::Color on_color,
                                       esphome::Color off_color) const {
  const int start_x = (ctx.active_width - bits + 1) / 2;
  for (int bit = 0; bit < bits; bit++) {
    this->draw_active_xy_(ctx, start_x + (bits - 1 - bit), y, (value & (1 << bit)) ? on_color : off_color);
  }
}

inline void Renderer::draw_matrix_digit_(const DrawContext &ctx, int digit, int left_x, int top_y,
                                         esphome::Color on_color, esphome::Color off_color) const {
  static const uint8_t digits[10][5] = {
      {0b00, 0b00, 0b00, 0b00, 0b00}, {0b10, 0b10, 0b10, 0b10, 0b10}, {0b11, 0b01, 0b11, 0b10, 0b11},
      {0b11, 0b10, 0b11, 0b10, 0b11}, {0b10, 0b10, 0b11, 0b01, 0b01}, {0b11, 0b10, 0b11, 0b01, 0b11},
      {0b11, 0b11, 0b01, 0b01, 0b11}, {0b10, 0b10, 0b10, 0b10, 0b11}, {0b11, 0b11, 0b00, 0b11, 0b11},
      {0b11, 0b10, 0b10, 0b11, 0b11},
  };

  if (digit < 0 || digit > 9) {
    return;
  }

  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 2; x++) {
      const bool lit = (digits[digit][y] & (1 << x)) != 0;
      this->draw_matrix_xy_(ctx, left_x + x, top_y + y, lit ? on_color : off_color);
    }
  }
}

inline int Renderer::sample_unique_(const int *source, int source_size, int wanted, int *target) {
  if (wanted < 0) {
    wanted = 0;
  }
  if (wanted > source_size) {
    wanted = source_size;
  }

  int pool[9];
  for (int i = 0; i < source_size; i++) {
    pool[i] = source[i];
  }

  for (int i = 0; i < wanted; i++) {
    const int j = i + (this->next_rand_() % (source_size - i));
    const int tmp = pool[i];
    pool[i] = pool[j];
    pool[j] = tmp;
    target[i] = pool[i];
  }

  return wanted;
}

inline void Renderer::reroll_clock_(int hours, int minutes) {
  this->sampled_m01_count_ = this->sample_unique_(kMinutesOnes, 9, minutes % 10, this->sampled_m01_);
  this->sampled_m02_count_ = this->sample_unique_(kMinutesTens, 6, minutes / 10, this->sampled_m02_);
  this->sampled_h01_count_ = this->sample_unique_(kHoursOnes, 9, hours % 10, this->sampled_h01_);
  this->sampled_h02_count_ = this->sample_unique_(kHoursTens, 3, hours / 10, this->sampled_h02_);
}

inline void Renderer::render_binary_clock_(const DrawContext &ctx, const ClockState &state) const {
  this->draw_binary_row_(ctx, state.seconds, 6, 0, this->palette_color_(ctx, 10, 180, 10, 0, 190, 220),
                         this->palette_color_(ctx, 0, 12, 0, 0, 6, 12));
  this->draw_binary_row_(ctx, state.minutes, 6, 1, this->palette_color_(ctx, 50, 50, 160, 160, 0, 190),
                         this->palette_color_(ctx, 0, 0, 12, 8, 0, 10));
  this->draw_binary_row_(ctx, state.hours, 5, 2, this->palette_color_(ctx, 200, 20, 20, 220, 120, 0),
                         this->palette_color_(ctx, 12, 2, 0, 12, 4, 0));
}

inline bool Renderer::render_numeric_clock_(const DrawContext &ctx, const ClockState &state,
                                            bool numeric_clock_mode) const {
  if (!numeric_clock_mode || ctx.animation_width < 9 || ctx.animation_height < 5) {
    return false;
  }

  const esphome::Color m01_lit = this->clock_section_color_(ctx, 0, true);
  const esphome::Color m02_lit = this->clock_section_color_(ctx, 1, true);
  const esphome::Color h01_lit = this->clock_section_color_(ctx, 2, true);
  const esphome::Color h02_lit = this->clock_section_color_(ctx, 3, true);
  const esphome::Color m01_base = this->clock_section_color_(ctx, 0, false);
  const esphome::Color m02_base = this->clock_section_color_(ctx, 1, false);
  const esphome::Color h01_base = this->clock_section_color_(ctx, 2, false);
  const esphome::Color h02_base = this->clock_section_color_(ctx, 3, false);
  const esphome::Color colon_lit = this->clock_section_color_(ctx, 2, true);
  const esphome::Color colon_base = this->clock_section_color_(ctx, 2, false);
  const int start_x = (ctx.animation_width - 9) / 2;
  const int start_y = (ctx.animation_height - 5) / 2;
  const bool colon_on = (state.seconds % 2) == 0;

  this->draw_matrix_digit_(ctx, state.hours / 10, start_x, start_y, h02_lit, h02_base);
  this->draw_matrix_digit_(ctx, state.hours % 10, start_x + 2, start_y, h01_lit, h01_base);
  this->draw_matrix_xy_(ctx, start_x + 4, start_y + 1, colon_on ? colon_lit : colon_base);
  this->draw_matrix_xy_(ctx, start_x + 4, start_y + 3, colon_on ? colon_lit : colon_base);
  this->draw_matrix_digit_(ctx, state.minutes / 10, start_x + 5, start_y, m02_lit, m02_base);
  this->draw_matrix_digit_(ctx, state.minutes % 10, start_x + 7, start_y, m01_lit, m01_base);
  return true;
}

inline void Renderer::render_dot_clock_(const DrawContext &ctx) const {
  const esphome::Color m01_lit = this->clock_section_color_(ctx, 0, true);
  const esphome::Color m02_lit = this->clock_section_color_(ctx, 1, true);
  const esphome::Color h01_lit = this->clock_section_color_(ctx, 2, true);
  const esphome::Color h02_lit = this->clock_section_color_(ctx, 3, true);
  const esphome::Color m01_base = this->clock_section_color_(ctx, 0, false);
  const esphome::Color m02_base = this->clock_section_color_(ctx, 1, false);
  const esphome::Color h01_base = this->clock_section_color_(ctx, 2, false);
  const esphome::Color h02_base = this->clock_section_color_(ctx, 3, false);

  for (int i = 0; i < 9; i++) {
    this->draw_active_index_(ctx, kMinutesOnes[i], m01_base);
  }
  for (int i = 0; i < 6; i++) {
    this->draw_active_index_(ctx, kMinutesTens[i], m02_base);
  }
  for (int i = 0; i < 9; i++) {
    this->draw_active_index_(ctx, kHoursOnes[i], h01_base);
  }
  for (int i = 0; i < 3; i++) {
    this->draw_active_index_(ctx, kHoursTens[i], h02_base);
  }

  for (int i = 0; i < this->sampled_m01_count_; i++) {
    this->draw_active_index_(ctx, this->sampled_m01_[i], m01_lit);
  }
  for (int i = 0; i < this->sampled_m02_count_; i++) {
    this->draw_active_index_(ctx, this->sampled_m02_[i], m02_lit);
  }
  for (int i = 0; i < this->sampled_h01_count_; i++) {
    this->draw_active_index_(ctx, this->sampled_h01_[i], h01_lit);
  }
  for (int i = 0; i < this->sampled_h02_count_; i++) {
    this->draw_active_index_(ctx, this->sampled_h02_[i], h02_lit);
  }
}

}  // namespace spaceclock
