#include <pebble.h>
#include "mission.h"
#include "utils.h"

static TextLayer *s_main_label;
static TextLayer *s_main_count;

void mission_init(Layer *window_layer, GRect bounds) {
  s_main_label = configure_text_layer(window_layer, GRect(0, 60, bounds.size.w, 44), fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT), GTextAlignmentLeft);
  text_layer_set_text(s_main_label, "FT");
  s_main_count = configure_text_layer(window_layer, GRect(0, 70, bounds.size.w, 30), fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD), GTextAlignmentRight);
  text_layer_set_text(s_main_count, "00:00");
}

void mission_destroy() {
  text_layer_destroy(s_main_label);
  text_layer_destroy(s_main_count);
}