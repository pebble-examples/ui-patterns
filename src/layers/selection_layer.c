// Pebble UI component adapted for modular use by Eric Phillips

#include <pebble.h>
#include "selection_layer.h"

// Look and feel
#define DEFAULT_CELL_PADDING 10
#define DEFAULT_SELECTED_INDEX 0
#define DEFAULT_FONT FONT_KEY_GOTHIC_28_BOLD
#ifdef PBL_COLOR
#define DEFAULT_ACTIVE_COLOR GColorWhite
#define DEFAULT_INACTIVE_COLOR GColorDarkGray
#endif

#define BUTTON_HOLD_REPEAT_MS 100

// 3 frames in the spec
#define BUMP_TEXT_DURATION_MS 107
// 6 frames in the spec
#define BUMP_SETTLE_DURATION_MS 214

// In the spec this is 3
#define SETTLE_HEIGHT_DIFF 6

// 3 frames in the spec
#define SLIDE_DURATION_MS 107
// 5 frames in the spec
#define SLIDE_SETTLE_DURATION_MS 179


///////////////////////////////////////////////////////////////////////////////////////////////////
//! function declarations
static Animation* prv_create_bump_settle_animation(Layer *layer);
static Animation* prv_create_slide_settle_animation(Layer *layer);


///////////////////////////////////////////////////////////////////////////////////////////////////
//! Drawing helpers

static int prv_get_pixels_for_bump_settle(int anim_percent_complete) {
  if (anim_percent_complete) {
    return SETTLE_HEIGHT_DIFF - ((SETTLE_HEIGHT_DIFF * anim_percent_complete) / 100);
  } else {
    return 0;
  }
}

static int prv_get_font_top_padding(GFont font) {
  if (font == fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD)) {
    return 10;
  } else if (font == fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)) {
    return 10;
  } else {
    return 0;
  }
}
// Assumes numbers / capital letters
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

///////////////////////////////////////////////////////////////////////////////////////////////////
//! Drawing the layer

static void prv_draw_cell_backgrounds(Layer *layer, GContext *ctx) {
  SelectionLayerData *data = layer_get_data(layer);
  // Loop over each cell and draw the background rectangles
  for (int i = 0, current_x_offset = 0; i < data->num_cells; i++) {
    if (data->cell_widths[i] == 0) {
      continue;
    }

    // The y-offset for each cell defaults to 0 (the box is drawn from the top of the frame).
    // If we are currently doing the increment animation, the y-offset will be above the frame
    // (negative).
    int y_offset = 0;
    if (data->selected_cell_idx == i && data->bump_is_upwards) {
      y_offset = -prv_get_pixels_for_bump_settle(data->bump_settle_anim_progress);
    }

    // The height of each cell default to the height of the frame.
    int height = layer_get_bounds(layer).size.h;
    if (data->selected_cell_idx == i) {
      // If we are currently doing the increment animation then the height must be increased so
      // that the bottom of the cell stays fixed as the top jumps up.
      // If we are currently doing the decrement animation then the height must be increased so
      // that we draw below the cell
      height += prv_get_pixels_for_bump_settle(data->bump_settle_anim_progress);
    }

    // No animations currently change the width of the cell. The slide animation is drawn later
    // over top of this.
    int width = data->cell_widths[i];

    // The cell rectangle has been constructed
    const GRect rect = GRect(current_x_offset, y_offset, width, height);

#ifdef PBL_SDK_3
    // Draw the cell as inactive by default
    GColor bg_color = data->inactive_background_color;

    // If the slide animation is in progress, then don't set the background color. The slide
    // will be drawn over top of this later
    if (data->selected_cell_idx == i && !data->slide_amin_progress) {
      bg_color = data->active_background_color;
    }
    graphics_context_set_fill_color(ctx, bg_color);
    graphics_fill_rect(ctx, rect, 1, GCornerNone);
#else
    // draw a black, empty box
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_rect(ctx, rect);
    // position highlight
    if (data->selected_cell_idx == i && !data->slide_amin_progress){
      layer_set_frame(inverter_layer_get_layer(data->inverter), rect);
    }
#endif

    // Update the x-offset so we are ready for the next cell
    current_x_offset += data->cell_widths[i] + data->cell_padding;
  }
}

