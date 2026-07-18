/* invoice_ui.h */
#ifndef INVOICE_UI_H
#define INVOICE_UI_H

#include "pc_invoice_data.h"
#include "pc_component_data.h"

void invoice_ui_create(struct invoice_manager *i_mgr, struct component_manager *c_mgr);
void invoice_ui_show_all(struct invoice_manager *i_mgr);
void invoice_ui_show_single(struct invoice_manager *i_mgr);
void invoice_ui_edit(struct invoice_manager *i_mgr);
void invoice_ui_remove(struct invoice_manager *i_mgr);
void invoice_ui_check_status(struct invoice_manager *i_mgr, struct component_manager *c_mgr);

#endif /* INVOICE_UI_H */