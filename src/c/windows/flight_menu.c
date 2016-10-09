#include <pebble.h>
#include "flight_menu.h"
#include "../components/mission.h"

static Window *s_main_window;
static MenuLayer *s_menu_layer;

static GBitmap *s_check_bitmap, *s_gas_bitmap, *s_exit_bitmap, *s_charlie_bitmap;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, 
                                      uint16_t section_index, void *context) {
  const uint16_t num_rows = 4;
  return num_rows;
}

static bool s_fpl_check;

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, 
                                        MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case 0:
      menu_cell_basic_draw(ctx, cell_layer, "Reminders", mission_checks_are_inhibited() ? "Enable" : "Inhibit", s_check_bitmap);
      break;
    case 1:
      menu_cell_basic_draw(ctx, cell_layer, "Exit", "Long-press back", s_exit_bitmap);
      break;
    case 2:
      menu_cell_basic_draw(ctx, cell_layer, "Endurance", "not implemented/at take-off", s_gas_bitmap);
      break;
    case 3:
      menu_cell_basic_draw(ctx, cell_layer, "Flight plan", s_fpl_check ? "Set alarm" : "Disable alarm", s_charlie_bitmap);
      break;
    default:
      break;
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, 
                                        MenuIndex *cell_index, void *context) {
  const int16_t cell_height = 40;
  return cell_height;
}

static void select_callback(struct MenuLayer *menu_layer, 
                                        MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case 0:
      mission_checks_inhibit(!mission_checks_are_inhibited());
      window_stack_pop(true);
      break;
    case 1:
      window_stack_pop_all(true);
      break;
    case 2:
      break;
    case 3:
      s_fpl_check = !s_fpl_check;
      menu_layer_reload_data(s_menu_layer);
      break;
    default:
      break;
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  s_check_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CONFIRM_SMALL);
  s_gas_bitmap = gbitmap_create_with_resource(RESOURCE_ID_GAS);
  s_exit_bitmap = gbitmap_create_with_resource(RESOURCE_ID_EXIT);
  s_charlie_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CHARLIE);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
#if defined(PBL_COLOR)
  menu_layer_set_normal_colors(s_menu_layer, GColorBlack, GColorWhite);
  menu_layer_set_highlight_colors(s_menu_layer, GColorRed, GColorWhite);
#endif
  
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = get_num_rows_callback,
    .draw_row = draw_row_callback,
    .get_cell_height = get_cell_height_callback,
    .select_click = select_callback,
  });
  
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  
  gbitmap_destroy(s_check_bitmap);
  gbitmap_destroy(s_gas_bitmap);
  gbitmap_destroy(s_exit_bitmap);
  gbitmap_destroy(s_charlie_bitmap);
  
  window_destroy(window);
  s_main_window = NULL;
}

void flight_menu_window_push() {
  if(!s_main_window) {
    s_main_window = window_create();
    window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorJaegerGreen, GColorWhite));
    window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_main_window, true);
}
