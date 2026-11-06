#include "co2_ring.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace esphome::co2_ring {
namespace {
namespace ui = tessera::ui;

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
    sensor_hint_ = c.options["sensor"] | "";
    low_threshold_ = c.options["low_threshold"] | 800.0f;
    high_threshold_ = c.options["high_threshold"] | 1200.0f;
    low_color_ = c.options["low_color"] | "green";
    mid_color_ = c.options["mid_color"] | "yellow";
    high_color_ = c.options["high_color"] | "red";
    if (high_threshold_ <= low_threshold_) high_threshold_ = low_threshold_ + 50.0f;
    ring_scale_ = c.options["ring_scale"] | 100;
    value_scale_ = c.options["value_scale"] | 180;
    label_scale_ = c.options["label_scale"] | 100;

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
    lv_obj_set_style_arc_color(arc_, lv_color_hex(0xDADDE0), LV_PART_MAIN);
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

    const int desired_zoom = std::clamp(value_scale_, 100, 200) * 256 / 100;
    tessera::Font face = tessera::Font::TITLE;
    int value_zoom = desired_zoom;
    for (tessera::Font candidate : {tessera::Font::VALUE, tessera::Font::HEADLINE, tessera::Font::TITLE}) {
      if (ui::text_width(text, candidate) * desired_zoom / 256 <= width_ - 12) {
        face = candidate;
        break;
      }
    }
    const int base_value_width = ui::text_width(text, face);
    if (base_value_width > 0 && base_value_width * value_zoom / 256 > width_ - 12)
      value_zoom = std::max(128, (width_ - 12) * 256 / base_value_width);
    ui::set_font(value_, face);
    ui::set_text(value_, text);
    lv_obj_set_style_transform_zoom(value_, value_zoom, 0);

    const int label_zoom = std::clamp(label_scale_, 75, 150) * 256 / 100;
    lv_obj_set_style_transform_zoom(unit_, label_zoom, 0);
    lv_obj_set_style_transform_zoom(center_, label_zoom, 0);
    ui::set_text(unit_, available ? plugin_->text("ppm") : "");
    ui::set_text(center_, available ? "CO2" : "");

    const int value_height = (ui::line_height(face) * value_zoom + 255) / 256;
    const int label_height = (ui::line_height(tessera::Font::BODY) * label_zoom + 255) / 256;
    const int unit_height = available ? label_height : 0;
    const int ring_limit = std::max(48, std::min(width_ - 10, height_ - value_height - unit_height - 18));
    ring_size_ = std::max(48, ring_limit * std::clamp(ring_scale_, 50, 100) / 100);
    ring_x_ = (width_ - ring_size_) / 2;
    ring_y_ = std::max(0, (ring_limit - ring_size_) / 2);
    lv_obj_set_size(arc_, ring_size_, ring_size_);
    lv_obj_set_pos(arc_, ring_x_, ring_y_);
    const int arc_width = std::max(6, ring_size_ / 14);
    lv_obj_set_style_arc_width(arc_, arc_width, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_, arc_width, LV_PART_INDICATOR);

    const int center_height = label_height;
    lv_obj_set_width(center_, ring_size_ * 3 / 5);
    lv_obj_set_pos(center_, ring_x_ + ring_size_ / 5, ring_y_ + (ring_size_ - center_height) / 2);
    lv_obj_set_height(center_, center_height);
    lv_obj_set_width(value_, width_);
    const int value_y = std::min(height_ - value_height - unit_height, ring_y_ + ring_size_ + 4);
    lv_obj_set_pos(value_, 0, std::max(0, value_y));
    lv_obj_set_height(value_, value_height + 2);
    lv_obj_set_width(unit_, width_);
    lv_obj_set_pos(unit_, 0, std::max(0, value_y + value_height - 2));
    lv_obj_set_height(unit_, std::max(1, unit_height));

    if (available) {
      // The ring fills over the 400–2000 ppm range; the selected threshold colours mark the three bands.
      const float fraction = std::clamp((ppm_ - 400.0f) / 1600.0f, 0.0f, 1.0f);
      const lv_color_t color = named_color(ppm_ < low_threshold_ ? low_color_ :
                                           (ppm_ < high_threshold_ ? mid_color_ : high_color_));
      lv_arc_set_value(arc_, static_cast<int>(std::lround(fraction * 100.0f)));
      lv_obj_set_style_arc_color(arc_, color, LV_PART_INDICATOR);
      lv_obj_set_style_text_color(center_, color, 0);
      ui::set_color(value_, theme::INK);
    } else {
      lv_arc_set_value(arc_, 0);
      lv_obj_set_style_arc_color(arc_, lv_color_hex(0xA6ABB0), LV_PART_INDICATOR);
      ui::set_color(center_, theme::MUTED);
      ui::set_color(value_, theme::MUTED);
    }
  }
  void on_theme() override {
    ui::set_color(unit_, theme::MUTED);
    if (std::isfinite(ppm_)) {
      ui::set_color(value_, theme::INK);
      lv_obj_set_style_text_color(center_, named_color(ppm_ < low_threshold_ ? low_color_ : (ppm_ < high_threshold_ ? mid_color_ : high_color_)), 0);
      lv_obj_set_style_arc_color(arc_, named_color(ppm_ < low_threshold_ ? low_color_ : (ppm_ < high_threshold_ ? mid_color_ : high_color_)), LV_PART_INDICATOR);
    }
  }

 private:
  const tessera::Plugin *plugin_;
  int width_ = 0, height_ = 0, ring_size_ = 0, ring_x_ = 0, ring_y_ = 0;
  std::string sensor_hint_;
  float ppm_ = NAN;
  float low_threshold_ = 800.0f, high_threshold_ = 1200.0f;
  int ring_scale_ = 100, value_scale_ = 180, label_scale_ = 100;
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






