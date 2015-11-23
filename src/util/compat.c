#if defined(PBL_SDK_2)
#include "compat.h"

GRect grect_inset(GRect source, GEdgeInsets insets) {
  return GRect(
    source.origin.x + insets.left,
    source.origin.y + insets.top,
    source.size.w - (insets.left + insets.right),
    source.size.h - (insets.top + insets.bottom));
}
#endif
