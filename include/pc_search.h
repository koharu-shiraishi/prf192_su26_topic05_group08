#ifndef PC_SEARCH_H
#define PC_SEARCH_H

#include <stddef.h>

#include "pc_component_data.h"

typedef void (*pc_component_display_fn)(const struct pc_component *component_p);

/* Portable, case-insensitive substring search. */
size_t pc_linear_search_by_name(const struct pc_component array[], size_t count,
				const char *keyword,
				pc_component_display_fn display_func);

#endif /* PC_SEARCH_H */
