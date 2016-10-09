#pragma once
#include <pebble.h>
#define DIALOG_MESSAGE_WINDOW_MARGIN   10

typedef enum Alarm_type {
  ALARM_CRUISE_CHECK, ALARM_ENDURANCE, ALARM_FLIGHT_PLAN
} alarm_type ;

void alarm_start(alarm_type type);
void alarm_display(alarm_type type);
void alarm_stop(alarm_type type);
void alarm_inhibit(alarm_type type);
void alarm_enable(alarm_type type);
bool alarm_is_inhibited(alarm_type type);
bool alarm_is_active(alarm_type type);