#include "spaceclock_renderer.h"

#include <cstdlib>

namespace spaceclock {
namespace {

const uint8_t kBarsR[9] = {100, 100, 0, 0, 50, 100, 100, 120, 180};
const uint8_t kBarsG[9] = {0, 60, 100, 50, 0, 0, 25, 110, 180};
const uint8_t kBarsB[9] = {0, 0, 0, 100, 100, 100, 75, 180, 40};
const uint8_t kAltBarsR[9] = {140, 110, 80, 40, 90, 150, 120, 180, 220};
const uint8_t kAltBarsG[9] = {0, 0, 20, 40, 0, 0, 30, 80, 140};
const uint8_t kAltBarsB[9] = {160, 220, 160, 220, 180, 220, 140, 220, 80};
const int kBurstDx[8] = {1, -1, 0, 0, 1, -1, 1, -1};
const int kBurstDy[8] = {0, 0, 1, -1, 1, 1, -1, -1};

uint8_t glyph_column(char ch, int col) {
  static const uint8_t kH[3] = {0b11111, 0b00100, 0b11111};
  static const uint8_t kA[3] = {0b11110, 0b00101, 0b11110};
  static const uint8_t kX[3] = {0b11011, 0b00100, 0b11011};
  static const uint8_t kK[3] = {0b11111, 0b00100, 0b11011};
  static const uint8_t kO[3] = {0b01110, 0b10001, 0b01110};
  static const uint8_t kSpace[3] = {0, 0, 0};

  if (col < 0 || col > 2) {
    return 0;
  }
  if (ch == 'H') {
    return kH[col];
  }
  if (ch == 'A') {
    return kA[col];
  }
  if (ch == 'X') {
    return kX[col];
  }
  if (ch == 'K') {
    return kK[col];
  }
  if (ch == 'O') {
    return kO[col];
  }
  return kSpace[col];
}

}  // namespace

inline void Renderer::reset_snake_(const AnimationContext &ctx) {
  if (ctx.draw.animation_width <= 0 || ctx.draw.animation_height <= 0) {
    this->snake_len_ = 1;
    this->snake_x_[0] = 0;
    this->snake_y_[0] = 0;
    this->apple_x_ = 0;
    this->apple_y_ = 0;
    return;
  }

  this->snake_len_ = 1;
  this->snake_x_[0] = ctx.draw.animation_width / 2;
  this->snake_y_[0] = ctx.draw.animation_height / 2;
  this->snake_dx_ = 1;
  this->snake_dy_ = 0;

  int free_cells_x[kMaxCells];
  int free_cells_y[kMaxCells];
  int free_count = 0;
  for (int y = 0; y < ctx.draw.animation_height; y++) {
    for (int x = 0; x < ctx.draw.animation_width; x++) {
      if (x == this->snake_x_[0] && y == this->snake_y_[0]) {
        continue;
      }
      free_cells_x[free_count] = x;
      free_cells_y[free_count] = y;
      free_count++;
    }
  }

  if (free_count == 0) {
    this->apple_x_ = this->snake_x_[0];
    this->apple_y_ = this->snake_y_[0];
    return;
  }

  const int choice = this->next_rand_() % free_count;
  this->apple_x_ = free_cells_x[choice];
  this->apple_y_ = free_cells_y[choice];
}

inline void Renderer::spawn_apple_(const AnimationContext &ctx) {
  int free_cells_x[kMaxCells];
  int free_cells_y[kMaxCells];
  int free_count = 0;
  for (int y = 0; y < ctx.draw.animation_height; y++) {
    for (int x = 0; x < ctx.draw.animation_width; x++) {
      bool occupied = false;
      for (int i = 0; i < this->snake_len_; i++) {
        if (this->snake_x_[i] == x && this->snake_y_[i] == y) {
          occupied = true;
          break;
        }
      }
      if (!occupied) {
        free_cells_x[free_count] = x;
        free_cells_y[free_count] = y;
        free_count++;
      }
    }
  }

  if (free_count == 0) {
    this->reset_snake_(ctx);
    return;
  }

  const int choice = this->next_rand_() % free_count;
  this->apple_x_ = free_cells_x[choice];
  this->apple_y_ = free_cells_y[choice];
}

inline void Renderer::reset_fireworks_(const AnimationContext &ctx) {
  const int usable_width = ctx.draw.animation_width > 2 ? ctx.draw.animation_width - 2 : 1;
  this->firework_x_ =
      ctx.draw.animation_width > 2 ? 1 + (this->next_rand_() % usable_width) : ctx.draw.animation_width / 2;

  if (ctx.draw.animation_height >= 5) {
    this->firework_peak_y_ = 1 + (this->next_rand_() % (ctx.draw.animation_height - 2));
  } else if (ctx.draw.animation_height >= 3) {
    this->firework_peak_y_ = ctx.draw.animation_height / 2;
  } else {
    this->firework_peak_y_ = 0;
  }
}

inline void Renderer::reset_starfield_(const AnimationContext &ctx) {
  if (ctx.draw.animation_width <= 0 || ctx.draw.animation_height <= 0) {
    for (int i = 0; i < kMaxStars; i++) {
      this->star_x_[i] = 0;
      this->star_y_[i] = 0;
      this->star_level_[i] = 0;
    }
    return;
  }

  const int star_count = ctx.draw.animation_width < kMaxStars ? ctx.draw.animation_width : kMaxStars;
  for (int i = 0; i < kMaxStars; i++) {
    if (i < star_count) {
      this->star_x_[i] = this->next_rand_() % ctx.draw.animation_width;
      this->star_y_[i] = this->next_rand_() % ctx.draw.animation_height;
      this->star_level_[i] = 1 + (this->next_rand_() % 3);
    } else {
      this->star_x_[i] = 0;
      this->star_y_[i] = 0;
      this->star_level_[i] = 0;
    }
  }
}

inline bool Renderer::render_snake_(const AnimationContext &ctx) {
  auto clamp_x = [&](int value) -> int {
    if (value < 0) {
      return 0;
    }
    if (value >= ctx.draw.animation_width) {
      return ctx.draw.animation_width - 1;
    }
    return value;
  };
  auto clamp_y = [&](int value) -> int {
    if (value < 0) {
      return 0;
    }
    if (value >= ctx.draw.animation_height) {
      return ctx.draw.animation_height - 1;
    }
    return value;
  };

  this->snake_progress_ += ctx.speed_scale * 0.5f;
  while (this->snake_progress_ >= 1.0f) {
    this->snake_progress_ -= 1.0f;
    ctx.animation_step_counter++;

    const int head_x = this->snake_x_[0];
    const int head_y = this->snake_y_[0];
    int preferred_x[4];
    int preferred_y[4];
    int preferred_count = 0;

    if (head_x < this->apple_x_) {
      preferred_x[preferred_count] = 1;
      preferred_y[preferred_count] = 0;
      preferred_count++;
    }
    if (head_x > this->apple_x_) {
      preferred_x[preferred_count] = -1;
      preferred_y[preferred_count] = 0;
      preferred_count++;
    }
    if (head_y < this->apple_y_) {
      preferred_x[preferred_count] = 0;
      preferred_y[preferred_count] = 1;
      preferred_count++;
    }
    if (head_y > this->apple_y_) {
      preferred_x[preferred_count] = 0;
      preferred_y[preferred_count] = -1;
      preferred_count++;
    }
    if (preferred_count == 0) {
      preferred_x[preferred_count] = this->snake_dx_;
      preferred_y[preferred_count] = this->snake_dy_;
      preferred_count++;
    }

    const int move_x[4] = {1, -1, 0, 0};
    const int move_y[4] = {0, 0, 1, -1};
    for (int i = 0; i < 4; i++) {
      bool seen = false;
      for (int j = 0; j < preferred_count; j++) {
        if (preferred_x[j] == move_x[i] && preferred_y[j] == move_y[i]) {
          seen = true;
          break;
        }
      }
      if (!seen) {
        preferred_x[preferred_count] = move_x[i];
        preferred_y[preferred_count] = move_y[i];
        preferred_count++;
      }
    }

    int chosen_x = preferred_x[0];
    int chosen_y = preferred_y[0];
    for (int i = 0; i < preferred_count; i++) {
      const int nx = clamp_x(head_x + preferred_x[i]);
      const int ny = clamp_y(head_y + preferred_y[i]);
      bool collides = false;
      for (int j = 0; j < this->snake_len_; j++) {
        if (this->snake_x_[j] == nx && this->snake_y_[j] == ny) {
          collides = true;
          break;
        }
      }
      if (!collides) {
        chosen_x = preferred_x[i];
        chosen_y = preferred_y[i];
        break;
      }
    }

    this->snake_dx_ = chosen_x;
    this->snake_dy_ = chosen_y;

    const int new_x = clamp_x(head_x + this->snake_dx_);
    const int new_y = clamp_y(head_y + this->snake_dy_);
    bool self_hit = false;
    for (int i = 0; i < this->snake_len_; i++) {
      if (this->snake_x_[i] == new_x && this->snake_y_[i] == new_y) {
        self_hit = true;
        break;
      }
    }

    if (self_hit) {
      this->reset_snake_(ctx);
      this->finish_animation_(ctx.continuous_animation, ctx.animation_mode);
    } else {
      for (int i = this->snake_len_; i > 0 && i < kMaxCells; i--) {
        this->snake_x_[i] = this->snake_x_[i - 1];
        this->snake_y_[i] = this->snake_y_[i - 1];
      }
      this->snake_x_[0] = new_x;
      this->snake_y_[0] = new_y;

      if (new_x == this->apple_x_ && new_y == this->apple_y_) {
        this->snake_len_++;
        if (this->snake_len_ > ctx.max_apples) {
          this->reset_snake_(ctx);
          this->finish_animation_(ctx.continuous_animation, ctx.animation_mode);
        } else {
          this->spawn_apple_(ctx);
        }
      }
    }
  }

  this->draw_matrix_xy_(ctx.draw, this->apple_x_, this->apple_y_,
                        this->snake_len_ >= ctx.max_apples ? this->palette_color_(ctx.draw, 100, 100, 8, 80, 180, 220)
                                                           : this->palette_color_(ctx.draw, 110, 8, 8, 160, 0, 190));
  for (int i = this->snake_len_ - 1; i >= 0; i--) {
    this->draw_matrix_xy_(
        ctx.draw, this->snake_x_[i], this->snake_y_[i],
        i == 0 ? this->palette_color_(ctx.draw, 8, 200, 8, 0, 190, 220)
               : this->palette_color_(ctx.draw, 3, 70, 3, 70, 0, 110));
  }
  if (ctx.animation_step_counter >= (ctx.draw.animation_width * ctx.draw.animation_height * 3)) {
    this->reset_snake_(ctx);
    this->finish_animation_(ctx.continuous_animation, ctx.animation_mode);
  }
  return true;
}

inline bool Renderer::render_noise_(const AnimationContext &ctx) {
  this->noise_progress_ += ctx.speed_scale;
  while (this->noise_progress_ >= 1.0f) {
    this->noise_progress_ -= 1.0f;
    ctx.animation_step_counter++;
    for (int i = 0; i < (ctx.draw.animation_width * ctx.draw.animation_height); i++) {
      this->noise_level_[i] = (this->next_rand_() % 3) * 40;
      this->noise_variant_[i] = this->next_rand_() % 2;
    }
  }

  for (int y = 0; y < ctx.draw.animation_height; y++) {
    for (int x = 0; x < ctx.draw.animation_width; x++) {
      const int idx = (y * ctx.draw.animation_width) + x;
      const int level = this->noise_level_[idx];
      this->draw_matrix_xy_(ctx.draw, x, y,
                            this->palette_color_(ctx.draw, level, level, level, level, 0,
                                                 level + this->noise_variant_[idx] * 20));
    }
  }

  if (ctx.animation_step_counter >= 40) {
    this->finish_animation_(ctx.continuous_animation, ctx.animation_mode);
  }
  return true;
}

inline bool Renderer::render_bars_(const AnimationContext &ctx) {
  const int cycle = (ctx.draw.animation_width * 2) - 2;
  if (cycle <= 0) {
    this->finish_animation_(ctx.continuous_animation, ctx.animation_mode);
    return true;
  }

  const int pos = this->bars_frame_ < ctx.draw.animation_width ? this->bars_frame_ : cycle - this->bars_frame_;
  const int prev_frame = this->bars_frame_ == 0 ? cycle - 1 : this->bars_frame_ - 1;
  const int prev_pos = prev_frame < ctx.draw.animation_width ? prev_frame : cycle - prev_frame;
  const int mirror = ctx.draw.animation_width - 1 - pos;
  const int prev_mirror = ctx.draw.animation_width - 1 - prev_pos;
  const int color_idx = pos % 9;
  const int prev_color_idx = prev_pos % 9;

  for (int y = 0; y < ctx.draw.animation_height; y++) {
    this->draw_matrix_xy_(ctx.draw, prev_pos, y,
                          this->palette_color_(ctx.draw, kBarsR[prev_color_idx] / 6, kBarsG[prev_color_idx] / 6,
                                               kBarsB[prev_color_idx] / 6, kAltBarsR[prev_color_idx] / 6,
                                               kAltBarsG[prev_color_idx] / 6, kAltBarsB[prev_color_idx] / 6));
    this->draw_matrix_xy_(ctx.draw, prev_mirror, y,
                          this->palette_color_(ctx.draw, kBarsR[prev_color_idx] / 6, kBarsG[prev_color_idx] / 6,
                                               kBarsB[prev_color_idx] / 6, kAltBarsR[prev_color_idx] / 6,
                                               kAltBarsG[prev_color_idx] / 6, kAltBarsB[prev_color_idx] / 6));
    this->draw_matrix_xy_(
        ctx.draw, pos, y,
        this->palette_color_(ctx.draw, kBarsR[color_idx], kBarsG[color_idx], kBarsB[color_idx], kAltBarsR[color_idx],
                             kAltBarsG[color_idx], kAltBarsB[color_idx]));
    this->draw_matrix_xy_(
        ctx.draw, mirror, y,
        this->palette_color_(ctx.draw, kBarsR[color_idx], kBarsG[color_idx], kBarsB[color_idx], kAltBarsR[color_idx],
                             kAltBarsG[color_idx], kAltBarsB[color_idx]));
  }

  this->bars_progress_ += ctx.speed_scale;
  while (this->bars_progress_ >= 1.0f) {
    this->bars_progress_ -= 1.0f;
    this->bars_frame_ = (this->bars_frame_ + 1) % cycle;
  }
  if (this->bars_frame_ == 0) {
    this->finish_animation_(ctx.continuous_animation, ctx.animation_mode);
  }
  return true;
}

inline bool Renderer::render_logo_(const AnimationContext &ctx) {
  const char text[] = " HAXKO ";
  const int text_len = 7;
  const int total_columns = text_len * 4;
  const int offset = this->logo_frame_ - ctx.draw.animation_width;

  for (int x = 0; x < ctx.draw.animation_width; x++) {
    const int src = offset + x;
    if (src < 0 || src >= total_columns) {
      continue;
    }

    const int char_index = src / 4;
    const int column_in_char = src % 4;
    uint8_t bits = 0;
    if (column_in_char < 3) {
      bits = glyph_column(text[char_index], column_in_char);
    }

    for (int y = 0; y < ctx.draw.animation_height && y < 5; y++) {
      if (bits & (1 << (4 - y))) {
        const esphome::Color color = text[char_index] == 'X' ? this->palette_color_(ctx.draw, 180, 70, 0, 160, 0, 190)
                                                             : this->palette_color_(ctx.draw, 134, 172, 172, 0, 190, 220);
        this->draw_matrix_xy_(ctx.draw, x, y, color);
      }
    }
  }

  this->logo_progress_ += ctx.speed_scale;
  while (this->logo_progress_ >= 1.0f) {
    this->logo_progress_ -= 1.0f;
    this->logo_frame_ = (this->logo_frame_ + 1) % (total_columns + ctx.draw.animation_width);
  }
  if (this->logo_frame_ == 0) {
    this->finish_animation_(ctx.continuous_animation, ctx.animation_mode);
  }
  return true;
}

inline bool Renderer::render_meteor_(const AnimationContext &ctx) {
  const int tail_length = ctx.draw.animation_height >= 5 ? 5 : 3;
  const int total_frames = ctx.draw.animation_width + tail_length + 2;

  this->meteor_progress_ += ctx.speed_scale * 0.9f;
  while (this->meteor_progress_ >= 1.0f) {
    this->meteor_progress_ -= 1.0f;
    ctx.animation_step_counter++;
  }

  for (int segment = 0; segment < tail_length; segment++) {
    const int x = ctx.animation_step_counter - segment;
    if (x < 0 || x >= ctx.draw.animation_width) {
      continue;
    }

    const int y = ctx.draw.animation_width > 1 ? (x * (ctx.draw.animation_height - 1)) / (ctx.draw.animation_width - 1)
                                               : 0;
    const int fade = tail_length - segment;
    this->draw_matrix_xy_(ctx.draw, x, y, this->palette_color_(ctx.draw, 35 * fade, 22 * fade, 6 * fade,
                                                                24 * fade, 8 * fade, 36 * fade));

    if (segment == 0 && y + 1 < ctx.draw.animation_height) {
      this->draw_matrix_xy_(ctx.draw, x, y + 1, this->palette_color_(ctx.draw, 20, 10, 2, 12, 0, 18));
    }
  }

  if (ctx.animation_step_counter >= total_frames) {
    this->finish_animation_(ctx.continuous_animation, ctx.animation_mode);
  }
  return true;
}

inline bool Renderer::render_radar_(const AnimationContext &ctx) {
  const int cx = ctx.draw.animation_width / 2;
  const int cy = ctx.draw.animation_height / 2;
  const int max_ring = cx + cy + 1;

  this->radar_progress_ += ctx.speed_scale * 0.75f;
  while (this->radar_progress_ >= 1.0f) {
    this->radar_progress_ -= 1.0f;
    ctx.animation_step_counter++;
  }

  const int ring = ctx.animation_step_counter;
  for (int y = 0; y < ctx.draw.animation_height; y++) {
    for (int x = 0; x < ctx.draw.animation_width; x++) {
      const int dist = std::abs(x - cx) + std::abs(y - cy);
      if (dist == ring) {
        this->draw_matrix_xy_(ctx.draw, x, y, this->palette_color_(ctx.draw, 0, 140, 50, 0, 130, 180));
      } else if (dist == ring - 1) {
        this->draw_matrix_xy_(ctx.draw, x, y, this->palette_color_(ctx.draw, 0, 28, 10, 0, 26, 36));
      } else if (dist == 0) {
        this->draw_matrix_xy_(ctx.draw, x, y, this->palette_color_(ctx.draw, 0, 45, 16, 0, 36, 52));
      }
    }
  }

  if (ctx.animation_step_counter > max_ring) {
    this->finish_animation_(ctx.continuous_animation, ctx.animation_mode);
  }
  return true;
}

inline bool Renderer::render_fireworks_(const AnimationContext &ctx) {
  const int rise_frames = ctx.draw.animation_height + 2;
  const int burst_frames = ctx.draw.animation_height >= 5 ? 4 : 3;

  this->fireworks_progress_ += ctx.speed_scale * 0.85f;
  while (this->fireworks_progress_ >= 1.0f) {
    this->fireworks_progress_ -= 1.0f;
    ctx.animation_step_counter++;
  }

  if (ctx.animation_step_counter < rise_frames) {
    const int rise_distance = (ctx.draw.animation_height - 1) - this->firework_peak_y_;
    const int y = rise_distance > 0
                      ? (ctx.draw.animation_height - 1) - ((ctx.animation_step_counter * rise_distance) / (rise_frames - 1))
                      : this->firework_peak_y_;
    this->draw_matrix_xy_(ctx.draw, this->firework_x_, y, this->palette_color_(ctx.draw, 180, 120, 20, 220, 80, 0));
    if (y + 1 < ctx.draw.animation_height) {
      this->draw_matrix_xy_(ctx.draw, this->firework_x_, y + 1, this->palette_color_(ctx.draw, 40, 20, 4, 28, 8, 0));
    }
    return true;
  }

  const int burst_step = ctx.animation_step_counter - rise_frames;
  this->draw_matrix_xy_(ctx.draw, this->firework_x_, this->firework_peak_y_,
                        this->palette_color_(ctx.draw, 180, 180, 60, 220, 120, 200));

  for (int i = 0; i < 8; i++) {
    const int x = this->firework_x_ + (kBurstDx[i] * burst_step);
    const int y = this->firework_peak_y_ + (kBurstDy[i] * burst_step);
    this->draw_matrix_xy_(ctx.draw, x, y,
                          i < 4 ? this->palette_color_(ctx.draw, 180, 90, 10, 200, 0, 220)
                                : this->palette_color_(ctx.draw, 120, 140, 24, 0, 170, 220));
    if (burst_step > 0) {
      this->draw_matrix_xy_(ctx.draw, x - kBurstDx[i], y - kBurstDy[i],
                            this->palette_color_(ctx.draw, 24, 10, 2, 18, 0, 24));
    }
  }

  if (burst_step >= burst_frames) {
    this->finish_animation_(ctx.continuous_animation, ctx.animation_mode);
  }
  return true;
}

inline bool Renderer::render_equalizer_(const AnimationContext &ctx) {
  this->equalizer_progress_ += ctx.speed_scale * 0.8f;
  while (this->equalizer_progress_ >= 1.0f) {
    this->equalizer_progress_ -= 1.0f;
    ctx.animation_step_counter++;
  }

  const int cycle = (ctx.draw.animation_height * 2) + 2;
  for (int x = 0; x < ctx.draw.animation_width; x++) {
    int phase = (ctx.animation_step_counter + (x * 2)) % cycle;
    int height = phase <= ctx.draw.animation_height ? phase : (cycle - phase);
    if (height > ctx.draw.animation_height) {
      height = ctx.draw.animation_height;
    }
    for (int level = 0; level < height; level++) {
      const int y = ctx.draw.animation_height - 1 - level;
      this->draw_matrix_xy_(ctx.draw, x, y,
                            level == height - 1
                                ? this->palette_color_(ctx.draw, 160, 120, 10, 220, 0, 180)
                                : this->palette_color_(ctx.draw, 20 + (x * 10), 60 + (level * 30), 10, 0,
                                                       40 + (x * 10), 80 + (level * 20)));
    }
  }

  if (ctx.animation_step_counter >= (ctx.draw.animation_width * 3)) {
    this->finish_animation_(ctx.continuous_animation, ctx.animation_mode);
  }
  return true;
}

inline bool Renderer::render_starfield_(const AnimationContext &ctx) {
  const int star_count = ctx.draw.animation_width < kMaxStars ? ctx.draw.animation_width : kMaxStars;

  this->starfield_progress_ += ctx.speed_scale * 0.8f;
  while (this->starfield_progress_ >= 1.0f) {
    this->starfield_progress_ -= 1.0f;
    ctx.animation_step_counter++;
    for (int i = 0; i < star_count; i++) {
      this->star_x_[i]--;
      if (this->star_x_[i] < 0) {
        this->star_x_[i] = ctx.draw.animation_width - 1 + (this->next_rand_() % 3);
        this->star_y_[i] = this->next_rand_() % ctx.draw.animation_height;
        this->star_level_[i] = 1 + (this->next_rand_() % 3);
      }
    }
  }

  for (int i = 0; i < star_count; i++) {
    const int x = this->star_x_[i];
    const int y = this->star_y_[i];
    if (x >= 0 && x < ctx.draw.animation_width) {
      const int bright = 40 + (this->star_level_[i] * 45);
      this->draw_matrix_xy_(ctx.draw, x, y, this->palette_color_(ctx.draw, bright, bright, bright, bright, 0, bright));
    }
    if (this->star_level_[i] >= 2 && x + 1 >= 0 && x + 1 < ctx.draw.animation_width) {
      this->draw_matrix_xy_(ctx.draw, x + 1, y, this->palette_color_(ctx.draw, 16, 16, 16, 16, 0, 16));
    }
  }

  if (ctx.animation_step_counter >= (ctx.draw.animation_width * 4)) {
    this->finish_animation_(ctx.continuous_animation, ctx.animation_mode);
  }
  return true;
}

}  // namespace spaceclock
