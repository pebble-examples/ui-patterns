/**
 * Example implementation of the 'value selection field' UI pattern
 */

#include "pin_window.h"

static Window *s_main_window;
static TextLayer *s_hint_layer;
static TextLayer *s_number_layers[4];

static PIN pin;

static char s_value_buffers[4][2];
static int s_selection;

static void update_ui() {
  for(int i = 0; i < 4; i++) {
    text_layer_set_background_color(s_number_layers[i], (i == s_selection) ? GColorJaegerGreen : GColorDarkGray);
    snprintf(s_value_buffers[i], sizeof("0"), "%d", pin.digits[i]);
    text_layer_set_text(s_number_layers[i], s_value_buffers[i]);
  }
}

static void init() {
  // Reset to initial selection
  s_selection = 0;
  for(int i = 0; i < 4; i++) {
    pin.digits[i] = 0;
  }

  update_ui();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Next column
  s_selection += 1;

  if(s_selection == 4) {
    // Confirm. Do something with pin, or pass it elsewhere
    APP_LOG(APP_LOG_LEVEL_INFO, "User pin is %d%d%d%d", 
      pin.digits[0], pin.digits[1], pin.digits[2], pin.digits[3]);

    window_stack_pop(true);
  }

  update_ui();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  pin.digits[s_selection] += (pin.digits[s_selection] == 9) ? -9 : 1;

  update_ui();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  pin.digits[s_selection] -= (pin.digits[s_selection] == 0) ? -9 : 1;

  update_ui();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_hint_layer = text_layer_create(GRect(0, 20, 144, 40));
  text_layer_set_text(s_hint_layer, "Enter PIN");
  text_layer_set_text_alignment(s_hint_layer, GTextAlignmentCenter);
  text_layer_set_font(s_hint_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_hint_layer));

  for(int i = 0; i < 4; i++) {
    s_number_layers[i] = text_layer_create(GRect(i * (PIN_WINDOW_SPACING + PIN_WINDOW_SPACING), 60, 30, 40));
    text_layer_set_text_color(s_number_layers[i], GColorWhite);
    text_layer_set_background_color(s_number_layers[i], GColorDarkGray);
    text_layer_set_text(s_number_layers[i], "0");
    text_layer_set_font(s_number_layers[i], fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS));
    text_layer_set_text_alignment(s_number_layers[i], GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_number_layers[i]));
  }
}

static void window_unload(Window *window) {
  for(int i = 0; i < 4; i++) {
    text_layer_destroy(s_number_layers[i]);
  }
  text_layer_destroy(s_hint_layer);

  window_destroy(window);
  s_main_window = NULL;
}

void pin_window_push() {
  if(!s_main_window) {
    s_main_window = window_create();
    window_set_click_config_provider(s_main_window, click_config_provider);
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
  }
  window_stack_push(s_main_window, true);

  init();
}