#include "spaceclock_renderer.h"

#include "esphome/core/hal.h"

#include "spaceclock_renderer_clock_impl.h"
#include "spaceclock_renderer_animations_impl.h"

namespace spaceclock {

const char *palette_name(int mode) {
  switch (mode) {
    case 1:
      return "Ice";
    case 2:
      return "Amber";
    case 3:
      return "Neon";
    case 4:
      return "Mono";
    case 5:
      return "Sunset";
    case 6:
      return "Mint";
    default:
      return "Classic";
  }
}

int palette_mode_from_name(const std::string &name) {
  if (name == "Ice") {
    return 1;
  }
  if (name == "Amber") {
    return 2;
  }
  if (name == "Neon") {
    return 3;
  }
  if (name == "Mono") {
    return 4;
  }
  if (name == "Sunset") {
    return 5;
  }
  if (name == "Mint") {
    return 6;
  }
  return 0;
}

const char *active_mode_name(int animation_mode, bool binary_clock_mode, bool numeric_clock_mode, int palette_mode) {
  switch (animation_mode) {
    case 1:
      return "Snake";
    case 2:
      return "Noise";
    case 3:
      return "Bars";
    case 4:
      return "HAXKO";
    case 5:
      return "Meteor";
    case 6:
      return "Radar";
    case 7:
      return "Fireworks";
    case 8:
      return "Equalizer";
    case 9:
      return "Starfield";
    default:
      break;
  }

  if (binary_clock_mode) {
    return "Binary Clock";
  }
  if (numeric_clock_mode) {
    return "24h Clock";
  }

  switch (palette_mode) {
    case 1:
      return "Ice Clock";
    case 2:
      return "Amber Clock";
    case 3:
      return "Neon Clock";
    case 4:
      return "Mono Clock";
    case 5:
      return "Sunset Clock";
    case 6:
      return "Mint Clock";
    default:
      return "SpaceClock";
  }
}

uint32_t Renderer::next_rand_() {
  this->rng_ ^= this->rng_ << 13;
  this->rng_ ^= this->rng_ >> 17;
  this->rng_ ^= this->rng_ << 5;
  return this->rng_;
}

int Renderer::clamp_channel_(int value) {
  if (value < 0) {
    return 0;
  }
  if (value > 255) {
    return 255;
  }
  return value;
}

int Renderer::luminance_(int r, int g, int b) {
  return ((r * 30) + (g * 59) + (b * 11)) / 100;
}

esphome::Color Renderer::scaled_color_(int r, int g, int b, float brightness) {
  return esphome::Color(int(r * brightness), int(g * brightness), int(b * brightness));
}

void Renderer::finish_animation_(bool continuous_animation, int &animation_mode) {
  if (continuous_animation) {
    this->last_animation_mode_ = -1;
  } else {
    animation_mode = 0;
  }
}

void Renderer::initialize_animation_(const AnimationContext &ctx) {
  if (ctx.animation_mode == 1) {
    this->reset_snake_(ctx);
  }
  if (ctx.animation_mode == 2) {
    for (int i = 0; i < kMaxCells; i++) {
      this->noise_level_[i] = 0;
      this->noise_variant_[i] = 0;
    }
  }
  if (ctx.animation_mode == 7) {
    this->reset_fireworks_(ctx);
  }
  if (ctx.animation_mode == 9) {
    this->reset_starfield_(ctx);
  }

  this->bars_frame_ = 0;
  this->logo_frame_ = 0;
  this->snake_progress_ = 0.0f;
  this->noise_progress_ = 0.0f;
  this->bars_progress_ = 0.0f;
  this->logo_progress_ = 0.0f;
  this->meteor_progress_ = 0.0f;
  this->radar_progress_ = 0.0f;
  this->fireworks_progress_ = 0.0f;
  this->equalizer_progress_ = 0.0f;
  this->starfield_progress_ = 0.0f;
  ctx.animation_step_counter = 0;
  this->last_animation_mode_ = ctx.animation_mode;
}

bool Renderer::render_animation_(const AnimationContext &ctx) {
  switch (ctx.animation_mode) {
    case 1:
      return this->render_snake_(ctx);
    case 2:
      return this->render_noise_(ctx);
    case 3:
      return this->render_bars_(ctx);
    case 4:
      return this->render_logo_(ctx);
    case 5:
      return this->render_meteor_(ctx);
    case 6:
      return this->render_radar_(ctx);
    case 7:
      return this->render_fireworks_(ctx);
    case 8:
      return this->render_equalizer_(ctx);
    case 9:
      return this->render_starfield_(ctx);
    default:
      return false;
  }
}

Renderer::ClockState Renderer::resolve_clock_state_(const esphome::ESPTime &ha_now, const esphome::ESPTime &sntp_now,
                                                    bool &debug_clock_mode, uint32_t debug_clock_start_ms,
                                                    uint32_t now_ms) const {
  auto now = ha_now;
  if (!now.is_valid()) {
    now = sntp_now;
  }

  ClockState state{
      now.is_valid() ? now.hour : 13,
      now.is_valid() ? now.minute : 37,
      now.is_valid() ? now.second : 0,
  };

  if (!debug_clock_mode) {
    return state;
  }

  const uint32_t elapsed_seconds = (now_ms - debug_clock_start_ms) / 1000u;
  const uint32_t elapsed_minutes = elapsed_seconds > 1440u ? 1440u : elapsed_seconds;

  if (elapsed_seconds > 1440u) {
    debug_clock_mode = false;
  }

  state.hours = elapsed_minutes / 60u;
  state.minutes = elapsed_minutes % 60u;
  state.seconds = now_ms % 2u;
  return state;
}

void Renderer::refresh_clock_samples_(const ClockState &state, uint32_t now_ms, uint32_t clock_shuffle_period_ms) {
  const bool clock_period_elapsed = (now_ms - this->last_clock_shuffle_ms_) >= clock_shuffle_period_ms;
  if (state.hours != this->last_clock_hours_ || state.minutes != this->last_clock_minutes_ || clock_period_elapsed) {
    this->reroll_clock_(state.hours, state.minutes);
    this->last_clock_hours_ = state.hours;
    this->last_clock_minutes_ = state.minutes;
    this->last_clock_shuffle_ms_ = now_ms;
  }
}

void Renderer::render(esphome::display::Display &it, const RenderConfig &config, bool display_enabled,
                      float display_brightness, int palette_mode, bool binary_clock_mode, bool numeric_clock_mode,
                      bool &debug_clock_mode, uint32_t &debug_clock_start_ms, int &animation_mode,
                      int &animation_step_counter, int render_speed, bool continuous_animation,
                      const esphome::ESPTime &ha_now, const esphome::ESPTime &sntp_now) {
  const DrawContext draw{
      it,
      config,
      display_brightness,
      palette_mode,
      config.active_width,
      config.active_height,
      config.matrix_width,
      config.matrix_height,
  };
  const float speed_scale = render_speed > 0 ? (render_speed / 50.0f) : 0.02f;
  const AnimationContext animation{
      draw,
      draw.animation_height >= 5 ? 18 : 12,
      speed_scale,
      continuous_animation,
      animation_mode,
      animation_step_counter,
  };
  const uint32_t now_ms = esphome::millis();
  const uint32_t clock_shuffle_period_ms = uint32_t((4000.0f / speed_scale) + 0.5f);

  if (animation_mode != this->last_animation_mode_) {
    this->initialize_animation_(animation);
  }

  if (!display_enabled) {
    it.fill(esphome::Color(0, 0, 0));
    return;
  }

  it.fill(esphome::Color(0, 0, 0));

  if (this->render_animation_(animation)) {
    return;
  }

  const ClockState clock_state =
      this->resolve_clock_state_(ha_now, sntp_now, debug_clock_mode, debug_clock_start_ms, now_ms);
  this->refresh_clock_samples_(clock_state, now_ms, clock_shuffle_period_ms);

  if (binary_clock_mode) {
    this->render_binary_clock_(draw, clock_state);
    return;
  }
  if (this->render_numeric_clock_(draw, clock_state, numeric_clock_mode)) {
    return;
  }
  this->render_dot_clock_(draw);
}

}  // namespace spaceclock
