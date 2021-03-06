#include <pebble.h>
#include "mission.h"
#include "../utils.h"
#include "../windows/check_msg.h"

#define PHASE_COUNT 5
#define CRUISE_CHECK_PERIOD_IN_MINUTES 15

static TextLayer *s_main_label;
static TextLayer *s_main_count;
static Layer *s_main_count_layer;
static TextLayer *s_live_indicator;
static Layer *s_layer_live_indicator;

static AppTimer *s_display_timer = NULL;
static AppTimer *s_vibes_timer = NULL;;

typedef struct Info {
  bool active;
  time_t timestamp;
  char name[3];
  char buf[INFO_BUFFER_SIZE];
} info_t;

typedef enum Phase_type {
  PREFLIGHT, TAXI_DEP, INFLIGHT, TAXI_ARR, POSTFLIGHT
} phase_type_t;

static info_t s_info_roll[INFO_COUNT];

typedef struct Phase {
  void (*next)();
  void (*start)(time_t tick);
  void (*cancel)();
  void (*update)(time_t tick);
} phase_t;

static phase_t s_phase_list[PHASE_COUNT];

static phase_type_t s_current_phase = PREFLIGHT;

static info_cat_t s_current_info_cat = FLIGHT_TIME;
static info_cat_t s_default_info_cat = FLIGHT_TIME;

static void switch_to_default(void *data);

static void change_display() {
  text_layer_set_text(s_main_label, s_info_roll[s_current_info_cat].name);
  text_layer_set_text(s_main_count, s_info_roll[s_current_info_cat].buf);
}

/* -------------------------------------------------------------
                Flight phases implementation
   ------------------------------------------------------------- */

// Pre-flight phase definitions

static phase_t s_preflight = {
  .next = NULL,
  .start = NULL,
  .update = NULL,
  .cancel = NULL
};

// Departure taxi phase definition

static void taxi_dep_reminder(void *data) {
  vibes_short_pulse();
  s_vibes_timer = app_timer_register(30000, taxi_dep_reminder, NULL);
}

static void taxi_dep_start(time_t tick) {
  s_info_roll[OFF_BLOCK].timestamp = tick;
  format_time_hhmm(tick, s_info_roll[OFF_BLOCK].buf, sizeof(s_info_roll[OFF_BLOCK].buf));
  s_info_roll[OFF_BLOCK].active = true;
  s_vibes_timer = app_timer_register(30000, taxi_dep_reminder, NULL);
  
  s_current_info_cat = OFF_BLOCK;
  s_display_timer = app_timer_register(3000, switch_to_default, NULL);
  change_display();
}

static void taxi_dep_update(time_t tick) {
  static bool tick_tock = false;
  tick_tock = !tick_tock;
  layer_set_hidden(s_main_count_layer, tick_tock);
}

static void taxi_dep_next() {
  layer_set_hidden(s_main_count_layer, false);
  app_timer_cancel(s_vibes_timer);
}

static void taxi_dep_cancel() {
  s_info_roll[OFF_BLOCK].active = false;
  layer_set_hidden(s_main_count_layer, false);
  app_timer_cancel(s_vibes_timer);
}

static phase_t s_taxi_dep = {
  .next = &taxi_dep_next,
  .start = &taxi_dep_start,
  .update = &taxi_dep_update,
  .cancel = &taxi_dep_cancel
};

// In-flight phase definitions
static void ft_update(time_t tick) {
  s_info_roll[FLIGHT_TIME].timestamp = tick - s_info_roll[TAKE_OFF].timestamp;
  format_duration_hhmm(s_info_roll[FLIGHT_TIME].timestamp, s_info_roll[FLIGHT_TIME].buf, sizeof(s_info_roll[FLIGHT_TIME].buf));
  
  static bool tick_tock = false;
  tick_tock = !tick_tock;
  layer_set_hidden(s_layer_live_indicator, tick_tock);
  
  if (s_info_roll[ENDURANCE].active && s_info_roll[ENDURANCE].timestamp < 45 * SECONDS_PER_MINUTE) {
    if (!alarm_is_active(ALARM_ENDURANCE)) {
      alarm_display(ALARM_ENDURANCE);
      s_default_info_cat = ENDURANCE;
      s_current_info_cat = ENDURANCE;
      change_display();
    }
    layer_set_hidden(text_layer_get_layer(s_main_label), tick_tock);
  } else if (alarm_is_active(ALARM_ENDURANCE)){
    alarm_stop(ALARM_ENDURANCE);
    layer_set_hidden(text_layer_get_layer(s_main_label), false);
    s_default_info_cat = FLIGHT_TIME;
    s_current_info_cat = FLIGHT_TIME;
    change_display();
  }
}

