#ifndef INVOICE_SEARCH_H
#define INVOICE_SEARCH_H

#include "pc_invoice_data.h"

/* * match_func: Returns 1 if matched, 0 otherwise.
 * display_func: Callback to UI layer to print the data.
 */
void invoice_linear_search(const struct invoice arr[], unsigned int count,
                           const void *search_key,
                           int (*match_func)(const struct invoice *, const void *),
                           void (*display_func)(const struct invoice *));

#endif /* INVOICE_SEARCH_H */