#include <pebble.h>
#include "mission.h"
#include "utils.h"

static TextLayer *s_main_label;
static TextLayer *s_main_count;
static TextLayer *s_live_indicator;
static Layer *s_layer_live_indicator;

typedef struct Phase {
  struct Phase *(*next)();
  void (*start)(time_t tick);
  void (*update)(time_t tick);
  void (*end)(time_t tick);
} phase_t;

static void post_start(time_t tick) {
  text_layer_set_background_color(s_main_count, GColorWhite);
  text_layer_set_text_color(s_main_count, GColorBlack);
}

static phase_t s_postflight = {
  .next = NULL,
  .start = &post_start,
  .update = NULL
};

static time_t s_ft_start;

static void ft_update(time_t tick) {
  int flight_time = tick - s_ft_start;
  int minutes = (int)flight_time / 60 % 60;
  int hours = (int)flight_time / 3600 % 1000;
  
  static char ft_buffer[] = "000:00";
  snprintf(ft_buffer, sizeof(ft_buffer), "%d:%02d", hours, minutes);
  text_layer_set_text(s_main_count, ft_buffer);
  
  static bool tick_tock = false;
  tick_tock = !tick_tock;
  layer_set_hidden(s_layer_live_indicator, tick_tock);
}

static void ft_start(time_t tick) {
  s_ft_start = tick;
  ft_update(s_ft_start);
}

static phase_t *ft_next() {
  layer_set_hidden(s_layer_live_indicator, true);
  return &s_postflight;
}

static phase_t s_inflight = {
  .next = &ft_next,
  .start = ft_start,
  .update = ft_update
};

static phase_t *pre_next() {
  return &s_inflight;
}

static phase_t s_preflight = {
  .next = &pre_next,
  .start = NULL,
  .update = NULL,
  .end = NULL
};

static phase_t *s_current_phase = &s_preflight;

void mission_init(Layer *window_layer, GRect bounds) {
  s_main_label = configure_text_layer(window_layer, GRect(0, 60, bounds.size.w, 44), fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT), GTextAlignmentLeft);
  text_layer_set_text(s_main_label, "FT");
  s_main_count = configure_text_layer(window_layer, GRect(bounds.size.w * 0.36, 73, bounds.size.w * 0.64, 30), fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD), GTextAlignmentRight);
  text_layer_set_text(s_main_count, "--:--");
  
  s_live_indicator = configure_text_layer(window_layer, GRect(bounds.size.w * 0.50, 65, bounds.size.w * 0.50, 9), fonts_get_system_font(FONT_KEY_GOTHIC_09), GTextAlignmentCenter);
  text_layer_set_text(s_live_indicator, "in flight");
  
  s_layer_live_indicator = text_layer_get_layer(s_live_indicator);
  layer_set_hidden(s_layer_live_indicator, true);
}

void mission_destroy() {
  text_layer_destroy(s_main_label);
  text_layer_destroy(s_main_count);
}

void mission_update(time_t tick) {
  if (s_current_phase->update != NULL) {
    s_current_phase->update(tick);
  }
}

void mission_next() {
  time_t tick = time(NULL);
  if (s_current_phase->next != NULL) {
    s_current_phase = s_current_phase->next();
    if (s_current_phase->start != NULL) {
      s_current_phase->start(tick);
    }
  }
}

void mission_previous() {
  // TODO 
}