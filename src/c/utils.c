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