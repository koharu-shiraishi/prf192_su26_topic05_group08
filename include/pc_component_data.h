#ifndef PC_COMPONENT_DATA_H
#define PC_COMPONENT_DATA_H

#include <stdint.h>

#define MAXIMUM_COMPONENTS 1000U
#define COMPONENT_NAME_LEN 40U
#define COMPONENT_SOCKET_LEN 20U

enum component_category
{
	CPU = 0,
	MAINBOARD,
	PSU,
	RAM,
	SSD,
	GPU,
	CASE,
	CATEGORY_COUNT
};

struct pc_component
{
	uint32_t id;
	enum component_category category;
	char name[COMPONENT_NAME_LEN];
	char socket[COMPONENT_SOCKET_LEN];
	uint32_t ddr;
	uint32_t stock;
	uint32_t warranty_period;
};

struct component_manager
{
	struct pc_component components[MAXIMUM_COMPONENTS];
	uint32_t count;
	uint32_t next_id; /* Monotonic auto-increment counter to prevent ID collisions */
};

#endif /* PC_COMPONENT_DATA_H */