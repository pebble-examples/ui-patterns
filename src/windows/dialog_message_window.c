/**
 * Example implementation of the dialog message UI pattern.
 */

#include "windows/dialog_message_window.h"

static Window *s_main_window;
static TextLayer *s_label_layer;
static BitmapLayer *s_icon_layer;
static Layer *s_background_layer;

static Animation *s_appear_anim = NULL;

static GBitmap *s_icon_bitmap;

static void anim_stopped_handler(Animation *animation, bool finished, void *context) {
  s_appear_anim = NULL;
}

static void background_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorYellow);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, 0);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_background_layer = layer_create(GRect(0, 168, bounds.size.w, bounds.size.h));
  layer_set_update_proc(s_background_layer, background_update_proc);
  layer_add_child(window_layer, s_background_layer);

  s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WARNING);
  GRect bitmap_bounds = gbitmap_get_bounds(s_icon_bitmap);

  s_icon_layer = bitmap_layer_create(GRect(10, 168 + 10, bitmap_bounds.size.w, bitmap_bounds.size.h));
  bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));

  s_label_layer = text_layer_create(GRect(10, 168 + 10 + bitmap_bounds.size.h + 5, 124, 168 - (10 + bitmap_bounds.size.h + 10)));
  text_layer_set_text(s_label_layer, DIALOG_MESSAGE_WINDOW_MESSAGE);
  text_layer_set_background_color(s_label_layer, GColorClear);
  text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_label_layer));
}

static void window_unload(Window *window) {
  layer_destroy(s_background_layer);

  text_layer_destroy(s_label_layer);

  bitmap_layer_destroy(s_icon_layer);
  gbitmap_destroy(s_icon_bitmap);

  window_destroy(window);
  s_main_window = NULL;
}

static void window_appear(Window *window) {
  if(s_appear_anim) {
     // In progress, cancel
    animation_unschedule(s_appear_anim);
  }

  GRect bounds = layer_get_bounds(window_get_root_layer(window));
  GRect bitmap_bounds = gbitmap_get_bounds(s_icon_bitmap);

  Layer *label_layer = text_layer_get_layer(s_label_layer);
  Layer *icon_layer = bitmap_layer_get_layer(s_icon_layer);

  GRect start = layer_get_frame(s_background_layer);
  GRect finish = bounds;
  Animation *background_anim = (Animation*)property_animation_create_layer_frame(s_background_layer, &start, &finish);

  start = layer_get_frame(icon_layer);
  finish = GRect(10, 10, bitmap_bounds.size.w, bitmap_bounds.size.h);
  Animation *icon_anim = (Animation*)property_animation_create_layer_frame(icon_layer, &start, &finish);

  start = layer_get_frame(label_layer);
  finish = GRect(10, 10 + bitmap_bounds.size.h + 5, 124, 168 - (10 + bitmap_bounds.size.h + 10));
  Animation *label_anim = (Animation*)property_animation_create_layer_frame(label_layer, &start, &finish);

  s_appear_anim = animation_spawn_create(background_anim, icon_anim, label_anim, NULL);
  animation_set_handlers(s_appear_anim, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
  animation_set_delay(s_appear_anim, 700);
  animation_schedule(s_appear_anim);
}

void dialog_message_window_push() {
  if(!s_main_window) {
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
        .appear = window_appear
    });
  }
  window_stack_push(s_main_window, true);
}
