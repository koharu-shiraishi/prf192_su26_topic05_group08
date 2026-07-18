#ifndef INVOICE_SORT_H
#define INVOICE_SORT_H

#include "pc_invoice_data.h"

void invoice_universal_merge_sort(struct invoice arr[], int left, int right,
                                  int (*compare)(const struct invoice *, const struct invoice *));

#endif /* INVOICE_SORT_H */