#pragma once
#include <pebble.h>

void endurance_init(Layer *window_layer, GRect bounds);
void endurance_destroy();
void endurance_update_proc(Layer *layer, GContext *ctx);

void endurance_set_takeoff_value(time_t duration);
time_t endurance_get_takeoff_value();
void endurance_update();