static void ft_start(time_t tick) {
  s_info_roll[TAKE_OFF].timestamp = tick;
  format_time_hhmm(tick, s_info_roll[TAKE_OFF].buf, sizeof(s_info_roll[TAKE_OFF].buf));
  s_info_roll[TAKE_OFF].active = true;
  alarm_start(ALARM_CRUISE_CHECK);
  
  s_current_info_cat = TAKE_OFF;
  s_default_info_cat = FLIGHT_TIME;
  s_display_timer = app_timer_register(3000, switch_to_default, NULL);
  change_display();
}

static void ft_cancel() {
  s_info_roll[TAKE_OFF].active = false;
  strcpy(s_info_roll[FLIGHT_TIME].buf, "--:--");
  s_info_roll[FLIGHT_TIME].timestamp = 0;
  alarm_stop(ALARM_CRUISE_CHECK);
  s_default_info_cat = FLIGHT_TIME;
  layer_set_hidden(text_layer_get_layer(s_main_label), false);
}

static void ft_next() {
  layer_set_hidden(s_layer_live_indicator, true);
  alarm_stop(ALARM_CRUISE_CHECK);
  s_default_info_cat = FLIGHT_TIME;
  layer_set_hidden(text_layer_get_layer(s_main_label), false);
}

static phase_t s_inflight = {
  .next = &ft_next,
  .start = &ft_start,
  .cancel = &ft_cancel,
  .update = &ft_update
};

// Arrival taxi phase definitions

static void taxi_arr_reminder(void *data) {
  vibes_short_pulse();
  s_vibes_timer = app_timer_register(60000, taxi_arr_reminder, NULL);
}

static void taxi_arr_start(time_t tick) {
  s_info_roll[LANDING].timestamp = tick;
  format_time_hhmm(tick, s_info_roll[LANDING].buf, sizeof(s_info_roll[LANDING].buf));
  
  time_t flight_time = s_info_roll[LANDING].timestamp - s_info_roll[TAKE_OFF].timestamp;
  format_duration_hhmm(flight_time, s_info_roll[FLIGHT_TIME].buf, sizeof(s_info_roll[FLIGHT_TIME].buf));
  
  s_info_roll[LANDING].active = true;
  s_current_info_cat = LANDING;
  s_default_info_cat = LANDING;
  text_layer_set_background_color(s_main_count, GColorWhite);
  text_layer_set_text_color(s_main_count, GColorBlack);
  app_timer_cancel(s_display_timer);
  change_display();
  
  s_vibes_timer = app_timer_register(60000, taxi_arr_reminder, NULL);
}

static void taxi_arr_update(time_t tick) {
  static bool tick_tock = false;
  tick_tock = !tick_tock;
  layer_set_hidden(s_main_count_layer, tick_tock);
}

static void taxi_arr_next() {
  layer_set_hidden(s_main_count_layer, false);
  app_timer_cancel(s_vibes_timer);
}

static void taxi_arr_cancel() {
  s_info_roll[LANDING].active = false;
  s_default_info_cat = FLIGHT_TIME;
  text_layer_set_background_color(s_main_count, GColorClear);
  text_layer_set_text_color(s_main_count, GColorWhite);
  app_timer_cancel(s_display_timer);
  layer_set_hidden(s_main_count_layer, false);
  change_display();
  
  app_timer_cancel(s_vibes_timer);
}

