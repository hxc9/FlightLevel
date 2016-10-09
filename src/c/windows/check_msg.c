#include <pebble.h>
#include "check_msg.h"
#define ALARM_TYPE_COUNT 2

typedef struct Alarm {
  bool inhibited;
  bool active;
  bool important;
  char text[45];
  time_t delay;
  time_t hide_delay;
  AppTimer *timer;
} alarm_t;

static alarm_t s_cruise_check = {
  .inhibited = false,
  .active = false,
  .important = false,
  .text = "Cruise check reminder",
  .delay = 15 * SECONDS_PER_MINUTE * 1000,
  .hide_delay = 2 * SECONDS_PER_MINUTE *1000,
  .timer = NULL
  };

static alarm_t s_endurance_alarm = {
  .inhibited = false,
  .active = false,
  .important = true,
  .text = "Fuel reserve low",
  .delay = 15 * SECONDS_PER_MINUTE * 1000,
  .hide_delay = 15 * SECONDS_PER_MINUTE *1000,
  .timer = NULL
  };

static alarm_t s_flight_plan = {
  .inhibited = true,
  .active = false,
  .important = false,
  .text = "Have you closed your flight plan ?",
  .delay = 5 * SECONDS_PER_MINUTE * 1000,
  .hide_delay = 10 * SECONDS_PER_MINUTE * 1000,
  .timer = NULL};

static alarm_t *s_alarm_defs[] = {&s_cruise_check, &s_endurance_alarm, &s_flight_plan};

static Window *s_main_window;
static TextLayer *s_label_layer;
static BitmapLayer *s_icon_layer;
static ActionBarLayer *s_action_bar_layer;

static GBitmap *s_icon_bitmap, *s_tick_bitmap, *s_danger_bitmap;

static AppTimer *s_display_timer;
static alarm_t *s_last_alarm;

void click_handler(ClickRecognizerRef recognizer, void *context) {
  window_stack_pop(true);
}

void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) click_handler);
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CONFIRM);
  s_danger_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DANGER);

  const GEdgeInsets icon_insets = {.top = 1, .right = 28, .bottom = 66, .left = 14};
  s_icon_layer = bitmap_layer_create(grect_inset(bounds, icon_insets));
  bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));

  const GEdgeInsets label_insets = {.top = 90, .right = ACTION_BAR_WIDTH , .left = ACTION_BAR_WIDTH / 2};
  s_label_layer = text_layer_create(grect_inset(bounds, label_insets));
  text_layer_set_background_color(s_label_layer, GColorClear);
  text_layer_set_text_alignment(s_label_layer, GTextAlignmentCenter);
  text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_label_layer));

  s_tick_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TICK);

  s_action_bar_layer = action_bar_layer_create();
  action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_SELECT, s_tick_bitmap);
  action_bar_layer_add_to_window(s_action_bar_layer, window);
  action_bar_layer_set_click_config_provider(s_action_bar_layer, click_config_provider);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_label_layer);
  action_bar_layer_destroy(s_action_bar_layer);
  bitmap_layer_destroy(s_icon_layer);

  gbitmap_destroy(s_icon_bitmap);
  gbitmap_destroy(s_tick_bitmap);
  gbitmap_destroy(s_danger_bitmap);

  window_destroy(window);
  s_main_window = NULL;
}

static void window_expired(void *data) {
  window_stack_pop(true);
}

static void window_appear(Window *window) {
  s_display_timer = app_timer_register(s_last_alarm->hide_delay, window_expired, NULL);
  text_layer_set_text(s_label_layer, s_last_alarm->text);
  bitmap_layer_set_bitmap(s_icon_layer, s_last_alarm->important ? s_danger_bitmap : s_icon_bitmap);
}

static void window_disappear(Window *window) {
  if (s_display_timer != NULL) {
    app_timer_cancel(s_display_timer);
    s_display_timer = NULL;
  } 
}

static void dialog_choice_window_push() {
  if(!s_main_window) {
    s_main_window = window_create();
    window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorJaegerGreen, GColorWhite));
    window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
      .appear = window_appear,
      .disappear = window_disappear,
    });
  }
  window_stack_push(s_main_window, true);
}

static void alarm_callback(void* data) {
  alarm_t *alarm = (alarm_t *) data;
  s_last_alarm = alarm;
  if (s_main_window == window_stack_get_top_window()) {
    text_layer_set_text(s_label_layer, s_last_alarm->text);
    if (alarm->timer != NULL) {
      app_timer_reschedule(s_display_timer, s_last_alarm->hide_delay);
    }
  } else {
    dialog_choice_window_push();
  }
  alarm->important ? vibes_long_pulse() : vibes_double_pulse();
  alarm->timer = app_timer_register(alarm->delay, alarm_callback, alarm);
}

static void alarm_do_start(alarm_t *alarm, bool now) {
  if (alarm->timer != NULL) {
    app_timer_cancel(alarm->timer);
    alarm->timer = NULL;
  }
  alarm->timer = app_timer_register(now ? 0 : alarm->delay, alarm_callback, alarm);
}

static void alarm_start_impl(alarm_type type, bool now) {
  alarm_t *alarm = s_alarm_defs[type];
  alarm->active = true;
  if (!alarm->inhibited) {
    alarm_do_start(alarm, now);
  }
}

void alarm_start(alarm_type type) {
  alarm_start_impl(type, false);
}

void alarm_display(alarm_type type) {
  alarm_start_impl(type, true);
}

void alarm_stop(alarm_type type) {
  alarm_t *alarm = s_alarm_defs[type];
  alarm->active = false;
  if (alarm->timer != NULL) {
  app_timer_cancel(alarm->timer);
  alarm->timer = NULL;
  }
}

void alarm_inhibit(alarm_type type) {
  alarm_t *alarm = s_alarm_defs[type];
  alarm->inhibited = true;
  if (alarm->timer != NULL) {
    app_timer_cancel(alarm->timer);
    alarm->timer = NULL;
  }
}

void alarm_enable(alarm_type type) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Enable...");
  alarm_t *alarm = s_alarm_defs[type];
  alarm->inhibited = false;
  if (alarm->active) {
    alarm_do_start(alarm, false);
  }
}

bool alarm_is_inhibited(alarm_type type) {
  return s_alarm_defs[type]->inhibited;
}

bool alarm_is_active(alarm_type type) {
  return s_alarm_defs[type]->active;
}