#if defined(PBL_SDK_2)
#pragma once

#include <pebble.h>

#ifndef STATUS_BAR_LAYER_HEIGHT
#define STATUS_BAR_LAYER_HEIGHT PBL_IF_ROUND_ELSE(24, 16)
#endif

typedef struct {
  int top;
  int right;
  int bottom;
  int left;
} GEdgeInsets;

GRect grect_inset(GRect source, GEdgeInsets insets);

#define GEdgeInsets(t, r, b, l) ((GEdgeInsets){.top=(t),.right=(r),.bottom=(b),.left=(l)})
#endif
