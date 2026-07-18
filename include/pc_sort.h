#ifndef PC_SORT_H
#define PC_SORT_H

#include "pc_component_data.h"

/* Khai báo prototype rõ ràng bằng struct tường minh, không dùng typedef ẩn */
void universal_merge_sort(struct pc_component array[], int left, int right,
                          int (*compare)(const struct pc_component *, const struct pc_component *));

#endif /* PC_SORT_H */