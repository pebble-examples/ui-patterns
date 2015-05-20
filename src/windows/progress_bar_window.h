#pragma once

#include <pebble.h>

#define PROGRESS_BAR_WINDOW_SIZE GSize(144, STATUS_BAR_LAYER_HEIGHT) // System default
#define PROGRESS_BAR_WINDOW_DELTA 33

void progress_bar_window_push();