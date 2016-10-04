#include <pebble.h>
#include "et.h"
#include "../utils.h"

static time_t s_et_start;

static TextLayer *s_counter;
static TextLayer *s_start_minute;

static TextLayer *s_desc;

void et_init(Layer *window_layer, GRect bounds) {
  s_counter = configure_text_layer(window_layer, GRect(0, 115, bounds.size.w, 44), fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS), GTextAlignmentRight);
  s_start_minute = configure_text_layer(window_layer, GRect(0, 115, bounds.size.w, 44), fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS), GTextAlignmentLeft);
  s_desc = configure_text_layer(window_layer, GRect(10, bounds.size.h - 26, 20, 26), fonts_get_system_font(FONT_KEY_GOTHIC_24), GTextAlignmentLeft);
  text_layer_set_text(s_desc, "ET");
}

void et_destroy() {
  text_layer_destroy(s_counter);
  text_layer_destroy(s_start_minute);
  text_layer_destroy(s_desc);
}

void elapsed_time_update(time_t tick) {
  int elapsed_time = tick - s_et_start;
  int seconds = (int)elapsed_time % 60;
  int minutes = (int)elapsed_time / 60 % 100;
  
  static char et_buffer[] = "00:00";
  snprintf(et_buffer, sizeof(et_buffer), "%d:%02d", minutes, seconds);
  text_layer_set_text(s_counter, et_buffer);
  
  if (elapsed_time > 60 * 100) {
    text_layer_set_text(s_start_minute, "");
  }
}

void elapsed_time_flyback() {
  s_et_start = time(NULL);
  struct tm *tick_time_z = gmtime(&s_et_start);
  
  static char et_start_buffer[] = "xx";
  snprintf(et_start_buffer, sizeof(et_start_buffer), "%02d", tick_time_z->tm_min);
  text_layer_set_text(s_start_minute, et_start_buffer);
  
  elapsed_time_update(s_et_start);
}