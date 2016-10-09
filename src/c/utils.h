#pragma once
#include <pebble.h>

TextLayer *configure_text_layer(Layer *window_layer, GRect box, GFont font, GTextAlignment alignment);
void format_duration_hhmm(time_t time_in_s, char buffer[], int size);
void format_time_hhmm(time_t time_in_s, char buffer[], int size);