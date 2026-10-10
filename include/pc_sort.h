#ifndef PC_SORT_H
#define PC_SORT_H

#include <stddef.h>

#include "pc_component_data.h"

typedef int (*pc_component_compare_fn)(const struct pc_component *left_p,
				       const struct pc_component *right_p);

/*
 * Stable merge sort. A comparator returns a negative value when left should
 * precede right, zero when equivalent, and a positive value otherwise.
 */
int universal_merge_sort(struct pc_component array[], size_t count,
			 pc_component_compare_fn compare);

#endif /* PC_SORT_H */
