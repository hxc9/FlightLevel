// Pebble UI component adapted for modular use by Eric Phillips

#include <pebble.h>
#include "selection_layer.h"

// Look and feel
#define DEFAULT_CELL_PADDING 10
#define DEFAULT_SELECTED_INDEX 0
#define DEFAULT_FONT FONT_KEY_GOTHIC_28_BOLD
#define DEFAULT_ACTIVE_COLOR GColorWhite
#define DEFAULT_INACTIVE_COLOR PBL_IF_COLOR_ELSE(GColorDarkGray, GColorBlack)

#define BUTTON_HOLD_REPEAT_MS 100

static int prv_get_font_top_padding(GFont font) {
  if (font == fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD)) {
    return 10;
  } else if (font == fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)) {
    return 10;
  } else {
    return 0;
  }
}

static int prv_get_y_offset_which_vertically_centers_font(GFont font, int height) {
  int font_height = 0;
  int font_top_padding = prv_get_font_top_padding(font);
  if (font == fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD)) {
    font_height = 18;
  } else if (font == fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)) {
    font_height = 14;
  }

  return (height / 2) - (font_height / 2) - font_top_padding;
}

static void prv_draw_cell_backgrounds(Layer *layer, GContext *ctx) {
  SelectionLayerData *data = layer_get_data(layer);
  // Loop over each cell and draw the background rectangles
  for (int i = 0, current_x_offset = 0; i < data->num_cells; i++) {
    if (data->cell_widths[i] == 0) {
      continue;
    }

    int y_offset = 0;

    int height = layer_get_bounds(layer).size.h;

    const GRect rect = GRect(current_x_offset, y_offset, data->cell_widths[i], height);

    GColor bg_color = data->inactive_background_color;

    if (data->selected_cell_idx == i) {
      bg_color = data->active_background_color;
    }
    graphics_context_set_fill_color(ctx, bg_color);
    graphics_fill_rect(ctx, rect, 1, GCornerNone);

    current_x_offset += data->cell_widths[i] + data->cell_padding;
  }
}

static void prv_draw_text(Layer *layer, GContext *ctx) {
  SelectionLayerData *data = layer_get_data(layer);
  for (int i = 0, current_x_offset = 0; i < data->num_cells; i++) {
    if (data->callbacks.get_cell_text) {
      char *text = data->callbacks.get_cell_text(i, data->context);
      if (text) {
        int height = layer_get_bounds(layer).size.h;
        int y_offset = prv_get_y_offset_which_vertically_centers_font(data->font, height);

        GRect rect = GRect(current_x_offset, y_offset, data->cell_widths[i], height);
        graphics_draw_text(ctx, text, data->font, rect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
      }
    }

    current_x_offset += data->cell_widths[i] + data->cell_padding;
  }
}

static void prv_draw_selection_layer(Layer *layer, GContext *ctx) {
  prv_draw_cell_backgrounds(layer, ctx);
  prv_draw_text(layer, ctx);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! Click handlers

void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  Layer *layer = (Layer*)context;
  SelectionLayerData *data = layer_get_data(layer);

  if (data->is_active) {
      data->callbacks.increment(data->selected_cell_idx, click_number_of_clicks_counted(recognizer), data->context);
      layer_mark_dirty(layer);
  }
}

void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  Layer *layer = (Layer*)context;
  SelectionLayerData *data = layer_get_data(layer);

  if (data->is_active) {
    data->callbacks.decrement(data->selected_cell_idx, click_number_of_clicks_counted(recognizer), data->context);
    layer_mark_dirty(layer);
  }
}

void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  Layer *layer = (Layer*)context;
  SelectionLayerData *data = layer_get_data(layer);

  if (data->is_active) {
    if (data->selected_cell_idx >= data->num_cells - 1) {
      data->selected_cell_idx = 0;
      data->callbacks.complete(data->context);
    } else {
      data->selected_cell_idx++;
      layer_mark_dirty(layer);
    }
  }
}

void prv_back_click_handler(ClickRecognizerRef recognizer, void *context) {
  Layer *layer = (Layer*)context;
  SelectionLayerData *data = layer_get_data(layer);

  if (data->is_active) {
    if (data->selected_cell_idx == 0) {
      data->selected_cell_idx = 0;
      window_stack_pop(true);
    } else {
      data->selected_cell_idx--;
      layer_mark_dirty(layer);
    }
  }
}

