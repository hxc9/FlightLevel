#include <pebble.h>
#include "battery.h"

static Layer *s_battery_layer;
static int s_battery_level;

void battery_init(Layer *window_layer, GRect bounds) {
  s_battery_layer = layer_create(GRect(0, 58, bounds.size.w, 2));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(window_layer, s_battery_layer);
}

void battery_destroy() {
  layer_destroy(s_battery_layer);
}

void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  int width = (int)(float)(((float)s_battery_level /100.0F) * bounds.size.w);
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect((bounds.size.w - width) / 2, 0, width, bounds.size.h), 0, GCornerNone);
}

void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  layer_mark_dirty(s_battery_layer);
}