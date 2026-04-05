#pragma once

#include <cstdint>
#include <string>

#include "esphome/components/display/display.h"
#include "esphome/core/color.h"
#include "esphome/core/time.h"

namespace spaceclock {

const char *palette_name(int mode);
int palette_mode_from_name(const std::string &name);
const char *active_mode_name(int animation_mode, bool binary_clock_mode, bool numeric_clock_mode, int palette_mode);

struct RenderConfig {
  int active_y_offset;
  int serpentine_phase;
  int matrix_width;
  int matrix_height;
  int active_width;
  int active_height;
};

class Renderer {
 public:
  void render(esphome::display::Display &it, const RenderConfig &config,
              bool display_enabled, float display_brightness, int palette_mode, bool binary_clock_mode,
              bool numeric_clock_mode, bool &debug_clock_mode, uint32_t &debug_clock_start_ms,
              int &animation_mode, int &animation_step_counter, int render_speed, bool continuous_animation,
              const esphome::ESPTime &ha_now, const esphome::ESPTime &sntp_now);

 private:
  struct DrawContext {
    esphome::display::Display &display;
    const RenderConfig &config;
    float brightness;
    int palette_mode;
    int active_width;
    int active_height;
    int animation_width;
    int animation_height;
  };

  struct AnimationContext {
    const DrawContext &draw;
    int max_apples;
    float speed_scale;
    bool continuous_animation;
    int &animation_mode;
    int &animation_step_counter;
  };

  struct ClockState {
    int hours;
    int minutes;
    int seconds;
  };

  static constexpr int kMaxCells = 45;
  static constexpr int kMaxStars = 8;
  static constexpr int kMinutesOnes[9] = {0, 1, 2, 15, 16, 17, 18, 19, 20};
  static constexpr int kMinutesTens[6] = {3, 4, 13, 14, 21, 22};
  static constexpr int kHoursOnes[9] = {5, 6, 7, 10, 11, 12, 23, 24, 25};
  static constexpr int kHoursTens[3] = {8, 9, 26};

  uint32_t next_rand_();
  static int clamp_channel_(int value);
  static int luminance_(int r, int g, int b);
  static esphome::Color scaled_color_(int r, int g, int b, float brightness);
  esphome::Color palette_color_(const DrawContext &ctx, int r1, int g1, int b1, int r2, int g2, int b2) const;
  esphome::Color clock_section_color_(const DrawContext &ctx, int section, bool lit) const;

  void draw_active_index_(const DrawContext &ctx, int active_index, esphome::Color color) const;
  void draw_active_xy_(const DrawContext &ctx, int x, int y, esphome::Color color) const;
  void draw_matrix_xy_(const DrawContext &ctx, int x, int y, esphome::Color color) const;
  void draw_animation_xy_(const DrawContext &ctx, int x, int y, esphome::Color color) const;
  void draw_binary_row_(const DrawContext &ctx, int value, int bits, int y, esphome::Color on_color,
                        esphome::Color off_color) const;
  void draw_matrix_digit_(const DrawContext &ctx, int digit, int left_x, int top_y, esphome::Color on_color,
                          esphome::Color off_color) const;

  int sample_unique_(const int *source, int source_size, int wanted, int *target);
  void reroll_clock_(int hours, int minutes);
  void finish_animation_(bool continuous_animation, int &animation_mode);
  void initialize_animation_(const AnimationContext &ctx);
  void reset_snake_(const AnimationContext &ctx);
  void spawn_apple_(const AnimationContext &ctx);
  void reset_fireworks_(const AnimationContext &ctx);
  void reset_starfield_(const AnimationContext &ctx);

  bool render_animation_(const AnimationContext &ctx);
  bool render_snake_(const AnimationContext &ctx);
  bool render_noise_(const AnimationContext &ctx);
  bool render_bars_(const AnimationContext &ctx);
  bool render_logo_(const AnimationContext &ctx);
  bool render_meteor_(const AnimationContext &ctx);
  bool render_radar_(const AnimationContext &ctx);
  bool render_fireworks_(const AnimationContext &ctx);
  bool render_equalizer_(const AnimationContext &ctx);
  bool render_starfield_(const AnimationContext &ctx);

  ClockState resolve_clock_state_(const esphome::ESPTime &ha_now, const esphome::ESPTime &sntp_now,
                                  bool &debug_clock_mode, uint32_t debug_clock_start_ms, uint32_t now_ms) const;
  void refresh_clock_samples_(const ClockState &state, uint32_t now_ms, uint32_t clock_shuffle_period_ms);
  void render_binary_clock_(const DrawContext &ctx, const ClockState &state) const;
  bool render_numeric_clock_(const DrawContext &ctx, const ClockState &state, bool numeric_clock_mode) const;
  void render_dot_clock_(const DrawContext &ctx) const;

  uint32_t rng_{0x51A2C3D4u};
  int last_animation_mode_{-1};
  int last_clock_hours_{-1};
  int last_clock_minutes_{-1};
  int sampled_m01_[9]{};
  int sampled_m02_[6]{};
  int sampled_h01_[9]{};
  int sampled_h02_[3]{};
  int sampled_m01_count_{0};
  int sampled_m02_count_{0};
  int sampled_h01_count_{0};
  int sampled_h02_count_{0};
  int snake_len_{1};
  int snake_x_[kMaxCells]{4};
  int snake_y_[kMaxCells]{1};
  int snake_dx_{1};
  int snake_dy_{0};
  int apple_x_{6};
  int apple_y_{1};
  uint8_t noise_level_[kMaxCells]{};
  uint8_t noise_variant_[kMaxCells]{};
  int firework_x_{4};
  int firework_peak_y_{1};
  int star_x_[kMaxStars]{};
  int star_y_[kMaxStars]{};
  int star_level_[kMaxStars]{};
  int bars_frame_{0};
  int logo_frame_{0};
  float snake_progress_{0.0f};
  float noise_progress_{0.0f};
  float bars_progress_{0.0f};
  float logo_progress_{0.0f};
  float meteor_progress_{0.0f};
  float radar_progress_{0.0f};
  float fireworks_progress_{0.0f};
  float equalizer_progress_{0.0f};
  float starfield_progress_{0.0f};
  uint32_t last_clock_shuffle_ms_{0};
};

}  // namespace spaceclock
