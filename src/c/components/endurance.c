#include <pebble.h>
#include "endurance.h"

// TODO implement logic (requires menu)
Layer *s_endurance_layer;
static int s_endurance_level = 100;

void endurance_init(Layer *window_layer, GRect bounds) {
  s_endurance_layer = layer_create(GRect(0, 110, bounds.size.w, 2));
  layer_set_update_proc(s_endurance_layer, endurance_update_proc);
  layer_add_child(window_layer, s_endurance_layer);
}

void endurance_destroy() {
  layer_destroy(s_endurance_layer);
}

void endurance_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  int width = (int)(float)(((float)s_endurance_level /100.0F) * bounds.size.w);
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect((bounds.size.w - width), 0, width, bounds.size.h), 0, GCornerNone);
}