#pragma once

void mission_init(Layer *window_layer, GRect bounds);
void mission_destroy();

void mission_update(time_t tick);
void mission_next();
void mission_previous();