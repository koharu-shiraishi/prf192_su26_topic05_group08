#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "pc_component_data.h"
#include "pc_component_function.h"
#include "pc_sort.h"
#include "pc_search.h"

const char *category_names[CATEGORY_COUNT] = {
	"CPU", "MAINBOARD", "PSU", "RAM", "SSD", "GPU", "CASE"};

void component_mgr_init(struct component_manager *mgr_p)
{
	if (!mgr_p)
		return;
	memset(mgr_p, 0, sizeof(*mgr_p));
	mgr_p->count = 0;
	mgr_p->next_id = 1;
}

/* Helper: Sanitize field invariants right inside the data layer */
static void sanitize_component_fields(struct pc_component *comp_p)
{
	if (!comp_p)
		return;
	if (comp_p->category != CPU && comp_p->category != MAINBOARD)
	{
		snprintf(comp_p->socket, sizeof(comp_p->socket), "N/A");
	}
	if (comp_p->category != MAINBOARD && comp_p->category != RAM)
	{
		comp_p->ddr = 0;
	}
}

int component_add_item(struct component_manager *mgr_p, const struct pc_component *new_comp_p)
{
	if (!mgr_p || !new_comp_p)
		return -1;
	if (mgr_p->count >= MAXIMUM_COMPONENTS)
		return -2;
	if (new_comp_p->category >= CATEGORY_COUNT)
		return -3;

	struct pc_component *dest_p = &mgr_p->components[mgr_p->count];
	memset(dest_p, 0, sizeof(*dest_p));

	snprintf(dest_p->name, sizeof(dest_p->name), "%s", new_comp_p->name);
	snprintf(dest_p->socket, sizeof(dest_p->socket), "%s", new_comp_p->socket);

	dest_p->category = new_comp_p->category;
	dest_p->ddr = new_comp_p->ddr;
	dest_p->stock = new_comp_p->stock;
	dest_p->warranty_period = new_comp_p->warranty_period;

	sanitize_component_fields(dest_p);

	dest_p->id = mgr_p->next_id++;
	mgr_p->count++;

	return 0;
}

int component_update_item(struct component_manager *mgr_p, uint32_t id, const struct pc_component *updated_data)
{
	if (!mgr_p || !updated_data || id == 0)
		return -2;
	if (updated_data->category >= CATEGORY_COUNT)
		return -3;

	struct pc_component *target = component_find_by_id(mgr_p, id);
	if (!target)
		return -1;

	snprintf(target->name, sizeof(target->name), "%s", updated_data->name);
	snprintf(target->socket, sizeof(target->socket), "%s", updated_data->socket);

	target->category = updated_data->category;
	target->ddr = updated_data->ddr;
	target->stock = updated_data->stock;
	target->warranty_period = updated_data->warranty_period;

	sanitize_component_fields(target);

	return 0;
}

int component_delete_item(struct component_manager *mgr_p, uint32_t id)
{
	if (!mgr_p || id == 0)
		return -2;

	uint32_t target_index = MAXIMUM_COMPONENTS;
	for (uint32_t i = 0; i < mgr_p->count; i++)
	{
		if (mgr_p->components[i].id == id)
		{
			target_index = i;
			break;
		}
	}

	if (target_index == MAXIMUM_COMPONENTS)
		return -1;

	for (uint32_t i = target_index; i < mgr_p->count - 1; i++)
	{
		mgr_p->components[i] = mgr_p->components[i + 1];
	}

	mgr_p->count--;
	memset(&mgr_p->components[mgr_p->count], 0, sizeof(struct pc_component));

	return 0;
}

struct pc_component *component_find_by_id(struct component_manager *mgr_p, uint32_t id)
{
	if (!mgr_p || id == 0)
		return NULL;
	for (uint32_t i = 0; i < mgr_p->count; i++)
	{
		if (mgr_p->components[i].id == id)
			return &mgr_p->components[i];
	}
	return NULL;
}

struct pc_component *component_get_by_index(struct component_manager *mgr_p, uint32_t index)
{
	if (!mgr_p || index >= mgr_p->count)
		return NULL;
	return &mgr_p->components[index];
}

/* --- Comparator Functions & Sorting --- */

/* Standard 3-way comparator (<0, 0, >0) for DESCENDING stock order */
static int compare_stock_desc(const struct pc_component *a_p, const struct pc_component *b_p)
{
	if (!a_p || !b_p)
		return 0;
	if (a_p->stock > b_p->stock)
		return -1;
	if (a_p->stock < b_p->stock)
		return 1;
	return 0;
}

/* Standard 3-way comparator (<0, 0, >0) for ASCENDING stock order */
static int compare_stock_asc(const struct pc_component *a_p, const struct pc_component *b_p)
{
	if (!a_p || !b_p)
		return 0;
	if (a_p->stock < b_p->stock)
		return -1;
	if (a_p->stock > b_p->stock)
		return 1;
	return 0;
}

/* Standard 3-way comparator (<0, 0, >0) for DESCENDING warranty order */
static int compare_period_desc(const struct pc_component *a_p, const struct pc_component *b_p)
{
	if (!a_p || !b_p)
		return 0;
	if (a_p->warranty_period > b_p->warranty_period)
		return -1;
	if (a_p->warranty_period < b_p->warranty_period)
		return 1;
	return 0;
}

void component_sort_by_stock_desc(struct component_manager *mgr_p)
{
	if (!mgr_p || mgr_p->count <= 1)
		return;
	universal_merge_sort(mgr_p->components, 0, (int)(mgr_p->count - 1), compare_stock_desc);
}

void component_sort_by_stock_asc(struct component_manager *mgr_p)
{
	if (!mgr_p || mgr_p->count <= 1)
		return;
	universal_merge_sort(mgr_p->components, 0, (int)(mgr_p->count - 1), compare_stock_asc);
}

void component_sort_by_period_desc(struct component_manager *mgr_p)
{
	if (!mgr_p || mgr_p->count <= 1)
		return;
	universal_merge_sort(mgr_p->components, 0, (int)(mgr_p->count - 1), compare_period_desc);
}