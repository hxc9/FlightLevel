#pragma once
#include <pebble.h>

void battery_init(Layer *window_layer, GRect bounds);
void battery_destroy();
void battery_update_proc(Layer *layer, GContext *ctx);
void battery_callback(BatteryChargeState state);