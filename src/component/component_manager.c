#include "component_manager.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "component_validation.h"

static enum component_status check_manager_bounds(
	const struct component_manager *manager)
{
	if (manager == NULL)
		return COMPONENT_STATUS_INVALID_ARGUMENT;
	if (manager->count > COMPONENT_MAX_COUNT)
		return COMPONENT_STATUS_CORRUPT_MANAGER;
	return COMPONENT_STATUS_OK;
}

enum component_status component_manager_init(struct component_manager *manager)
{
	if (manager == NULL)
		return COMPONENT_STATUS_INVALID_ARGUMENT;

	memset(manager, 0, sizeof(*manager));
	manager->next_id = 1U;
	return COMPONENT_STATUS_OK;
}

enum component_status component_manager_validate(
	const struct component_manager *manager)
{
	enum component_status status = check_manager_bounds(manager);

	if (status != COMPONENT_STATUS_OK)
		return status;

	for (size_t index = 0; index < manager->count; index++) {
		const struct component *item = &manager->items[index];

		if (item->id == 0U)
			return COMPONENT_STATUS_INVALID_ID;
		if (manager->next_id != 0U && item->id >= manager->next_id)
			return COMPONENT_STATUS_CORRUPT_MANAGER;

		status = component_validate_fields(item);
		if (status != COMPONENT_STATUS_OK)
			return status;

		for (size_t other = 0; other < index; other++) {
			if (manager->items[other].id == item->id)
				return COMPONENT_STATUS_DUPLICATE_ID;
		}
	}

	return COMPONENT_STATUS_OK;
}

enum component_status component_manager_add(
	struct component_manager *manager,
	const struct component *new_component,
	uint32_t *created_id)
{
	struct component draft;
	enum component_status status;
	uint32_t assigned_id;

	if (manager == NULL || new_component == NULL)
		return COMPONENT_STATUS_INVALID_ARGUMENT;

	/* Sao chép trước: new_component có thể nằm trong mảng manager. */
	draft = *new_component;
	status = component_manager_validate(manager);
	if (status != COMPONENT_STATUS_OK)
		return status;
	if (manager->count == COMPONENT_MAX_COUNT)
		return COMPONENT_STATUS_FULL;
	if (manager->next_id == 0U)
		return COMPONENT_STATUS_ID_EXHAUSTED;

	status = component_validate_fields(&draft);
	if (status != COMPONENT_STATUS_OK)
		return status;

	assigned_id = manager->next_id;
	draft.id = assigned_id;
	manager->items[manager->count] = draft;
	manager->count++;
	manager->next_id = assigned_id == UINT32_MAX ? 0U : assigned_id + 1U;
	if (created_id != NULL)
		*created_id = assigned_id;
	return COMPONENT_STATUS_OK;
}

enum component_status component_manager_update(
	struct component_manager *manager, uint32_t id,
	const struct component *replacement)
{
	struct component draft;
	enum component_status status;

	if (manager == NULL || replacement == NULL)
		return COMPONENT_STATUS_INVALID_ARGUMENT;
	if (id == 0U)
		return COMPONENT_STATUS_INVALID_ID;

	/* Cho phép replacement chính là phần tử đang được sửa. */
	draft = *replacement;
	status = component_manager_validate(manager);
	if (status != COMPONENT_STATUS_OK)
		return status;
	status = component_validate_fields(&draft);
	if (status != COMPONENT_STATUS_OK)
		return status;

	for (size_t index = 0; index < manager->count; index++) {
		if (manager->items[index].id == id) {
			draft.id = id;
			manager->items[index] = draft;
			return COMPONENT_STATUS_OK;
		}
	}
	return COMPONENT_STATUS_NOT_FOUND;
}

enum component_status component_manager_delete(
	struct component_manager *manager, uint32_t id)
{
	enum component_status status;

	if (manager == NULL)
		return COMPONENT_STATUS_INVALID_ARGUMENT;
	if (id == 0U)
		return COMPONENT_STATUS_INVALID_ID;
	status = component_manager_validate(manager);
	if (status != COMPONENT_STATUS_OK)
		return status;

	for (size_t index = 0; index < manager->count; index++) {
		if (manager->items[index].id != id)
			continue;
		for (size_t next = index + 1U; next < manager->count; next++)
			manager->items[next - 1U] = manager->items[next];
		manager->count--;
		memset(&manager->items[manager->count], 0,
		       sizeof(manager->items[manager->count]));
		return COMPONENT_STATUS_OK;
	}
	return COMPONENT_STATUS_NOT_FOUND;
}

enum component_status component_manager_get_by_id(
	const struct component_manager *manager, uint32_t id,
	struct component *output)
{
	enum component_status status;

	if (output == NULL)
		return COMPONENT_STATUS_INVALID_ARGUMENT;
	status = check_manager_bounds(manager);
	if (status != COMPONENT_STATUS_OK)
		return status;
	if (id == 0U)
		return COMPONENT_STATUS_INVALID_ID;

	for (size_t index = 0; index < manager->count; index++) {
		if (manager->items[index].id == id) {
			*output = manager->items[index];
			return COMPONENT_STATUS_OK;
		}
	}
	return COMPONENT_STATUS_NOT_FOUND;
}

enum component_status component_manager_get_by_index(
	const struct component_manager *manager, size_t index,
	struct component *output)
{
	enum component_status status;

	if (output == NULL)
		return COMPONENT_STATUS_INVALID_ARGUMENT;
	status = check_manager_bounds(manager);
	if (status != COMPONENT_STATUS_OK)
		return status;
	if (index >= manager->count)
		return COMPONENT_STATUS_INDEX_OUT_OF_RANGE;

	*output = manager->items[index];
	return COMPONENT_STATUS_OK;
}
