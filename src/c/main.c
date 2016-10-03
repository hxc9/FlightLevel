#include <pebble.h>
#include "components/et.h"
#include "components/battery.h"
#include "components/clock.h"
#include "components/endurance.h"
#include "components/mission.h"
#include "windows/flight_menu.h"

static Window *s_main_window;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  time_t tick = time(NULL);
  update_clock(tick);
  update_elapsed_time(tick);
  mission_update(tick);
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  start_elapsed_time();
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  mission_switch_display(false);
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  mission_switch_display(true);
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  mission_next();
}

static void up_long_click_handler(ClickRecognizerRef recognizer,  void *context) {
  mission_previous();
}

static void back_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  flight_menu_window_push();
}

static void config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_long_click_subscribe(BUTTON_ID_UP, 700, up_long_click_handler, NULL);
  window_single_click_subscribe(BUTTON_ID_BACK, back_single_click_handler);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  clock_init(window_layer, bounds);
  battery_init(window_layer, bounds);
  mission_init(window_layer, bounds);
  endurance_init(window_layer, bounds);
  et_init(window_layer, bounds);
}

static void main_window_unload(Window *window) {
  clock_destroy();
  et_destroy();
  battery_destroy();
  mission_destroy();
  endurance_destroy();
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  battery_state_service_subscribe(battery_callback);
  
  window_set_click_config_provider(s_main_window, (ClickConfigProvider) config_provider);
  
  update_clock(time(NULL));
  battery_callback(battery_state_service_peek());
  start_elapsed_time();
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}