static void prv_click_config_provider(Layer *layer) {
  window_set_click_context(BUTTON_ID_UP, layer);
  window_set_click_context(BUTTON_ID_DOWN, layer);
  window_set_click_context(BUTTON_ID_SELECT, layer);
  window_set_click_context(BUTTON_ID_BACK, layer);

  window_single_repeating_click_subscribe(BUTTON_ID_UP, BUTTON_HOLD_REPEAT_MS, prv_up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, BUTTON_HOLD_REPEAT_MS, prv_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, prv_back_click_handler);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! API

static Layer* selection_layer_init(SelectionLayerData *selection_layer_, GRect frame, int num_cells) {
  Layer *layer = layer_create_with_data(frame, sizeof(SelectionLayerData));
  SelectionLayerData *selection_layer_data = layer_get_data(layer);

  if (num_cells > MAX_SELECTION_LAYER_CELLS) {
    num_cells = MAX_SELECTION_LAYER_CELLS;
  }

  // Set layer defaults
  *selection_layer_data = (SelectionLayerData) {
    .active_background_color = DEFAULT_ACTIVE_COLOR,
    .inactive_background_color = DEFAULT_INACTIVE_COLOR,
    .num_cells = num_cells,
    .cell_padding = DEFAULT_CELL_PADDING,
    .selected_cell_idx = DEFAULT_SELECTED_INDEX,
    .font = fonts_get_system_font(DEFAULT_FONT),
    .is_active = true,
  };
  for (int i = 0; i < num_cells; i++) {
    selection_layer_data->cell_widths[i] = 0;
  }
  layer_set_frame(layer, frame);
  layer_set_clips(layer, false);
  layer_set_update_proc(layer, (LayerUpdateProc)prv_draw_selection_layer);

  return layer;
}

Layer* selection_layer_create(GRect frame, int num_cells) {
  SelectionLayerData *selection_layer_data = NULL;
  return selection_layer_init(selection_layer_data, frame, num_cells);
}

static void selection_layer_deinit(Layer* layer) {
  layer_destroy(layer);
}

void selection_layer_destroy(Layer* layer) {
  SelectionLayerData *data = layer_get_data(layer);

  animation_unschedule_all();
  if (data) {
    selection_layer_deinit(layer);
  }
}

void selection_layer_set_cell_width(Layer *layer, int idx, int width) {
  SelectionLayerData *data = layer_get_data(layer);

  if (data && idx < data->num_cells) {
    data->cell_widths[idx] = width;
  }
}

void selection_layer_set_font(Layer *layer, GFont font) {
  SelectionLayerData *data = layer_get_data(layer);

  if (data) {
    data->font = font;
  }
}

void selection_layer_set_inactive_bg_color(Layer *layer, GColor color) {
  SelectionLayerData *data = layer_get_data(layer);

  if (data) {
    data->inactive_background_color = color;
  }
}

void selection_layer_set_active_bg_color(Layer *layer, GColor color) {
  SelectionLayerData *data = layer_get_data(layer);

  if (data) {
    data->active_background_color = color;
  }
}

void selection_layer_set_cell_padding(Layer *layer, int padding) {
  SelectionLayerData *data = layer_get_data(layer);

  if (data) {
    data->cell_padding = padding;
  }
}

void selection_layer_set_active(Layer *layer, bool is_active) {
  SelectionLayerData *data = layer_get_data(layer);

  if (data) {
    if (is_active && !data->is_active) {
      data->selected_cell_idx = 0;
    } if (!is_active && data->is_active) {
      data->selected_cell_idx = MAX_SELECTION_LAYER_CELLS + 1;
    }

    data->is_active = is_active;
    layer_mark_dirty(layer);
  }
}

void selection_layer_set_click_config_onto_window(Layer *layer, struct Window *window) {
  if (layer && window) {
    window_set_click_config_provider_with_context(window, (ClickConfigProvider)prv_click_config_provider, layer);
  }
}

void selection_layer_set_callbacks(Layer *layer, void *context, SelectionLayerCallbacks callbacks) {
  SelectionLayerData *data = layer_get_data(layer);
  data->callbacks = callbacks;
  data->context = context;
}
