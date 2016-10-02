#include <pebble.h>
#include "mission.h"
#include "../utils.h"
#define PHASE_COUNT 3
#define INFO_COUNT 3

static TextLayer *s_main_label;
static TextLayer *s_main_count;
static TextLayer *s_live_indicator;
static Layer *s_layer_live_indicator;

typedef struct Info {
  bool active;
  time_t timestamp;
  char name[3];
  char buf[6];
} info_t;

typedef enum Info_category {
  FT, TO, LD
} info_cat_t;

typedef enum Phase_type {
  PREFLIGHT, INFLIGHT, POSTFLIGHT
} phase_type_t;

static info_t s_info_roll[INFO_COUNT];

typedef struct Phase {
  void (*next)();
  void (*start)(time_t tick);
  void (*cancel)();
  void (*update)(time_t tick);
  void (*end)(time_t tick);
} phase_t;

static phase_t s_phase_list[PHASE_COUNT];

static void post_start(time_t tick) {
  s_info_roll[LD].timestamp = tick;
  format_time_hhmm(tick, s_info_roll[LD].buf, sizeof(s_info_roll[LD].buf));
  
  time_t flight_time = s_info_roll[LD].timestamp - s_info_roll[TO].timestamp;
  format_duration_hhmm(flight_time, s_info_roll[FT].buf, sizeof(s_info_roll[FT].buf));
  
  s_info_roll[LD].active = true;
  text_layer_set_background_color(s_main_count, GColorWhite);
  text_layer_set_text_color(s_main_count, GColorBlack);
}

static void post_cancel() {
  s_info_roll[LD].active = false;
  text_layer_set_background_color(s_main_count, GColorClear);
  text_layer_set_text_color(s_main_count, GColorWhite);
}

static phase_t s_postflight = {
  .next = NULL,
  .start = &post_start,
  .cancel = &post_cancel,
  .update = NULL
};

static void ft_update(time_t tick) {
  time_t flight_time = tick - s_info_roll[TO].timestamp;
  format_duration_hhmm(flight_time, s_info_roll[FT].buf, sizeof(s_info_roll[FT].buf));
  
  static bool tick_tock = false;
  tick_tock = !tick_tock;
  layer_set_hidden(s_layer_live_indicator, tick_tock);
}

static void ft_start(time_t tick) {
  s_info_roll[TO].timestamp = tick;
  format_time_hhmm(tick, s_info_roll[TO].buf, sizeof(s_info_roll[TO].buf));
  s_info_roll[TO].active = true;
}

static void ft_cancel() {
  s_info_roll[TO].active = false;
  strcpy(s_info_roll[FT].buf, "--:--");
}

static void ft_next() {
  layer_set_hidden(s_layer_live_indicator, true);
}

static phase_t s_inflight = {
  .next = &ft_next,
  .start = &ft_start,
  .cancel = &ft_cancel,
  .update = &ft_update
};

static phase_t s_preflight = {
  .next = NULL,
  .start = NULL,
  .update = NULL,
  .end = NULL
};

static phase_type_t s_current_phase = PREFLIGHT;

static info_cat_t s_current_info_cat = FT;
static info_cat_t s_default_info_cat = FT;

static void change_display() {
  text_layer_set_text(s_main_label, s_info_roll[s_current_info_cat].name);
  text_layer_set_text(s_main_count, s_info_roll[s_current_info_cat].buf);
}

void mission_init(Layer *window_layer, GRect bounds) {
  s_phase_list[PREFLIGHT] = s_preflight;
  s_phase_list[INFLIGHT] = s_inflight;
  s_phase_list[POSTFLIGHT] = s_postflight;
  
  s_info_roll[FT].active = true;
  strcpy(s_info_roll[FT].name, "FT");
  strcpy(s_info_roll[FT].buf, "--:--");
  
  s_info_roll[TO].active = false;
  strcpy(s_info_roll[TO].name, "TO");
  
  s_info_roll[LD].active = false;
  strcpy(s_info_roll[LD].name, "LD");
  
  s_main_label = configure_text_layer(window_layer, GRect(0, 60, bounds.size.w, 44), fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT), GTextAlignmentLeft);
  s_main_count = configure_text_layer(window_layer, GRect(bounds.size.w * 0.47, 73, bounds.size.w * 0.53, 30), fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD), GTextAlignmentRight);
  
  change_display();
  
  s_live_indicator = configure_text_layer(window_layer, GRect(bounds.size.w * 0.50, 65, bounds.size.w * 0.50, 9), fonts_get_system_font(FONT_KEY_GOTHIC_09), GTextAlignmentCenter);
  text_layer_set_text(s_live_indicator, "in flight");
  s_layer_live_indicator = text_layer_get_layer(s_live_indicator);
  layer_set_hidden(s_layer_live_indicator, true);
}

void mission_destroy() {
  text_layer_destroy(s_main_label);
  text_layer_destroy(s_main_count);
  text_layer_destroy(s_live_indicator);
}

void mission_update(time_t tick) {
  if (s_phase_list[s_current_phase].update != NULL) {
    s_phase_list[s_current_phase].update(tick);
  }
}

void mission_next() {
  time_t tick = time(NULL);
  if (s_current_phase < PHASE_COUNT - 1) {
    if (s_phase_list[s_current_phase].next != NULL) {
      s_phase_list[s_current_phase].next();
    }
    s_current_phase++;
    if (s_phase_list[s_current_phase].start != NULL) {
      s_phase_list[s_current_phase].start(tick);
      mission_update(tick);
    }
  }
}

static AppTimer *s_display_timer = NULL;

static void switch_to_default(void *data) {
  s_current_info_cat = s_default_info_cat;
  change_display();
}

void mission_previous() {
  if (s_phase_list[s_current_phase].cancel == NULL) {
    return;
  }
  
  s_phase_list[s_current_phase].cancel();
  s_current_phase--;
  if (!s_info_roll[s_current_info_cat].active) {
    app_timer_cancel(s_display_timer);
    switch_to_default(NULL);
  }
  mission_update(time(NULL));
}

void mission_switch_display() {
  if (s_display_timer != NULL) {
    app_timer_cancel(s_display_timer);
  }
  
  info_cat_t next = s_current_info_cat;
  do {
     next = (next + 1) % INFO_COUNT;
  } while (!s_info_roll[next].active);
  s_current_info_cat = next;
  
  change_display();
  
  if (s_current_info_cat != s_default_info_cat) {
    s_display_timer = app_timer_register(10000, switch_to_default, NULL);
  }
}