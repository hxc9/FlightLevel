#include <pebble.h>
#include "utils.h"

TextLayer *configure_text_layer(Layer *window_layer, GRect box, GFont font, GTextAlignment alignment) {
  TextLayer *layer = text_layer_create(box);
  
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_text_color(layer, GColorWhite);
  text_layer_set_font(layer, font);
  text_layer_set_text_alignment(layer, alignment);
  
  layer_add_child(window_layer, text_layer_get_layer(layer));
  
  return layer;
}

void format_duration_hhmm(time_t time_in_s, char *buffer, int size) {
  int minutes = (int)time_in_s / 60 % 60;
  int hours = (int)time_in_s / 3600 % 1000;
  
  if (hours > 99) {
    hours = 99;
    minutes = 99;
  }
  
  snprintf(buffer, size, "%d:%02d", hours, minutes);
}

void format_time_hhmm(time_t time_in_s, char *buffer, int size) {
  struct tm *tick_time_z = gmtime(&time_in_s);
  strftime(buffer, size, "%H:%M", tick_time_z);
}