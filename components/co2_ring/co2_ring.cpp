#include "co2_ring.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace esphome::co2_ring {
namespace {
namespace ui = tessera::ui;

struct TextLayout {
  int x;
  int y;
  int size;
};

static TextLayout parse_text_layout(const std::string &text, TextLayout fallback) {
  int x = fallback.x;
  int y = fallback.y;
  int size = fallback.size;
  if (std::sscanf(text.c_str(), "%d;%d;%d", &x, &y, &size) != 3) return fallback;
  return {std::clamp(x, -50, 50), std::clamp(y, -50, 50), std::clamp(size, 50, 250)};
}

static void parse_pair(const std::string &text, float &first, float &second) {
  float parsed_first = first;
  float parsed_second = second;
  if (std::sscanf(text.c_str(), "%f;%f", &parsed_first, &parsed_second) == 2) {
    first = parsed_first;
    second = parsed_second;
  }
}

static lv_color_t named_color(const std::string &name) {
  if (name == "green") return lv_color_hex(0x42B85A);
  if (name == "lime") return lv_color_hex(0x9BCB3C);
  if (name == "teal") return lv_color_hex(0x26A69A);
  if (name == "blue") return lv_color_hex(0x4386D8);
  if (name == "orange") return lv_color_hex(0xE68A2E);
  if (name == "red") return lv_color_hex(0xD94A4A);
  if (name == "purple") return lv_color_hex(0x9059B5);
  return lv_color_hex(0xE1BD32);  // yellow, and the safe default
}

class Co2Tile : public tessera::Tile {
 public:
  explicit Co2Tile(const tessera::Plugin *plugin) : plugin_(plugin) {}