static phase_t s_taxi_arr = {
  .next = &taxi_arr_next,
  .start = &taxi_arr_start,
  .cancel = &taxi_arr_cancel,
  .update = &taxi_arr_update
};

// Post-flight phase definition

static void postflight_start(time_t tick) {
  s_info_roll[ON_BLOCK].timestamp = tick;
  format_time_hhmm(tick, s_info_roll[ON_BLOCK].buf, sizeof(s_info_roll[ON_BLOCK].buf));
  s_info_roll[ON_BLOCK].active = true;
  
  time_t flight_time = s_info_roll[ON_BLOCK].timestamp - s_info_roll[OFF_BLOCK].timestamp;
  format_duration_hhmm(flight_time, s_info_roll[BLOCK_TIME].buf, sizeof(s_info_roll[BLOCK_TIME].buf));
  s_info_roll[BLOCK_TIME].active = true;
  
  s_current_info_cat = ON_BLOCK;
  s_display_timer = app_timer_register(3000, switch_to_default, NULL);
  change_display();
  
  alarm_start(ALARM_FLIGHT_PLAN);
}

static void postflight_cancel() {
  s_info_roll[ON_BLOCK].active = false;
  s_info_roll[BLOCK_TIME].active = false;
  alarm_stop(ALARM_FLIGHT_PLAN);
}

static phase_t s_postflight = {
  .next = NULL,
  .start = &postflight_start,
  .update = NULL,
  .cancel = &postflight_cancel
};

/* -------------------------------------------------------------
              Display and update logic
   ------------------------------------------------------------- */

static void init_info_item(info_cat_t cat, const char *title) {
  s_info_roll[cat].active = false;
  strcpy(s_info_roll[cat].name, title);
  s_info_roll[cat].timestamp = 0;
}

void mission_init(Layer *window_layer, GRect bounds) {
  s_phase_list[PREFLIGHT] = s_preflight;
  s_phase_list[TAXI_DEP] = s_taxi_dep;
  s_phase_list[INFLIGHT] = s_inflight;
  s_phase_list[TAXI_ARR] = s_taxi_arr;
  s_phase_list[POSTFLIGHT] = s_postflight;
  
  init_info_item(FLIGHT_TIME, "FT");
  s_info_roll[FLIGHT_TIME].active = true;
  strcpy(s_info_roll[FLIGHT_TIME].buf, "--:--");
  
  init_info_item(ENDURANCE, "EN");
  init_info_item(BLOCK_TIME, "BT");
  init_info_item(OFF_BLOCK, "OF");
  init_info_item(TAKE_OFF, "TO");
  init_info_item(LANDING, "LD");
  init_info_item(ON_BLOCK, "ON");
  
  s_main_label = configure_text_layer(window_layer, GRect(0, 60, bounds.size.w, 44), fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT), GTextAlignmentLeft);
  s_main_count = configure_text_layer(window_layer, GRect(bounds.size.w * 0.47, 73, bounds.size.w * 0.53, 30), fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD), GTextAlignmentRight);
  s_main_count_layer = text_layer_get_layer(s_main_count);
  
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

void mission_switch_display(bool to_flight_time) {
  if (s_display_timer != NULL) {
    app_timer_cancel(s_display_timer);
  }
  
  info_cat_t next = s_current_info_cat;
  if (to_flight_time) {
    next = FLIGHT_TIME;
  } else {
    do {
       next = (next + 1) % INFO_COUNT;
    } while (!s_info_roll[next].active);
  }
  s_current_info_cat = next;
  
  change_display();
  
  if (s_current_info_cat != s_default_info_cat) {
    s_display_timer = app_timer_register(10000, switch_to_default, NULL);
  }
}

time_t mission_get_timestamp(info_cat_t category) {
  info_t info = s_info_roll[category];
  return info.active ? info.timestamp : 0;
}

char * mission_get_info_buffer(info_cat_t category) {
  return s_info_roll[category].buf;
}

void mission_set_status(info_cat_t category, bool active) {
 s_info_roll[category].active = active;
}

void mission_set_timestamp(info_cat_t category, time_t timestamp) {
 s_info_roll[category].timestamp = timestamp;
}