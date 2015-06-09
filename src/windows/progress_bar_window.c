/**
 * Example progress bar implementation.
 */

#include "progress_bar_window.h"

static Window *s_window;
static Layer *s_progress_bar;
static StatusBarLayer *s_status_bar;

static AppTimer *s_timer;
static int s_progress = 0;  // 0 - 100

static void progress_callback(void *context);

static void progress_bar_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  int width = (int)(float)(((float)s_progress / 100.0F) * bounds.size.w);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_line(ctx, GPointZero, GPoint(width, 0));
}

static void next_timer() {
  s_timer = app_timer_register(PROGRESS_BAR_WINDOW_DELTA, progress_callback, NULL);
}

static void progress_callback(void *context) {
  s_progress += (s_progress < 100) ? 1 : -100;
  layer_mark_dirty(s_progress_bar);
  next_timer();
}

static void window_appear(Window *window) {
  s_progress = 0;
  next_timer();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_status_bar = status_bar_layer_create();
  status_bar_layer_set_separator_mode(s_status_bar, StatusBarLayerSeparatorModeDotted);
  status_bar_layer_set_colors(s_status_bar, GColorClear, GColorWhite);
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));

  s_progress_bar = layer_create((GRect){ 
    .origin = GPoint(0, STATUS_BAR_LAYER_HEIGHT - 2), 
    .size = PROGRESS_BAR_WINDOW_SIZE 
  });
  layer_set_update_proc(s_progress_bar, progress_bar_proc);
  layer_add_child(window_layer, s_progress_bar);
}

static void window_unload(Window *window) {
  layer_destroy(s_progress_bar);
  status_bar_layer_destroy(s_status_bar);
  window_destroy(s_window);
  s_window = NULL;
}

static void window_disappear(Window *window) {
  if(s_timer) {
    app_timer_cancel(s_timer);
    s_timer = NULL;
  }
}

void progress_bar_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, GColorDarkCandyAppleRed);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .appear = window_appear,
      .load = window_load,
      .unload = window_unload,
      .disappear = window_disappear
    });
  }
  window_stack_push(s_window, true);
}
