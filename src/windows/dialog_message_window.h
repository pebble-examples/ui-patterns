#pragma once

#include <pebble.h>

#define DIALOG_MESSAGE_WINDOW_MESSAGE  "Battery is low! Connect the charger."
#define DIALOG_MESSAGE_WINDOW_MARGIN   10

void dialog_message_window_push();
