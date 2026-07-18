#ifndef PC_COMPONENT_FUNCTION_H
#define PC_COMPONENT_FUNCTION_H

#include "pc_component_data.h"

extern const char *category_names[CATEGORY_COUNT];

/* ========================================================================= */
/* CORE LOGIC LAYER (No I/O, No Printf/Scanf)                                */
/* ========================================================================= */
void component_mgr_init(struct component_manager *mgr_p);
int component_add_item(struct component_manager *mgr_p, const struct pc_component *new_comp_p);
int component_update_item(struct component_manager *mgr_p, uint32_t id, const struct pc_component *updated_data);
int component_delete_item(struct component_manager *mgr_p, uint32_t id);
struct pc_component *component_find_by_id(struct component_manager *mgr_p, uint32_t id);
struct pc_component *component_get_by_index(struct component_manager *mgr_p, uint32_t index);

void component_sort_by_stock_desc(struct component_manager *mgr_p);
void component_sort_by_stock_asc(struct component_manager *mgr_p);
void component_sort_by_period_desc(struct component_manager *mgr_p);

/* ========================================================================= */
/* USER INTERFACE LAYER (UI / IO - Contains Printf/Scanf)                    */
/* ========================================================================= */
void component_ui_add(struct component_manager *mgr_p);
void component_ui_show_single(const struct pc_component *comp_p);
void component_ui_show_all(struct component_manager *mgr_p);
void component_ui_track_low_stock(struct component_manager *mgr_p);
void component_ui_sort_menu(struct component_manager *mgr_p);
void component_ui_update(struct component_manager *mgr_p);
void component_ui_delete(struct component_manager *mgr_p);

#endif /* PC_COMPONENT_FUNCTION_H */