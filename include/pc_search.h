#ifndef PC_SEARCH_H
#define PC_SEARCH_H

#include "pc_component_data.h"

void pc_linear_search_by_name(const struct pc_component arr[], unsigned int count,
                              const char *keyword,
                              void (*display_func)(const struct pc_component *));

#endif /* PC_SEARCH_H */