  void create(const tessera::TileContext &c) override {
    width_ = c.width;
    height_ = c.height;
    lv_obj_set_style_bg_color(c.parent, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(c.parent, LV_OPA_COVER, 0);
    sensor_hint_ = c.options["sensor"] | "";
    parse_pair(c.options["thresholds"] | "800;1200", low_threshold_, high_threshold_);
    low_threshold_ = std::clamp(low_threshold_, 400.0f, 3000.0f);
    high_threshold_ = std::clamp(high_threshold_, 450.0f, 5000.0f);
    parse_pair(c.options["ring_range"] | "400;2000", ring_min_, ring_max_);
    ring_min_ = std::clamp(ring_min_, 0.0f, 5000.0f);
    ring_max_ = std::clamp(ring_max_, 500.0f, 10000.0f);
    if (ring_max_ <= ring_min_) ring_max_ = ring_min_ + 50.0f;
    low_color_ = c.options["low_color"] | "green";
    mid_color_ = c.options["mid_color"] | "yellow";
    high_color_ = c.options["high_color"] | "red";
    if (high_threshold_ <= low_threshold_) high_threshold_ = low_threshold_ + 50.0f;
    ring_scale_ = c.options["ring_scale"] | 100;
    value_layout_ = parse_text_layout(c.options["value_layout"] | "0;15;180", {0, 15, 180});
    co2_layout_ = parse_text_layout(c.options["co2_layout"] | "0;0;100", {0, 0, 100});
    unit_layout_ = parse_text_layout(c.options["unit_layout"] | "0;0;100", {0, 0, 100});
    value_layout_.size = std::clamp(value_layout_.size, 100, 200);
    co2_layout_.size = std::clamp(co2_layout_.size, 75, 150);
    unit_layout_.size = std::clamp(unit_layout_.size, 75, 150);

    // Gauge size follows the tile dimensions, with the large value kept below it.
    ring_size_ = std::max(48, std::min(width_ - 10, height_ - 64));
    ring_x_ = (width_ - ring_size_) / 2;
    ring_y_ = std::max(0, (height_ - ring_size_ - 58) / 2);

    arc_ = lv_arc_create(c.parent);
    lv_obj_set_size(arc_, ring_size_, ring_size_);
    lv_obj_set_pos(arc_, ring_x_, ring_y_);
    lv_arc_set_range(arc_, 0, 100);
    lv_arc_set_bg_angles(arc_, 0, 270);
    lv_arc_set_rotation(arc_, 135);
    lv_arc_set_mode(arc_, LV_ARC_MODE_NORMAL);
    lv_obj_set_style_arc_width(arc_, std::max(8, ring_size_ / 14), LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_, std::max(8, ring_size_ / 14), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc_, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_opa(arc_, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_remove_flag(arc_, LV_OBJ_FLAG_CLICKABLE);

    center_ = ui::label(c.parent, tessera::Font::BODY, theme::INK);
    lv_obj_set_width(center_, ring_size_ * 3 / 5);
    lv_obj_set_style_text_align(center_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(center_, ring_x_ + ring_size_ / 5, ring_y_ + (ring_size_ - ui::line_height(tessera::Font::BODY)) / 2);
    lv_obj_set_height(center_, ui::line_height(tessera::Font::BODY));

    value_ = ui::label(c.parent, tessera::Font::VALUE, theme::INK);
    lv_obj_set_width(value_, width_);
    lv_obj_set_style_text_align(value_, LV_TEXT_ALIGN_CENTER, 0);
    unit_ = ui::label(c.parent, tessera::Font::BODY, theme::MUTED);
    lv_obj_set_width(unit_, width_);
    lv_obj_set_style_text_align(unit_, LV_TEXT_ALIGN_CENTER, 0);
  }

  void on_state(JsonObjectConst data) override {
    ppm_ = NAN;
    if (!data["wait"].isNull() || sensor_hint_.empty()) return;
    JsonVariantConst sensor = data["entities"]["sensor"];
    if (sensor.isNull()) return;
    const char *state = sensor["state"] | "";
    if (!state[0] || std::string(state) == "unknown" || std::string(state) == "unavailable") return;
    char *end = nullptr;
    const float parsed = std::strtof(state, &end);
    if (end != state && std::isfinite(parsed)) ppm_ = parsed;
  }

  void on_tick(uint32_t) override {
    const bool available = std::isfinite(ppm_);
    const std::string text = available ? std::to_string(static_cast<int>(std::lround(ppm_))) :
        (sensor_hint_.empty() ? plugin_->text("choose_sensor") : plugin_->text("unavailable"));
    const bool centered_gauge = width_ >= 220 && height_ >= 145;
    const int desired_value_zoom = std::clamp(value_layout_.size, 100, 200) * 256 / 100;
    const int co2_zoom = co2_layout_.size * 256 / 100;
    const int unit_zoom = unit_layout_.size * 256 / 100;
    const int co2_height = (ui::line_height(tessera::Font::BODY) * co2_zoom + 255) / 256;
    const int header_bottom = 4 + co2_height + 8;
    lv_obj_set_style_transform_zoom(unit_, unit_zoom, 0);
    lv_obj_set_style_transform_zoom(center_, co2_zoom, 0);
    ui::set_text(unit_, available ? plugin_->text("ppm") : "");
    ui::set_text(center_, available ? "CO2" : "");

    int ring_limit;
    if (centered_gauge) {
      ring_limit = std::max(48, std::min(width_ - 10, height_ - header_bottom - 8));
    } else {
      // First fit the compact number; the ring uses whatever height remains above it.
      ring_limit = 48;
    }
    if (centered_gauge)
      ring_size_ = std::max(48, ring_limit * std::clamp(ring_scale_, 50, 100) / 100);

    const int desired_zoom = desired_value_zoom;
    const int fit_width = centered_gauge ? ring_size_ * 68 / 100 : width_ - 12;
    const int fit_height = centered_gauge ? ring_size_ * 34 / 100 : height_ / 2;
    tessera::Font face = tessera::Font::VALUE;
    int rendered_value_zoom = 128;
    int best_height = 0;
    for (tessera::Font candidate : {tessera::Font::VALUE, tessera::Font::HEADLINE, tessera::Font::TITLE}) {
      const int base_width = ui::text_width(text, candidate);
      const int base_height = ui::line_height(candidate);
      if (base_width <= 0 || base_height <= 0) continue;
      const int width_zoom = fit_width * 256 / base_width;
      const int height_zoom = fit_height * 256 / base_height;
      const int candidate_zoom = std::min(desired_zoom, std::min(width_zoom, height_zoom));
      const int rendered_height = base_height * candidate_zoom / 256;
      if (candidate_zoom >= 128 && rendered_height > best_height) {
        face = candidate;
        rendered_value_zoom = candidate_zoom;
        best_height = rendered_height;
      }
    }
    ui::set_font(value_, face);
    ui::set_text(value_, text);
    lv_obj_set_style_transform_zoom(value_, rendered_value_zoom, 0);

    const int value_height = (ui::line_height(face) * rendered_value_zoom + 255) / 256;
    const int unit_height = available ? (ui::line_height(tessera::Font::BODY) * unit_zoom + 255) / 256 : 0;
    const int value_dx = width_ * value_layout_.x / 100;
    const int value_dy = height_ * value_layout_.y / 100;
    const int co2_dx = width_ * co2_layout_.x / 100;
    const int co2_dy = height_ * co2_layout_.y / 100;
    const int unit_dx = width_ * unit_layout_.x / 100;
    const int unit_dy = height_ * unit_layout_.y / 100;
    int ring_area_top = header_bottom;
    int ring_area_height = 0;
    if (!centered_gauge) {
      ring_area_height = std::max(0, height_ - ring_area_top - value_height - unit_height - 18);
      ring_limit = std::max(48, std::min(width_ - 10, ring_area_height));
      ring_size_ = std::max(48, ring_limit * std::clamp(ring_scale_, 50, 100) / 100);
    }
    if (centered_gauge)
      ring_area_height = std::max(0, height_ - ring_area_top - 8);
    ring_x_ = (width_ - ring_size_) / 2;
    ring_y_ = ring_area_top + std::max(0, (ring_area_height - ring_size_) / 2);
    lv_obj_set_size(arc_, ring_size_, ring_size_);
    lv_obj_set_pos(arc_, ring_x_, ring_y_);
    const int arc_width = std::max(6, ring_size_ / 14);
    lv_obj_set_style_arc_width(arc_, arc_width, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_, arc_width, LV_PART_INDICATOR);

    if (centered_gauge) {
      // CO2 sits above and outside the ring; the value and ppm stay together in the middle.
      const int value_width = std::max(1, ring_size_ * 68 / 100);
      lv_obj_set_width(value_, value_width);
      lv_obj_set_style_text_align(value_, LV_TEXT_ALIGN_CENTER, 0);
      const int group_height = value_height + unit_height;
      const int value_y = ring_y_ + (ring_size_ - group_height) / 2;
      lv_obj_set_pos(value_, ring_x_ + (ring_size_ - value_width) / 2 + value_dx, value_y + value_dy);
      lv_obj_set_height(value_, value_height + 2);
      lv_obj_set_width(unit_, value_width);
      lv_obj_set_style_text_align(unit_, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_pos(unit_, ring_x_ + (ring_size_ - value_width) / 2 + value_dx + unit_dx,
                     value_y + value_dy + value_height - 1 + unit_dy);
      lv_obj_set_height(unit_, std::max(1, unit_height));
      lv_obj_set_width(center_, std::max(1, width_ - 8));
      lv_obj_set_style_text_align(center_, LV_TEXT_ALIGN_LEFT, 0);
      lv_obj_set_pos(center_, 4 + co2_dx, 4 + co2_dy);
      lv_obj_set_height(center_, co2_height);
    } else {
      // Compact tiles keep CO2 above the ring and stack the value below it.
      lv_obj_set_width(center_, std::max(1, width_ - 8));
      lv_obj_set_style_text_align(center_, LV_TEXT_ALIGN_LEFT, 0);
      lv_obj_set_pos(center_, 4 + co2_dx, 4 + co2_dy);
      lv_obj_set_height(center_, co2_height);
      lv_obj_set_width(value_, width_);
      lv_obj_set_style_text_align(value_, LV_TEXT_ALIGN_CENTER, 0);
      const int value_y = std::min(height_ - value_height - unit_height, ring_y_ + ring_size_ + 4);
      lv_obj_set_pos(value_, value_dx, std::max(0, value_y + value_dy));
      lv_obj_set_height(value_, value_height + 2);
      lv_obj_set_width(unit_, width_);
      lv_obj_set_style_text_align(unit_, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_pos(unit_, value_dx + unit_dx,
                     std::max(0, value_y + value_dy + value_height - 2 + unit_dy));
      lv_obj_set_height(unit_, std::max(1, unit_height));
    }

    // Keep scaling anchored at the value box's centre so a larger font does
    // not visually drift sideways into the gauge.
    lv_obj_set_style_transform_pivot_x(value_, lv_obj_get_width(value_) / 2, 0);
    lv_obj_set_style_transform_pivot_y(value_, lv_obj_get_height(value_) / 2, 0);
    lv_obj_set_style_transform_pivot_x(center_, lv_obj_get_width(center_) / 2, 0);
    lv_obj_set_style_transform_pivot_y(center_, lv_obj_get_height(center_) / 2, 0);
    lv_obj_set_style_transform_pivot_x(unit_, lv_obj_get_width(unit_) / 2, 0);
    lv_obj_set_style_transform_pivot_y(unit_, lv_obj_get_height(unit_) / 2, 0);

    if (available) {
      // The configured ring range sets progress; colour thresholds select its three colours.
      const float fraction = std::clamp((ppm_ - ring_min_) / (ring_max_ - ring_min_), 0.0f, 1.0f);
      const lv_color_t color = named_color(ppm_ < low_threshold_ ? low_color_ :
                                           (ppm_ < high_threshold_ ? mid_color_ : high_color_));
      lv_arc_set_value(arc_, static_cast<int>(std::lround(fraction * 100.0f)));
      lv_obj_set_style_arc_color(arc_, color, LV_PART_INDICATOR);
      ui::set_color(value_, theme::INK);
      ui::set_color(center_, theme::MUTED);
    } else {
      lv_arc_set_value(arc_, 0);
      lv_obj_set_style_arc_color(arc_, lv_color_hex(0xA6ABB0), LV_PART_INDICATOR);
      ui::set_color(center_, theme::MUTED);
      ui::set_color(value_, theme::MUTED);
    }
    ui::set_color(unit_, theme::MUTED);
  }
  void on_theme() override {
    ui::set_color(unit_, theme::MUTED);
    ui::set_color(center_, theme::MUTED);
    ui::set_color(value_, std::isfinite(ppm_) ? theme::INK : theme::MUTED);
    if (std::isfinite(ppm_)) {
      const lv_color_t color = named_color(ppm_ < low_threshold_ ? low_color_ :
                                           (ppm_ < high_threshold_ ? mid_color_ : high_color_));
      lv_obj_set_style_arc_color(arc_, color, LV_PART_INDICATOR);
    }
  }
 private:
  const tessera::Plugin *plugin_;
  int width_ = 0, height_ = 0, ring_size_ = 0, ring_x_ = 0, ring_y_ = 0;
  std::string sensor_hint_;
  float ppm_ = NAN;
  float low_threshold_ = 800.0f, high_threshold_ = 1200.0f;
  float ring_min_ = 400.0f, ring_max_ = 2000.0f;
  int ring_scale_ = 100;
  TextLayout value_layout_{0, 15, 180}, co2_layout_{0, 0, 100}, unit_layout_{0, 0, 100};
  std::string low_color_ = "green", mid_color_ = "yellow", high_color_ = "red";
  lv_obj_t *arc_ = nullptr;
  lv_obj_t *center_ = nullptr;
  lv_obj_t *value_ = nullptr;
  lv_obj_t *unit_ = nullptr;
};
}  // namespace

void Co2Ring::setup() {
  add_tile("co2", [this]() { return new Co2Tile(this); });
}

}  // namespace esphome::co2_ring









