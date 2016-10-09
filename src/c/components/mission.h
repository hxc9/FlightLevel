#pragma once
#include <pebble.h>
#define INFO_COUNT 7
#define INFO_BUFFER_SIZE 6

typedef enum Info_category {
  FLIGHT_TIME, ENDURANCE, BLOCK_TIME, OFF_BLOCK, TAKE_OFF, LANDING, ON_BLOCK
} info_cat_t;

void mission_init(Layer *window_layer, GRect bounds);
void mission_destroy();

void mission_update(time_t tick);
void mission_next();
void mission_previous();
void mission_switch_display(bool to_flight_time);

time_t mission_get_timestamp(info_cat_t category);
void mission_set_timestamp(info_cat_t category, time_t timestamp);
char * mission_get_info_buffer(info_cat_t category);
void mission_set_status(info_cat_t category, bool active);