static void prv_draw_slider_slide(Layer *layer, GContext *ctx) {
  SelectionLayerData *data = layer_get_data(layer);
  // Find the active cell's x-offset
  int starting_x_offset = 0;
  for (int i = 0; i < data->num_cells; i++) {
    if (data->selected_cell_idx == i) {
      break;
    }
    starting_x_offset += data->cell_widths[i] + data->cell_padding;
  }

  // The slider moves horizontally (to the right only) from 1 cell to another.
  // In total we need to slide from our current x offset, to the x offset of the next cell
  int next_cell_width = data->cell_widths[data->selected_cell_idx + 1];
  if (!data->slide_is_forward)
    next_cell_width = data->cell_widths[data->selected_cell_idx - 1];
  int slide_distance = next_cell_width + data->cell_padding;

  // The current distance we have moved depends on how far we are through the animation
  int current_slide_distance = (slide_distance * data->slide_amin_progress) / 100;
  if (!data->slide_is_forward)
    current_slide_distance = -current_slide_distance;

  // Finally our current x-offset is our starting offset plus our current distance
  int current_x_offset = starting_x_offset + current_slide_distance;


  // As the cell slides the width of the cell also changes...
  // It starts as the width of the current active cell.
  int cur_cell_width = data->cell_widths[data->selected_cell_idx];

  // It then morphs to the size of the next cell + padding causing the illusion that the selector
  // overshoot it's mark (it will settle back to the correct size in a different animation).
  // This means the the width change is the width difference between the two cells plus padding
  int total_cell_width_change = next_cell_width - cur_cell_width + data->cell_padding;

  // The current width change depends on how far we are through the animation
  int current_cell_width_change =
      (total_cell_width_change * (int) data->slide_amin_progress) / 100;

  // And finally our current width is the starting width plus any change
  int current_cell_width = cur_cell_width + current_cell_width_change;
  if (!data->slide_is_forward)
    current_x_offset -= current_cell_width_change;


  GRect rect = GRect(current_x_offset, 0, current_cell_width, layer_get_bounds(layer).size.h);

#ifdef PBL_COLOR
  graphics_context_set_fill_color(ctx, data->active_background_color);
  graphics_fill_rect(ctx, rect, 1, GCornerNone);
#else
  layer_set_frame(inverter_layer_get_layer(data->inverter), rect);
#endif
}

static void prv_draw_slider_settle(Layer *layer, GContext *ctx) {
  SelectionLayerData *data = layer_get_data(layer);
  // Find the active cell's x-offset.
  int starting_x_offset = 0;
  for (int i = 0; i < data->num_cells; i++) {
    if (data->selected_cell_idx == i) {
      break;
    }
    starting_x_offset += data->cell_widths[i] + data->cell_padding;
  }

  // After the slider is done sliding then active cell is updated and filled in with the
  // correct background color.
  // This animation is responsible for the settle effect. It removes the extra width
  // (padding width) which was drawn to create an overshoot effect.

  // We only need to draw the receding padding (prv_draw_cell_backgrounds should do the cell).
  // Increment out offset so we are and the right edge of the active cell.
  int x_offset = starting_x_offset;
  if (data->slide_is_forward)
    x_offset += data->cell_widths[data->selected_cell_idx];

  // How much of the extra padding to draw depends on the animation's progress
  int current_width =
      (data->cell_padding * data->slide_settle_anim_progress) / 100;
  if (!data->slide_is_forward)
    x_offset -= current_width;

  GRect rect = GRect(x_offset, 0, current_width, layer_get_bounds(layer).size.h);

#ifdef PBL_COLOR
  graphics_context_set_fill_color(ctx, data->active_background_color);
  graphics_fill_rect(ctx, rect, 1, GCornerNone);
#else
  if (data->slide_is_forward){
    rect.origin.x -= data->cell_widths[data->selected_cell_idx];
    rect.size.w += data->cell_widths[data->selected_cell_idx];
  }
  else
    rect.size.w += data->cell_widths[data->selected_cell_idx];
  layer_set_frame(inverter_layer_get_layer(data->inverter), rect);
#endif
}

