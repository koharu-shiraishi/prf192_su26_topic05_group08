#ifndef PC_COMPONENT_UI_H
#define PC_COMPONENT_UI_H

#include <stdint.h>

#include "pc_component_data.h"

int component_ui_prompt_main_choice(uint32_t *choice_p);
int component_ui_confirm(const char *prompt);

void component_ui_show_single(const struct pc_component *component_p);
void component_ui_show_all(const struct component_manager *mgr_p);
void component_ui_track_low_stock(const struct component_manager *mgr_p);
void component_ui_search(const struct component_manager *mgr_p);

/* Return 1 when data/order changed, 0 when unchanged, and -1 on EOF. */
int component_ui_add(struct component_manager *mgr_p);
int component_ui_update(struct component_manager *mgr_p);
int component_ui_delete(struct component_manager *mgr_p);
int component_ui_sort_menu(struct component_manager *mgr_p);

#endif /* PC_COMPONENT_UI_H */
