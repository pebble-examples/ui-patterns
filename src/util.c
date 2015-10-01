#include "util.h"

bool menu_layer_menu_index_selected(MenuLayer *menu_layer, MenuIndex *index) {
  MenuIndex selected = menu_layer_get_selected_index(menu_layer);
  return selected.row == index->row && selected.section == index->section;
}
