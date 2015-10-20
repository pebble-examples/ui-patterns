#include <pebble.h>

#include "windows/checkbox_window.h"
#include "windows/dialog_choice_window.h"
#include "windows/dialog_message_window.h"
#include "windows/list_message_window.h"
#include "windows/radio_button_window.h"
#include "windows/pin_window.h"
#include "windows/text_animation_window.h"
#include "windows/progress_bar_window.h"
#include "windows/progress_layer_window.h"
#include "windows/dialog_config_window.h"

#define NUM_WINDOWS 10

static Window *s_main_window;
static MenuLayer *s_menu_layer;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return NUM_WINDOWS;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case 0:
      menu_cell_basic_draw(ctx, cell_layer, "Checkbox List", NULL, NULL);
      break;
    case 1:
      menu_cell_basic_draw(ctx, cell_layer, "Choice Dialog", NULL, NULL);
      break;
    case 2:
      menu_cell_basic_draw(ctx, cell_layer, "Message Dialog", NULL, NULL);
      break;
    case 3:
      menu_cell_basic_draw(ctx, cell_layer, "List Message", NULL, NULL);
      break;
    case 4:
      menu_cell_basic_draw(ctx, cell_layer, "Radio Button", NULL, NULL);
      break;
    case 5: 
      menu_cell_basic_draw(ctx, cell_layer, "PIN Entry", NULL, NULL);
      break;
    case 6:
      menu_cell_basic_draw(ctx, cell_layer, "Text Animation", NULL, NULL);
      break;
    case 7:
      menu_cell_basic_draw(ctx, cell_layer, "Progress Bar", NULL, NULL);
      break;
    case 8:
      menu_cell_basic_draw(ctx, cell_layer, "Progress Layer", NULL, NULL);
      break;
    case 9:
      menu_cell_basic_draw(ctx, cell_layer, "App Config Prompt", NULL, NULL);
      break;
    default:
      break;
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return PBL_IF_ROUND_ELSE(
    menu_layer_is_index_selected(menu_layer, cell_index) ? 
      MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT,
    44);
}

static void pin_complete_callback(PIN pin, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Pin was %d %d %d", pin.digits[0], pin.digits[1], pin.digits[2]);
  pin_window_pop((PinWindow*)context, true);
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case 0:
      checkbox_window_push();
      break;
    case 1:
      dialog_choice_window_push();
      break;
    case 2:
      dialog_message_window_push();
      break;
    case 3:
      list_message_window_push();
      break;
    case 4:
      radio_button_window_push();
      break;
    case 5: {
        PinWindow *pin_window = pin_window_create((PinWindowCallbacks) {
          .pin_complete = pin_complete_callback
        });
        pin_window_push(pin_window, true);
      }
      break;
    case 6:
      text_animation_window_push();
      break;
    case 7:
      progress_bar_window_push();
      break;
    case 8:
      progress_layer_window_push();
      break;
    case 9:
      dialog_config_window_push();
      break;
    default:
      break;
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_normal_colors(s_menu_layer, GColorBlack, GColorWhite);
  menu_layer_set_highlight_colors(s_menu_layer, GColorRed, GColorWhite);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
      .get_num_rows = get_num_rows_callback,
      .draw_row = draw_row_callback,
      .get_cell_height = PBL_IF_ROUND_ELSE(get_cell_height_callback, NULL),
      .select_click = select_callback,
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
