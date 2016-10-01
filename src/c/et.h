#pragma once
#include <pebble.h>

void et_init(Layer *window_layer, GRect bounds);
void et_destroy();
void start_elapsed_time();
void update_elapsed_time(time_t tick);