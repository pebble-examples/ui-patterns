#pragma once

#include <pebble.h>

#define PIN_WINDOW_SPACING 19

typedef struct {
  int digits[4];
} PIN;

void pin_window_push();
