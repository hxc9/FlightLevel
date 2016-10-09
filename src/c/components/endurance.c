#include <pebble.h>
#include "endurance.h"
#include "mission.h"
#include "../utils.h"

Layer *s_endurance_layer;
static time_t s_endurance_at_takeoff = 0;
static int s_endurance_level = -1;

void endurance_init(Layer *window_layer, GRect bounds) {
  s_endurance_layer = layer_create(GRect(0, 110, bounds.size.w, 2));
  layer_set_update_proc(s_endurance_layer, endurance_update_proc);
  layer_add_child(window_layer, s_endurance_layer);
}

void endurance_destroy() {
  layer_destroy(s_endurance_layer);
}

void endurance_update_proc(Layer *layer, GContext *ctx) {
  
  if (s_endurance_level < 0) {
    return;
  }
  
  GRect bounds = layer_get_bounds(layer);
  
  int width = (int)(float)(((float)s_endurance_level /100.0F) * bounds.size.w);
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect((bounds.size.w - width), 0, width, bounds.size.h), 0, GCornerNone);
}

static void update_mission_display(time_t endurance_left) {
  mission_set_timestamp(ENDURANCE, endurance_left);
  format_duration_hhmm(endurance_left, mission_get_info_buffer(ENDURANCE), INFO_BUFFER_SIZE);
}

void endurance_set_takeoff_value(time_t duration) {
  s_endurance_at_takeoff = duration;
  if (duration == 0) {
    mission_set_status(ENDURANCE, false);
  } else {
    endurance_update();
    mission_set_status(ENDURANCE, true);
  }
}

time_t endurance_get_takeoff_value() {
  return s_endurance_at_takeoff;
}

void endurance_update() {
  if (s_endurance_at_takeoff == 0) {
    s_endurance_level = -1;
    layer_mark_dirty(s_endurance_layer);
    return;
  }
  
  time_t flight_time = mission_get_timestamp(FLIGHT_TIME);
  time_t endurance_left = s_endurance_at_takeoff - flight_time > 0 ? s_endurance_at_takeoff - flight_time : 0;
  if (endurance_left == 0) {
    s_endurance_level = 0;
  } else {
    s_endurance_level = endurance_left * 100 / s_endurance_at_takeoff;
  }
  
  if (s_endurance_at_takeoff != 0) {
    update_mission_display(endurance_left);
  }
  
  layer_mark_dirty(s_endurance_layer);
}