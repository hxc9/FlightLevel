#pragma once

#include <pebble.h>

#define TIME_WINDOW_NUM_CELLS 4
#define TIME_WINDOW_SIZE GSize(128, 34)

typedef struct {
  int digits[TIME_WINDOW_NUM_CELLS];
  int max_values[TIME_WINDOW_NUM_CELLS];
} TIME;

typedef void (*TimeWindowComplete)(time_t duration, void *context);

typedef struct TimeWindowDefinition {
  const char * main_text;
  const char * sub_text;
  TimeWindowComplete time_complete;
} TimeWindowDefinition;

typedef struct {
  Window *window;
  TextLayer *main_text, *sub_text;
  Layer *selection;
  GColor highlight_color;
  StatusBarLayer *status;
  TimeWindowDefinition definition;

  TIME time;
  char field_buffs[TIME_WINDOW_NUM_CELLS][2];
  int8_t field_selection;
} TimeWindow;

/*
 * Creates a new TimeWindow in memory but does not push it into view
 *  time_window_callbacks: callbacks for communication
 *  returns: a pointer to a new TimeWindow structure
 */
TimeWindow* time_window_create(TimeWindowDefinition time_window_definition);

/*
 * Destroys an existing TimeWindow
 *  time_window: a pointer to the TimeWindow being destroyed
 */
void time_window_destroy(TimeWindow *time_window);

/*
 * Push the window onto the stack
 *  time_window: a pointer to the TimeWindow being pushed
 *  animated: whether to animate the push or not
 */
void time_window_push(TimeWindow *time_window, bool animated, time_t initial_value);

/*
 * Pop the window off the stack
 *  time_window: a pointer to the TimeWindow to pop
 *  animated: whether to animate the pop or not
 */
void time_window_pop(TimeWindow *time_window, bool animated);

/*
 * Gets whether it is the topmost window or not
 *  time_window: a pointer to the TimeWindow being checked
 *  returns: a boolean indicating if it is the topmost window
 */
bool time_window_get_topmost_window(TimeWindow *time_window);

/*
 * Sets the over-all color scheme of the window
 *  color: the GColor to set the highlight to
 */
void time_window_set_highlight_color(TimeWindow *time_window, GColor color);
