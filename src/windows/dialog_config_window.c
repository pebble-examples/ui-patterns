/**
 * Example implementation of the 'config required' dialog UI pattern.
 */

#include "windows/dialog_config_window.h"

static Window *s_main_window;
static TextLayer *s_body_layer, *s_title_layer;
static BitmapLayer *s_icon_layer;

static GBitmap *s_icon_bitmap;

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CONFIG_REQUIRED);
  GRect bitmap_bounds = gbitmap_get_bounds(s_icon_bitmap);

  s_icon_layer = bitmap_layer_create(GRect(
    (bounds.size.w - bitmap_bounds.size.w) / 2, 
    ((bounds.size.h - bitmap_bounds.size.h) / 2) - 10, 
    bitmap_bounds.size.w, bitmap_bounds.size.h)
  );
  bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));

  s_title_layer = text_layer_create(GRect(0, 5, bounds.size.w, 60));
  text_layer_set_text(s_title_layer, DIALOG_CONFIG_WINDOW_APP_NAME);
  text_layer_set_text_color(s_title_layer, GColorWhite);
  text_layer_set_background_color(s_title_layer, GColorClear);
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));

  s_body_layer = text_layer_create(GRect(5, 120, bounds.size.w - 10, 60));
  text_layer_set_text(s_body_layer, DIALOG_CONFIG_WINDOW_MESSAGE);
  text_layer_set_text_color(s_body_layer, GColorWhite);
  text_layer_set_background_color(s_body_layer, GColorClear);
  text_layer_set_font(s_body_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_body_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_body_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_body_layer);

  bitmap_layer_destroy(s_icon_layer);
  gbitmap_destroy(s_icon_bitmap);

  window_destroy(window);
  s_main_window = NULL;
}

void dialog_config_window_push() {
  if(!s_main_window) {
    s_main_window = window_create();
    window_set_background_color(s_main_window, GColorDarkGray);
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });
  }
  window_stack_push(s_main_window, true);
}
