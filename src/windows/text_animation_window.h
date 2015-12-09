#pragma once

#include <pebble.h>

#define TEXT_ANIMATION_WINDOW_DURATION 40   // Duration of each half of the animation
#define TEXT_ANIMATION_WINDOW_DISTANCE 5    // Pixels the animating text move by
#define TEXT_ANIMATION_WINDOW_INTERVAL 1000 // Interval between timers

void text_animation_window_push();
