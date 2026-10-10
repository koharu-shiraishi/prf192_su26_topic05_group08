#ifndef PC_VALIDATION_H
#define PC_VALIDATION_H

#include <stdint.h>

#include "pc_component_data.h"

enum pc_validation_status {
	PC_VALIDATION_OK = 0,
	PC_VALIDATION_ERR_ARGUMENT = -1,
	PC_VALIDATION_ERR_NUMBER = -2,
	PC_VALIDATION_ERR_CATEGORY = -3,
	PC_VALIDATION_ERR_NAME = -4,
	PC_VALIDATION_ERR_SOCKET = -5,
	PC_VALIDATION_ERR_DDR = -6,
	PC_VALIDATION_ERR_STOCK = -7,
	PC_VALIDATION_ERR_WARRANTY = -8
};

int pc_parse_u32(const char *text, uint32_t *value_p);
int pc_category_from_string(const char *text,
			    enum component_category *category_p);
int pc_validate_component_fields(const struct pc_component *component_p);
const char *pc_validation_status_string(int status);

#endif /* PC_VALIDATION_H */
