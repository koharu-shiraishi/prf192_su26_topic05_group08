#ifndef PC_COMPONENT_FUNCTION_H
#define PC_COMPONENT_FUNCTION_H

#include <stddef.h>
#include <stdint.h>

#include "pc_component_data.h"

enum component_status
{
    COMPONENT_OK = 0,
    COMPONENT_ERR_ARGUMENT = -1,
    COMPONENT_ERR_FULL = -2,
    COMPONENT_ERR_INVALID_DATA = -3,
    COMPONENT_ERR_NOT_FOUND = -4,
    COMPONENT_ERR_ID_EXHAUSTED = -5,
    COMPONENT_ERR_MEMORY = -6
};

extern const char *const category_names[CATEGORY_COUNT];

void component_mgr_init(struct component_manager *mgr_p);
void component_mgr_recalculate_next_id(struct component_manager *mgr_p);

int component_add_item(struct component_manager *mgr_p,
                       const struct pc_component *new_comp_p);
int component_update_item(struct component_manager *mgr_p, uint32_t id,
                          const struct pc_component *updated_data_p);
int component_delete_item(struct component_manager *mgr_p, uint32_t id);

struct pc_component *component_find_by_id(struct component_manager *mgr_p,
                                          uint32_t id);
const struct pc_component *
component_find_by_id_const(const struct component_manager *mgr_p, uint32_t id);
struct pc_component *component_get_by_index(struct component_manager *mgr_p,
                                            size_t index);
const struct pc_component *
component_get_by_index_const(const struct component_manager *mgr_p, size_t index);

int component_sort_by_stock_desc(struct component_manager *mgr_p);
int component_sort_by_stock_asc(struct component_manager *mgr_p);
int component_sort_by_period_desc(struct component_manager *mgr_p);

const char *component_status_string(int status);

#endif /* PC_COMPONENT_FUNCTION_H */
