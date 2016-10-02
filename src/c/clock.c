#include <pebble.h>
#include "clock.h"
#include "utils.h"

static TextLayer *s_date, *s_time;

void clock_init(Layer *window_layer, GRect bounds) {
  s_date = configure_text_layer(window_layer, GRect(0, 0, bounds.size.w, 44), fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT), GTextAlignmentLeft);
  s_time = configure_text_layer(window_layer, GRect(0, 0, bounds.size.w, 44), fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT), GTextAlignmentRight);
}

void clock_destroy() {
  text_layer_destroy(s_date);
  text_layer_destroy(s_time);
}

void update_clock(time_t tick) {
  struct tm *tick_time_z = gmtime(&tick);
  
  static char s_buffer_z[6];
  strftime(s_buffer_z, sizeof(s_buffer_z), "%H%M", tick_time_z);
  text_layer_set_text(s_time, s_buffer_z);
  
  static char s_buffer_date[3];
  strftime(s_buffer_date, sizeof(s_buffer_date), "%e", tick_time_z);
  text_layer_set_text(s_date, s_buffer_date);
}
