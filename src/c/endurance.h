#pragma once
#include <pebble.h>

void endurance_init(Layer *window_layer, GRect bounds);
void endurance_destroy();
void endurance_update_proc(Layer *layer, GContext *ctx);