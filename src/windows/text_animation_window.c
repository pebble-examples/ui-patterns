/**
 * Example text change animation implementation.
 */

#include "text_animation_window.h"

static Window *s_window;
static TextLayer *s_text_layer;

static AppTimer *s_timer;
static char s_text[2][32];
static uint8_t s_current_text;

static void animate();

static void out_stopped_handler(Animation *animation, bool finished, void *context) {
  s_current_text += (s_current_text == 0) ? 1 : -1;
  text_layer_set_text(s_text_layer, s_text[s_current_text]);

  Layer *text_layer = text_layer_get_layer(s_text_layer);
  GRect frame = layer_get_frame(text_layer);
  GRect start = GRect(frame.origin.x + (2 * TEXT_ANIMATION_WINDOW_DISTANCE), frame.origin.y, frame.size.w, frame.size.h);
  GRect finish = GRect(frame.origin.x + TEXT_ANIMATION_WINDOW_DISTANCE, frame.origin.y, frame.size.w, frame.size.h);

  PropertyAnimation *in_prop_anim = property_animation_create_layer_frame(text_layer, &start, &finish);
  Animation *in_anim = property_animation_get_animation(in_prop_anim);
  animation_set_curve(in_anim, AnimationCurveEaseInOut);
  animation_set_duration(in_anim, TEXT_ANIMATION_WINDOW_DURATION);
  animation_schedule(in_anim);
}

static void shake_animation() {
  Layer *text_layer = text_layer_get_layer(s_text_layer);
  GRect start = layer_get_frame(text_layer);
  GRect finish = GRect(start.origin.x - TEXT_ANIMATION_WINDOW_DISTANCE, start.origin.y, start.size.w, start.size.h);

  PropertyAnimation *out_prop_anim = property_animation_create_layer_frame(text_layer, &start, &finish);
  Animation *out_anim = property_animation_get_animation(out_prop_anim);
  animation_set_curve(out_anim, AnimationCurveEaseInOut);
  animation_set_duration(out_anim, TEXT_ANIMATION_WINDOW_DURATION);
  animation_set_handlers(out_anim, (AnimationHandlers) {
    .stopped = out_stopped_handler
  }, NULL);
  animation_schedule(out_anim);
}

static void animate_callback(void *context) {
  animate();
}

static void animate() {
  shake_animation();
  s_timer = app_timer_register(TEXT_ANIMATION_WINDOW_INTERVAL, animate_callback, NULL);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, (bounds.size.h / 2) - 24, bounds.size.w, bounds.size.h));
  text_layer_set_text(s_text_layer, "Example text.");
  text_layer_set_text_color(s_text_layer, GColorWhite);
  text_layer_set_background_color(s_text_layer, GColorClear);
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  window_destroy(s_window);
  s_window = NULL;
}

static void window_disappear(Window *window) {
  if(s_timer) {
    app_timer_cancel(s_timer);
    s_timer = NULL;
  }
}

void text_animation_window_push() {
  snprintf(s_text[0], sizeof(s_text[0]), "Some example text");
  snprintf(s_text[1], sizeof(s_text[1]), "Some more example text");
  s_current_text = 0;

  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, GColorBlueMoon);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
      .disappear = window_disappear
    });
  }
  window_stack_push(s_window, true);

  animate();
}
