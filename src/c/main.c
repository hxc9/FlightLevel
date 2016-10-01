#include <pebble.h>

static Window *s_main_window;

static TextLayer *s_date_layer;
static TextLayer *s_time_layer_u;

static Layer *s_battery_layer;
static int s_battery_level;

static TextLayer *s_main_label;
static TextLayer *s_main_count;

static Layer *s_endurance_layer;
static int s_endurance_level = 100;

static TextLayer *s_et_label;
static TextLayer *s_et_count;
static TextLayer *s_et_desc;

static time_t s_et_start;

static void update_time(time_t tick) {
  struct tm *tick_time_z = gmtime(&tick);
  
  static char s_buffer_z[6];
  strftime(s_buffer_z, sizeof(s_buffer_z), "%H%M", tick_time_z);
  text_layer_set_text(s_time_layer_u, s_buffer_z);
  
  static char s_buffer_date[3];
  strftime(s_buffer_date, sizeof(s_buffer_date), "%d", tick_time_z);
  text_layer_set_text(s_date_layer, s_buffer_date);
}

static void update_elapsed_time(time_t tick) {
  int elapsed_time = tick - s_et_start;
  int seconds = (int)elapsed_time % 60;
  int minutes = (int)elapsed_time / 60 % 100;
  
  static char et_buffer[] = "00:00";
  snprintf(et_buffer, sizeof(et_buffer), "%d:%02d", minutes, seconds);
  text_layer_set_text(s_et_count, et_buffer);
}

static void start_elapsed_time() {
  s_et_start = time(NULL);
  struct tm *tick_time_z = gmtime(&s_et_start);
  
  static char et_start_buffer[] = "xx";
  snprintf(et_start_buffer, sizeof(et_start_buffer), "%02d", tick_time_z->tm_min);
  text_layer_set_text(s_et_label, et_start_buffer);
  
  update_elapsed_time(s_et_start);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  time_t tick = time(NULL);
  update_time(tick);
  update_elapsed_time(tick);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  int width = (int)(float)(((float)s_battery_level /100.0F) * bounds.size.w);
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect((bounds.size.w - width) / 2, 0, width, bounds.size.h), 0, GCornerNone);
}

static void endurance_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  int width = (int)(float)(((float)s_endurance_level /100.0F) * bounds.size.w);
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect((bounds.size.w - width), 0, width, bounds.size.h), 0, GCornerNone);
}

static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  layer_mark_dirty(s_battery_layer);
}

static TextLayer *configure_text_layer(Layer *window_layer, GRect box, GFont font, GTextAlignment alignment) {
  TextLayer *layer = text_layer_create(box);
  
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_text_color(layer, GColorWhite);
  text_layer_set_font(layer, font);
  text_layer_set_text_alignment(layer, alignment);
  
  layer_add_child(window_layer, text_layer_get_layer(layer));
  
  return layer;
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  start_elapsed_time();
}

static void config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  GRect bounds = layer_get_bounds(window_layer);
  
  s_time_layer_u = configure_text_layer(window_layer, GRect(0, 0, bounds.size.w, 44), fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT), GTextAlignmentRight);
  s_date_layer = configure_text_layer(window_layer, GRect(0, 0, bounds.size.w, 44), fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT), GTextAlignmentLeft);
  
  s_battery_layer = layer_create(GRect(0, 55, bounds.size.w, 2));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(window_layer, s_battery_layer);
  
  s_main_label = configure_text_layer(window_layer, GRect(0, 60, bounds.size.w, 44), fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT), GTextAlignmentLeft);
  text_layer_set_text(s_main_label, "FT");
  s_main_count = configure_text_layer(window_layer, GRect(0, 70, bounds.size.w, 30), fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD), GTextAlignmentRight);
  text_layer_set_text(s_main_count, "00:00");
  
  s_endurance_layer = layer_create(GRect(0, 110, bounds.size.w, 2));
  layer_set_update_proc(s_endurance_layer, endurance_update_proc);
  layer_add_child(window_layer, s_endurance_layer);
  
  s_et_label = configure_text_layer(window_layer, GRect(0, 115, bounds.size.w, 44), fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS), GTextAlignmentLeft);
  s_et_count = configure_text_layer(window_layer, GRect(0, 115, bounds.size.w, 44), fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS), GTextAlignmentRight);
  s_et_desc = configure_text_layer(window_layer, GRect(10, bounds.size.h - 26, 20, 26), fonts_get_system_font(FONT_KEY_GOTHIC_24), GTextAlignmentLeft);
  text_layer_set_text(s_et_desc, "ET");
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer_u);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_main_label);
  text_layer_destroy(s_main_count);
  layer_destroy(s_endurance_layer);
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
  
  update_time(true);
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