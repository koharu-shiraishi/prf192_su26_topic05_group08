#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pc_invoice_function.h"
#include "pc_component_function.h"
#include "invoice_sort.h"
#include "invoice_search.h"

const char *status_names[STATUS_COUNT] = {
	"PENDING", "ASSEMBLING", "COMPLETED", "CANCELLED"};

void invoice_mgr_init(struct invoice_manager *mgr_p)
{
	if (!mgr_p)
		return;
	memset(mgr_p, 0, sizeof(*mgr_p));
	mgr_p->count = 0;
	mgr_p->next_id = 1;
}

int invoice_add(struct invoice_manager *mgr_p, const struct invoice *new_inv_p)
{
	if (!mgr_p || !new_inv_p)
		return -1;
	if (mgr_p->count >= MAXIMUM_INVOICES)
		return -2;

	struct invoice *dest_p = &mgr_p->invoices[mgr_p->count];
	memcpy(dest_p, new_inv_p, sizeof(*dest_p));

	dest_p->id = mgr_p->next_id++;
	mgr_p->count++;
	return 0;
}

int invoice_update(struct invoice_manager *mgr_p, uint32_t id, const struct invoice *update_p)
{
	if (!mgr_p || !update_p || id == 0)
		return -1;

	struct invoice *target = invoice_find_by_id(mgr_p, id);
	if (!target)
		return -2;

	/* Retain original ID, overwrite rest */
	uint32_t original_id = target->id;
	memcpy(target, update_p, sizeof(*target));
	target->id = original_id;

	return 0;
}

int invoice_delete(struct invoice_manager *mgr_p, uint32_t id)
{
	if (!mgr_p || id == 0)
		return -1;

	uint32_t idx = MAXIMUM_INVOICES;
	for (uint32_t i = 0; i < mgr_p->count; i++)
	{
		if (mgr_p->invoices[i].id == id)
		{
			idx = i;
			break;
		}
	}

	if (idx == MAXIMUM_INVOICES)
		return -2;

	for (uint32_t i = idx; i < mgr_p->count - 1; i++)
	{
		mgr_p->invoices[i] = mgr_p->invoices[i + 1];
	}

	mgr_p->count--;
	memset(&mgr_p->invoices[mgr_p->count], 0, sizeof(struct invoice));
	return 0;
}

struct invoice *invoice_find_by_id(struct invoice_manager *mgr_p, uint32_t id)
{
	if (!mgr_p || id == 0)
		return NULL;
	for (uint32_t i = 0; i < mgr_p->count; i++)
	{
		if (mgr_p->invoices[i].id == id)
			return &mgr_p->invoices[i];
	}
	return NULL;
}

int invoice_check_compatibility(const struct invoice *inv_p, struct component_manager *comp_mgr_p, char *err_msg, size_t err_size)
{
	if (!inv_p || !comp_mgr_p || !err_msg)
		return -1;

	struct pc_component *cpu = NULL;
	struct pc_component *mb = NULL;
	struct pc_component *ram = NULL;

	for (uint32_t i = 0; i < inv_p->comp_count; i++)
	{
		struct pc_component *c = component_find_by_id(comp_mgr_p, inv_p->component_ids[i]);
		if (!c)
			continue;
		if (c->category == CPU)
			cpu = c;
		if (c->category == MAINBOARD)
			mb = c;
		if (c->category == RAM)
			ram = c;
	}

	/* Socket Check */
	if (cpu && mb && strcmp(cpu->socket, mb->socket) != 0)
	{
		snprintf(err_msg, err_size, "[FATAL] Socket mismatch: CPU (%s) vs Mainboard (%s)", cpu->socket, mb->socket);
		return -1;
	}

	/* DDR Check */
	if (ram && mb && ram->ddr != mb->ddr)
	{
		snprintf(err_msg, err_size, "[FATAL] Memory mismatch: RAM (DDR%u) vs Mainboard (DDR%u)", ram->ddr, mb->ddr);
		return -1;
	}

	snprintf(err_msg, err_size, "Fully Compatible");
	return 0;
}

/* --- Comparators for Sorting --- */
static int compare_invoice_id_asc(const struct invoice *a, const struct invoice *b)
{
	if (a->id < b->id)
		return -1;
	if (a->id > b->id)
		return 1;
	return 0;
}

static int compare_invoice_id_desc(const struct invoice *a, const struct invoice *b)
{
	if (a->id > b->id)
		return -1;
	if (a->id < b->id)
		return 1;
	return 0;
}

void invoice_sort_by_id_asc(struct invoice_manager *mgr_p)
{
	if (mgr_p && mgr_p->count > 1)
		invoice_universal_merge_sort(mgr_p->invoices, 0, (int)(mgr_p->count - 1), compare_invoice_id_asc);
}

void invoice_sort_by_id_desc(struct invoice_manager *mgr_p)
{
	if (mgr_p && mgr_p->count > 1)
		invoice_universal_merge_sort(mgr_p->invoices, 0, (int)(mgr_p->count - 1), compare_invoice_id_desc);
}

/* --- Match Functions for Searching --- */
static int match_invoice_by_id(const struct invoice *inv, const void *key)
{
	uint32_t search_id = *(const uint32_t *)key;
	return (inv->id == search_id) ? 1 : 0;
}