static void prv_draw_text(Layer *layer, GContext *ctx) {
  // set text color
#ifndef PBL_COLOR
  graphics_context_set_text_color(ctx, GColorBlack);
#endif

  SelectionLayerData *data = layer_get_data(layer);
  // Loop over each cell and draw the text
  for (int i = 0, current_x_offset = 0; i < data->num_cells; i++) {
    if (data->callbacks.get_cell_text) {
      // Potential optimization: cache the cell text somewhere as this function gets called
      // a lot (because of animations). The current users of this modules just call snprintf()
      // in the get_cell_text() function, so it isn't a big deal for now
      char *text = data->callbacks.get_cell_text(i, data->callback_context);
      if (text) {
        // We need to figure out the height of the box that the text is being drawn in so that
        // it can be vertically centered
        int height = layer_get_bounds(layer).size.h;
        if (data->selected_cell_idx == i) {
          // See prv_draw_cell_backgrounds() for reasoning
          height += prv_get_pixels_for_bump_settle(data->bump_settle_anim_progress);
        }
        // The text should be be vertically centered, unless we are performing an increment /
        // decrment animation.
        int y_offset =
            prv_get_y_offset_which_vertically_centers_font(data->font, height);

        // We might not be drawing from the top of the frame, compensate if needed
        if (data->selected_cell_idx == i && data->bump_is_upwards) {
          y_offset -= prv_get_pixels_for_bump_settle(data->bump_settle_anim_progress);
        }
        // If we are performing an increment or decrement animation then update the
        // y offset with our progress
        if (data->selected_cell_idx == i) {
          int delta = (data->bump_text_anim_progress *
              prv_get_font_top_padding(data->font)) / 100;
          if (data->bump_is_upwards) {
            delta *= -1;
          }
          y_offset += delta;
        }

        GRect rect = GRect(current_x_offset, y_offset, data->cell_widths[i], height);
        graphics_draw_text(ctx, text, data->font,
            rect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
      }
    }
    // Update the x-offset so we are ready for the next cell
    current_x_offset += data->cell_widths[i] + data->cell_padding;
  }
}

static void prv_draw_selection_layer(Layer *layer, GContext *ctx) {
  SelectionLayerData *data = layer_get_data(layer);
  // The first thing that is drawn is the background for each cell
  prv_draw_cell_backgrounds(layer, ctx);

#ifndef PBL_COLOR // aplite
  // draw text under inverter layer
  prv_draw_text(layer, ctx);
#endif

  // If the slider is in motion draw it. This is above the backgrounds, but below the text
  if (data->slide_amin_progress) {
    prv_draw_slider_slide(layer, ctx);
  }
  if (data->slide_settle_anim_progress) {
    prv_draw_slider_settle(layer, ctx);
  }

#ifdef PBL_COLOR // basalt
  // Finally the text is drawn over everything
  prv_draw_text(layer, ctx);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! Increment / Decrement Animation

//! This animation causes a the active cell to "bump" when the user presses the up button.
//! This animation has two parts:
//! 1) The "text to cell edge"
//! 2) The "background settle"

//! The "text to cell edge" (bump_text) moves the text until it hits the top / bottom of the cell.

//! The "background settle" (bump_settle) is a reaction to the "text to cell edge" animation.
//! The top of the cell immediately expands down giving the impression that the text "pushed" the
//! cell making it bigger. The cell then shrinks / settles back to its original height
//! with the text vertically centered

static void prv_bump_text_impl(struct Animation *animation,
                               const AnimationProgress distance_normalized) {
  Layer *layer = (Layer*) animation_get_context(animation);
  SelectionLayerData *data = layer_get_data(layer);

  // Update the completion percent of the animation
  data->bump_text_anim_progress = (100 * distance_normalized) / ANIMATION_NORMALIZED_MAX;
  layer_mark_dirty(layer);
}

static void prv_bump_text_stopped(Animation *animation, bool finished, void *context) {
  Layer *layer = (Layer*)animation_get_context(animation);
  SelectionLayerData *data = layer_get_data(layer);
  data->bump_text_anim_progress = 0;

  // The text value is updated halfway through the animation
  if (data->bump_is_upwards == true) {
    data->callbacks.increment(data->selected_cell_idx, 1,
        data->callback_context);
  } else {
    data->callbacks.decrement(data->selected_cell_idx, 1,
        data->callback_context);
  }
  // destroy
  animation_destroy(animation);

  // schedule next animation
#ifndef PBL_SDK_3
  Animation *bump_settle = prv_create_bump_settle_animation(layer);
  animation_schedule(bump_settle);
#endif
}

static void prv_bump_settle_impl(struct Animation *animation,
                                 const AnimationProgress distance_normalized) {
  Layer *layer = (Layer*)animation_get_context(animation);
  SelectionLayerData *data = layer_get_data(layer);

  // Update the completion percent of the animation
  data->bump_settle_anim_progress =
      (100 * distance_normalized) / ANIMATION_NORMALIZED_MAX;
  layer_mark_dirty(layer);
}

static void prv_bump_settle_stopped(Animation *animation, bool finished, void *context) {
  Layer *layer = (Layer*)animation_get_context(animation);
  SelectionLayerData *data = layer_get_data(layer);
  data->bump_settle_anim_progress = 0;
  // destroy
  animation_destroy(animation);
}

static Animation* prv_create_bump_text_animation(Layer *layer) {
  SelectionLayerData *data = layer_get_data(layer);
  PropertyAnimation *bump_text_anim =
      property_animation_create_layer_frame(layer, NULL, NULL);
  Animation *animation = property_animation_get_animation(bump_text_anim);
  animation_set_curve(animation, AnimationCurveEaseIn);
  animation_set_duration(animation, BUMP_TEXT_DURATION_MS);
  AnimationHandlers anim_handler = {
      .stopped = prv_bump_text_stopped,
  };
  animation_set_handlers(animation, anim_handler, layer);

  data->bump_text_impl = (AnimationImplementation) {
    .update = prv_bump_text_impl,
  };
  animation_set_implementation(animation, &data->bump_text_impl);

  return animation;
}

static Animation* prv_create_bump_settle_animation(Layer *layer) {
  SelectionLayerData *data = layer_get_data(layer);
  PropertyAnimation *bump_settle_anim =
      property_animation_create_layer_frame(layer, NULL, NULL);
  Animation *animation = property_animation_get_animation(bump_settle_anim);
  animation_set_curve(animation, AnimationCurveEaseOut);
  animation_set_duration(animation, BUMP_SETTLE_DURATION_MS);
  AnimationHandlers anim_handler = {
      .stopped = prv_bump_settle_stopped,
  };
  animation_set_handlers(animation, anim_handler, layer);

  data->bump_settle_anim_impl = (AnimationImplementation) {
    .update = prv_bump_settle_impl,
  };
  animation_set_implementation(animation, &data->bump_settle_anim_impl);

  return animation;
}

static void prv_run_value_change_animation(Layer *layer) {
  SelectionLayerData *data = layer_get_data(layer);
  Animation *bump_text = prv_create_bump_text_animation(layer);
#ifdef PBL_SDK_3
  Animation *bump_settle = prv_create_bump_settle_animation(layer);
  data->value_change_animation =
      animation_sequence_create(bump_text, bump_settle, NULL);
  //animation_set_auto_destroy(selection_layer->value_change_animation, true);
  animation_schedule(data->value_change_animation);
#else
  animation_schedule(bump_text);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! Slide Animation

//! This animation moves the "selection box" (active color) to the next cell to the right.
//! This animation has two parts:
//! 1) The "move and expand"
//! 2) The "settle"

//! The "move and expand" (slide) moves the selection box from the currently active cell to
//! the next cell to the right. At the same time the width is changed to be the size of the
//! next cell plus the size of the padding. This creates an overshoot effect.

//! The "settle" (slide_settle) removes the extra width that was added in the "move and expand"
//! step.

static void prv_slide_impl(struct Animation *animation,
                           const AnimationProgress distance_normalized) {
  Layer *layer = (Layer*) animation_get_context(animation);
  SelectionLayerData *data = layer_get_data(layer);

  // Update the completion percent of the animation
  data->slide_amin_progress = (100 * distance_normalized) / ANIMATION_NORMALIZED_MAX;
  layer_mark_dirty(layer);
}

static void prv_slide_stopped(Animation *animation, bool finished, void *context) {
  Layer *layer = (Layer*)animation_get_context(animation);
  SelectionLayerData *data = layer_get_data(layer);
  data->slide_amin_progress = 0;
  if (data->slide_is_forward)
    data->selected_cell_idx++;
  else
    data->selected_cell_idx--;
  // destroy
  animation_destroy(animation);

  // start next animation
#ifndef PBL_SDK_3
  Animation *settle_animation = prv_create_slide_settle_animation(layer);
  animation_schedule(settle_animation);
#endif
}

static void prv_slide_settle_impl(struct Animation *animation,
                                  const AnimationProgress distance_normalized) {
  Layer *layer = (Layer*)animation_get_context(animation);
  SelectionLayerData *data = layer_get_data(layer);

  // Update the completion percent of the animation. This is a reverse animation. It starts
  // fully drawn, then the amount drawn decreases
  data->slide_settle_anim_progress =
      100 - (100 * distance_normalized) / ANIMATION_NORMALIZED_MAX;
  layer_mark_dirty(layer);
}

static void prv_slide_settle_stopped(Animation *animation, bool finished, void *context) {
  Layer *layer = (Layer*) animation_get_context(animation);
  SelectionLayerData *data = layer_get_data(layer);
  data->slide_settle_anim_progress = 0;
  // destroy
  animation_destroy(animation);
}

static Animation* prv_create_slide_animation(Layer *layer) {
  SelectionLayerData *data = layer_get_data(layer);
  PropertyAnimation *slide_amin =
      property_animation_create_layer_frame(layer, NULL, NULL);
  Animation *animation = property_animation_get_animation(slide_amin);
  animation_set_curve(animation, AnimationCurveEaseIn);
  animation_set_duration(animation, SLIDE_DURATION_MS);
  AnimationHandlers anim_handler = {
      .stopped = prv_slide_stopped,
  };
  animation_set_handlers(animation, anim_handler, layer);

  data->slide_amin_impl = (AnimationImplementation) {
    .update = prv_slide_impl,
  };
  animation_set_implementation(animation, &data->slide_amin_impl);

  return animation;
}

static Animation* prv_create_slide_settle_animation(Layer *layer) {
  SelectionLayerData *data = layer_get_data(layer);
  PropertyAnimation *slide_settle_anim =
      property_animation_create_layer_frame(layer, NULL, NULL);
  Animation *animation = property_animation_get_animation(slide_settle_anim);
  animation_set_curve(animation, AnimationCurveEaseOut);
  animation_set_duration(animation, SLIDE_SETTLE_DURATION_MS);
  AnimationHandlers anim_handler = {
      .stopped = prv_slide_settle_stopped,
  };
  animation_set_handlers(animation, anim_handler, layer);

  data->slide_settle_anim_impl = (AnimationImplementation) {
    .update = prv_slide_settle_impl,
  };
  animation_set_implementation(animation, &data->slide_settle_anim_impl);

  return animation;
}

static void prv_run_slide_animation(Layer *layer) {
  SelectionLayerData *data = layer_get_data(layer);
  Animation *over_animation = prv_create_slide_animation(layer);
#ifdef PBL_SDK_3
  Animation *settle_animation = prv_create_slide_settle_animation(layer);
  data->next_cell_animation =
      animation_sequence_create(over_animation, settle_animation, NULL);
  //animation_set_auto_destroy(selection_layer->next_cell_animation, true);
  animation_schedule(data->next_cell_animation);
#else
  animation_schedule(over_animation);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! Click handlers
void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  Layer *layer = (Layer*)context;
  SelectionLayerData *data = layer_get_data(layer);
  if (data->is_active) {
    if (click_recognizer_is_repeating(recognizer)) {
      // Don't animate if the button is being held down. Just update the text
      data->callbacks.increment(data->selected_cell_idx, click_number_of_clicks_counted(recognizer),
          data->callback_context);
      layer_mark_dirty(layer);
    } else {
      // Run the animation. The decrement callback will be run halfway through
      data->bump_is_upwards = true;
      prv_run_value_change_animation(layer);
    }
  }
}

void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  Layer *layer = (Layer*)context;
  SelectionLayerData *data = layer_get_data(layer);
  if (data->is_active) {
    if (click_recognizer_is_repeating(recognizer)) {
      // Don't animate if the button is being held down. Just update the text
      data->callbacks.decrement(data->selected_cell_idx, click_number_of_clicks_counted(recognizer),
          data->callback_context);
      layer_mark_dirty(layer);
    } else {
      // Run the animation. The decrement callback will be run halfway through
      data->bump_is_upwards = false;
      prv_run_value_change_animation(layer);
    }
  }
}

void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  Layer *layer = (Layer*)context;
  SelectionLayerData *data = layer_get_data(layer);
  if (data->is_active) {
    animation_unschedule(data->next_cell_animation);
    if (data->selected_cell_idx >= data->num_cells - 1) {
      data->selected_cell_idx = 0;
      data->callbacks.complete(data->callback_context);
    } else {
      data->slide_is_forward = true;
      prv_run_slide_animation(layer);
    }
  }
}

void prv_back_click_handler(ClickRecognizerRef recognizer, void *context) {
  Layer *layer = (Layer*)context;
  SelectionLayerData *data = layer_get_data(layer);
  if (data->is_active) {
    animation_unschedule(data->next_cell_animation);
    if (data->selected_cell_idx == 0) {
      data->selected_cell_idx = 0;
      window_stack_pop(true);
    } else {
      data->slide_is_forward = false;
      prv_run_slide_animation(layer);
    }
  }
}

static void prv_click_config_provider(Layer *layer) {
  // Config UP / DOWN button behavior:
  window_set_click_context(BUTTON_ID_UP, layer);
  window_set_click_context(BUTTON_ID_DOWN, layer);
  window_set_click_context(BUTTON_ID_SELECT, layer);
  window_set_click_context(BUTTON_ID_BACK, layer);

  window_single_repeating_click_subscribe(BUTTON_ID_UP, BUTTON_HOLD_REPEAT_MS,
      prv_up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, BUTTON_HOLD_REPEAT_MS,
      prv_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, prv_back_click_handler);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//! API
Layer* selection_layer_init(SelectionLayerData *selection_layer_, GRect frame, int num_cells) {
  if (num_cells > MAX_SELECTION_LAYER_CELLS) {
    num_cells = MAX_SELECTION_LAYER_CELLS;
  }
  Layer *layer = layer_create_with_data(frame, sizeof(SelectionLayerData));
  SelectionLayerData *selection_layer_data = layer_get_data(layer);
  // Set layer defaults
  *selection_layer_data = (SelectionLayerData) {
#ifdef PBL_COLOR
    .active_background_color = DEFAULT_ACTIVE_COLOR,
    .inactive_background_color = DEFAULT_INACTIVE_COLOR,
#else
    .inverter = inverter_layer_create(GRect(0, 0, 0, frame.size.h)),
#endif
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

  // add inverter layer
#ifndef PBL_COLOR
  layer_add_child(layer, inverter_layer_get_layer(selection_layer_data->inverter));
#endif

  return layer;
}

Layer* selection_layer_create(GRect frame, int num_cells) {
  SelectionLayerData *selection_layer_data = NULL;
  Layer *layer = selection_layer_init(selection_layer_data, frame, num_cells);
  return layer;
}

void selection_layer_deinit(Layer* layer) {
  SelectionLayerData *data = layer_get_data(layer);

  // remove inverter layer
#ifndef PBL_COLOR
  inverter_layer_destroy(data->inverter);
#endif

  layer_destroy(layer);
}

void selection_layer_destroy(Layer* layer) {
  SelectionLayerData *data = layer_get_data(layer);
  animation_unschedule_all();
  if (data) {
    selection_layer_deinit(layer);
    //free(data);
  }
}

void selection_layer_set_cell_width(Layer *layer, int idx, int width) {
  SelectionLayerData *data = layer_get_data(layer);
  if (data && idx < data->num_cells) {
    data->cell_widths[idx] = width;
  }
#ifndef PBL_COLOR
  layer_set_bounds(inverter_layer_get_layer(data->inverter), GRect(0, 0, width, layer_get_bounds(inverter_layer_get_layer(data->inverter)).size.h));
#endif
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

void selection_layer_set_click_config_onto_window(Layer *layer,
                                                  struct Window *window) {
  if (layer && window) {
    window_set_click_config_provider_with_context(window,
        (ClickConfigProvider) prv_click_config_provider, layer);
  }
}

void selection_layer_set_callbacks(Layer *layer, void *callback_context,
                                   SelectionLayerCallbacks callbacks) {
  SelectionLayerData *data = layer_get_data(layer);
  data->callbacks = callbacks;
  data->callback_context = callback_context;
}