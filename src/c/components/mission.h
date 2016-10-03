#pragma once
#include <pebble.h>

void mission_init(Layer *window_layer, GRect bounds);
void mission_destroy();

void mission_update(time_t tick);
void mission_next();
void mission_previous();
void mission_switch_display(bool to_flight_time);

bool mission_checks_are_inhibited();
void mission_checks_inhibit(bool inhibit);