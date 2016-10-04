#pragma once
#include <pebble.h>

void clock_init(Layer *window_layer, GRect bounds);
void clock_update(time_t tick);
void clock_destroy();