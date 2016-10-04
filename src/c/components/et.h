#pragma once
#include <pebble.h>

void et_init(Layer *window_layer, GRect bounds);
void et_destroy();
void elapsed_time_flyback();
void elapsed_time_update(time_t tick);