/* invoice_file.h */
#ifndef INVOICE_FILE_H
#define INVOICE_FILE_H
#include "pc_invoice_data.h"

void file_load_invoices(struct invoice_manager *mgr_p);
void file_save_invoices(struct invoice_manager *mgr_p);

#endif /* INVOICE_FILE_H */