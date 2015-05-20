#pragma once

#include <pebble.h>

#define ANIMATION_WINDOW_DURATION 40   // Duration of each half of the animation
#define ANIMATION_WINDOW_DISTANCE 5    // Pixels the animating text move by
#define ANIMATION_WINDOW_INTERVAL 1000 // Interval between timers

void animation_window_push();
