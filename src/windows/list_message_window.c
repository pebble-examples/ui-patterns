/**
 * Example implementation of the list menu UI pattern.
 */

#include "list_message_window.h"

static Window *s_main_window;
static MenuLayer *s_menu_layer;
static TextLayer *s_list_message_layer;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return LIST_MESSAGE_WINDOW_NUM_ROWS;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  static char s_buff[16];
  snprintf(s_buff, sizeof(s_buff), "Item %d", (int)cell_index->row);
  menu_cell_basic_draw(ctx, cell_layer, s_buff, NULL, NULL);
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  return LIST_MESSAGE_WINDOW_CELL_HEIGHT;
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(GRect(0, 0, 144, LIST_MESSAGE_WINDOW_MENU_HEIGHT));
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
      .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback)get_num_rows_callback,
      .draw_row = (MenuLayerDrawRowCallback)draw_row_callback,
      .get_cell_height = (MenuLayerGetCellHeightCallback)get_cell_height_callback,
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

  s_list_message_layer = text_layer_create(GRect(0, LIST_MESSAGE_WINDOW_MENU_HEIGHT, 144, 168 - LIST_MESSAGE_WINDOW_MENU_HEIGHT));
  text_layer_set_text_alignment(s_list_message_layer, GTextAlignmentCenter);
  text_layer_set_text(s_list_message_layer, LIST_MESSAGE_WINDOW_HINT_TEXT);
  layer_add_child(window_layer, text_layer_get_layer(s_list_message_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  text_layer_destroy(s_list_message_layer);

  window_destroy(window);
  s_main_window = NULL;
}

void list_message_window_push() {
  if(!s_main_window) {
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
  }
  window_stack_push(s_main_window, true);
}
