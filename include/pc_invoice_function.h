#ifndef INVOICE_CORE_H
#define INVOICE_CORE_H

#include "pc_invoice_data.h"

void invoice_mgr_init(struct invoice_manager *mgr_p);
int invoice_add(struct invoice_manager *mgr_p, const struct invoice *new_inv_p);
int invoice_update(struct invoice_manager *mgr_p, uint32_t id, const struct invoice *update_p);
int invoice_delete(struct invoice_manager *mgr_p, uint32_t id);
struct invoice *invoice_find_by_id(struct invoice_manager *mgr_p, uint32_t id);

/* Compatibility checking */
int invoice_check_compatibility(const struct invoice *inv_p, struct component_manager *comp_mgr_p, char *err_msg, size_t err_size);

/* Sorting */
void invoice_sort_by_id_asc(struct invoice_manager *mgr_p);
void invoice_sort_by_id_desc(struct invoice_manager *mgr_p);

#endif /* INVOICE_CORE_H */