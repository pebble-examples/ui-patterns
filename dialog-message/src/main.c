/**
 * Example implementation of the dialog message UI pattern.
 */

#include <pebble.h>

#define MESSAGE  "Battery is low! Connect the charger."

static Window *s_main_window;
static TextLayer *s_label_layer;
static BitmapLayer *s_icon_layer;

static GBitmap *s_icon_bitmap;

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WARNING);
  GRect bitmap_bounds = gbitmap_get_bounds(s_icon_bitmap);

  s_icon_layer = bitmap_layer_create(GRect(10, 10, bitmap_bounds.size.w, bitmap_bounds.size.h));
  bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
#ifdef PBL_PLATFORM_BASALT
  bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
#endif
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));

  s_label_layer = text_layer_create(GRect(10, 10 + bitmap_bounds.size.h + 5, 124, 168 - (10 + bitmap_bounds.size.h + 10)));
  text_layer_set_text(s_label_layer, MESSAGE);
  text_layer_set_background_color(s_label_layer, GColorClear);
  text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_label_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_label_layer);

  bitmap_layer_destroy(s_icon_layer);
  gbitmap_destroy(s_icon_bitmap);
}

static void init() {
  s_main_window = window_create();
#ifdef PBL_PLATFORM_BASALT
  window_set_background_color(s_main_window, GColorYellow);
#endif
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
