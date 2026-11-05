#include "component_status.h"

const char *component_status_message(enum component_status status)
{
	switch (status) {
	case COMPONENT_STATUS_OK:
		return "Success";
	case COMPONENT_STATUS_INVALID_ARGUMENT:
		return "Invalid argument";
	case COMPONENT_STATUS_INVALID_ID:
		return "ID must be a positive 32-bit number";
	case COMPONENT_STATUS_NOT_FOUND:
		return "Component not found";
	case COMPONENT_STATUS_INDEX_OUT_OF_RANGE:
		return "Index out of range";
	case COMPONENT_STATUS_FULL:
		return "The component list is full";
	case COMPONENT_STATUS_DUPLICATE_ID:
		return "Duplicate component ID";
	case COMPONENT_STATUS_ID_EXHAUSTED:
		return "The increasing ID range is exhausted";
	case COMPONENT_STATUS_CORRUPT_MANAGER:
		return "Invalid manager state";
	case COMPONENT_STATUS_INVALID_CATEGORY:
		return "Invalid component category";
	case COMPONENT_STATUS_INVALID_NAME:
		return "Invalid component name";
	case COMPONENT_STATUS_INVALID_SOCKET:
		return "Invalid socket";
	case COMPONENT_STATUS_INVALID_DDR:
		return "Invalid DDR generation for this category";
	case COMPONENT_STATUS_INVALID_STOCK:
		return "Stock is outside the allowed range";
	case COMPONENT_STATUS_INVALID_WARRANTY:
		return "Warranty is outside the allowed range";
	case COMPONENT_STATUS_INVALID_NUMBER:
		return "Invalid unsigned 32-bit decimal number";
	case COMPONENT_STATUS_INVALID_SEARCH_TEXT:
		return "Search text must not be empty";
	case COMPONENT_STATUS_INVALID_SORT_ORDER:
		return "Invalid sort order";
	}

	return "Unknown component status";
}
