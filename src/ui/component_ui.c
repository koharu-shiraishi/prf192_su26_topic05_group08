#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "pc_component_data.h"
#include "pc_component_function.h"

#define LOW_STOCK_THRESHOLD 5U

static int ui_safe_readline(char *buffer, size_t size)
{
	if (!fgets(buffer, (int)size, stdin))
		return -1;

	size_t len = strlen(buffer);
	if (len > 0 && buffer[len - 1] == '\n')
	{
		buffer[len - 1] = '\0';
	}
	else
	{
		int c;
		while ((c = getchar()) != '\n' && c != EOF)
			;
	}
	return 0;
}

static int ui_parse_uint32(const char *buffer, uint32_t *out_val)
{
	if (!buffer || !out_val || *buffer == '\0')
		return -1;

	while (isspace((unsigned char)*buffer))
		buffer++;

	if (*buffer == '-')
		return -1;

	char *endptr = NULL;
	unsigned long val = strtoul(buffer, &endptr, 10);

	if (buffer == endptr || (*endptr != '\0' && !isspace((unsigned char)*endptr)))
		return -1;
	if (val > UINT32_MAX)
		return -1;

	*out_val = (uint32_t)val;
	return 0;
}

static enum component_category ui_prompt_category(void)
{
	char line[64];
	uint32_t choice = 0;
	do
	{
		printf("Choose category:\n");
		for (int i = 0; i < CATEGORY_COUNT; i++)
			printf("%d. %s\n", i + 1, category_names[i]);
		printf("Your choice: ");

		if (ui_safe_readline(line, sizeof(line)) != 0)
		{
			return CPU;
		}
		if (ui_parse_uint32(line, &choice) == 0 && choice >= 1 && choice <= CATEGORY_COUNT)
		{
			break;
		}
		printf("[Error] Invalid choice. Please enter a number between 1 and %d.\n", CATEGORY_COUNT);
	} while (1);

	return (enum component_category)(choice - 1);
}

void component_ui_show_single(const struct pc_component *comp_p)
{
	if (!comp_p)
		return;
	const char *cat_name = (comp_p->category < CATEGORY_COUNT) ? category_names[comp_p->category] : "UNKNOWN";

	printf("ID: %u | %-10s | Name: %-20s | Socket: %-7s | DDR%u | Warranty: %u mos | Stock: %u\n",
		   comp_p->id, cat_name, comp_p->name,
		   comp_p->socket, comp_p->ddr, comp_p->warranty_period, comp_p->stock);
}

void component_ui_add(struct component_manager *mgr_p)
{
	if (!mgr_p || mgr_p->count >= MAXIMUM_COMPONENTS)
	{
		printf("Error: List full!\n");
		return;
	}

	struct pc_component tmp;
	memset(&tmp, 0, sizeof(tmp));

	printf("\n--- ADD COMPONENT ---\n");
	tmp.category = ui_prompt_category();

	printf("Enter Name: ");
	if (ui_safe_readline(tmp.name, sizeof(tmp.name)) != 0 || strlen(tmp.name) == 0)
		snprintf(tmp.name, sizeof(tmp.name), "Unknown");

	if (tmp.category == CPU || tmp.category == MAINBOARD)
	{
		printf("Enter Socket: ");
		if (ui_safe_readline(tmp.socket, sizeof(tmp.socket)) != 0 || strlen(tmp.socket) == 0)
			snprintf(tmp.socket, sizeof(tmp.socket), "N/A");
	}
	else
	{
		snprintf(tmp.socket, sizeof(tmp.socket), "N/A");
	}

	if (tmp.category == MAINBOARD || tmp.category == RAM)
	{
		char line[64];
		do
		{
			printf("Enter DDR (4 or 5): ");
			if (ui_safe_readline(line, sizeof(line)) == 0 && ui_parse_uint32(line, &tmp.ddr) == 0)
			{
				if (tmp.ddr == 4 || tmp.ddr == 5)
					break;
			}
			printf("[Error] DDR must be exact integer 4 or 5.\n");
		} while (1);
	}
	else
	{
		tmp.ddr = 0;
	}

	char num_buf[64];
	printf("Enter Stock Quantity: ");
	if (ui_safe_readline(num_buf, sizeof(num_buf)) != 0 || ui_parse_uint32(num_buf, &tmp.stock) != 0)
		tmp.stock = 0;

	printf("Enter Warranty Period (months): ");
	if (ui_safe_readline(num_buf, sizeof(num_buf)) != 0 || ui_parse_uint32(num_buf, &tmp.warranty_period) != 0)
		tmp.warranty_period = 0;

	int status = component_add_item(mgr_p, &tmp);
	if (status == 0)
		printf("[Success] Added component with ID: %u\n", mgr_p->components[mgr_p->count - 1].id);
	else
		printf("[Error] Failed to add. Code: %d\n", status);
}

void component_ui_update(struct component_manager *mgr_p)
{
	if (!mgr_p || mgr_p->count == 0)
	{
		printf("No components available to update.\n");
		return;
	}

	char line[64];
	uint32_t search_id = 0;
	printf("\nEnter Component ID to update: ");
	if (ui_safe_readline(line, sizeof(line)) != 0 || ui_parse_uint32(line, &search_id) != 0 || search_id == 0)
	{
		printf("[Error] Invalid ID input.\n");
		return;
	}

	struct pc_component *existing = component_find_by_id(mgr_p, search_id);
	if (!existing)
	{
		printf("[Error] Product not found.\n");
		return;
	}

	printf("\n--- CURRENT DATA ---\n");
	component_ui_show_single(existing);

	struct pc_component tmp;
	memset(&tmp, 0, sizeof(tmp));

	printf("\n--- ENTER NEW DATA ---\n");
	printf("Enter New Name: ");
	if (ui_safe_readline(tmp.name, sizeof(tmp.name)) != 0 || strlen(tmp.name) == 0)
		snprintf(tmp.name, sizeof(tmp.name), "%s", existing->name);

	tmp.category = existing->category;

	if (existing->category == CPU || existing->category == MAINBOARD)
	{
		printf("Enter New Socket: ");
		if (ui_safe_readline(tmp.socket, sizeof(tmp.socket)) != 0 || strlen(tmp.socket) == 0)
			snprintf(tmp.socket, sizeof(tmp.socket), "%s", existing->socket);

		do
		{
			printf("Enter New DDR (0, 4, or 5): ");
			if (ui_safe_readline(line, sizeof(line)) == 0 && ui_parse_uint32(line, &tmp.ddr) == 0)
			{
				if (tmp.ddr == 0 || tmp.ddr == 4 || tmp.ddr == 5)
					break;
			}
			printf("[Error] Invalid DDR.\n");
		} while (1);
	}
	else if (existing->category == RAM)
	{
		snprintf(tmp.socket, sizeof(tmp.socket), "N/A");
		do
		{
			printf("Enter New DDR (4 or 5): ");
			if (ui_safe_readline(line, sizeof(line)) == 0 && ui_parse_uint32(line, &tmp.ddr) == 0)
			{
				if (tmp.ddr == 4 || tmp.ddr == 5)
					break;
			}
			printf("[Error] RAM requires DDR 4 or 5.\n");
		} while (1);
	}
	else
	{
		snprintf(tmp.socket, sizeof(tmp.socket), "N/A");
		tmp.ddr = 0;
	}

	printf("Enter New Warranty Period (months): ");
	if (ui_safe_readline(line, sizeof(line)) != 0 || ui_parse_uint32(line, &tmp.warranty_period) != 0)
		tmp.warranty_period = existing->warranty_period;

	printf("Enter New Stock Quantity: ");
	if (ui_safe_readline(line, sizeof(line)) != 0 || ui_parse_uint32(line, &tmp.stock) != 0)
		tmp.stock = existing->stock;

	int result = component_update_item(mgr_p, search_id, &tmp);
	if (result == 0)
		printf("[Success] Product with ID %u updated successfully.\n", search_id);
	else
		printf("[Error] Update failed.\n");
}

void component_ui_delete(struct component_manager *mgr_p)
{
	if (!mgr_p || mgr_p->count == 0)
	{
		printf("No components available to delete.\n");
		return;
	}

	char line[64];
	uint32_t delete_id = 0;
	printf("\nEnter Component ID to delete: ");
	if (ui_safe_readline(line, sizeof(line)) != 0 || ui_parse_uint32(line, &delete_id) != 0 || delete_id == 0)
	{
		printf("[Error] Invalid ID.\n");
		return;
	}

	int result = component_delete_item(mgr_p, delete_id);
	if (result == 0)
		printf("[Success] Product with ID %u deleted. Count reduced to %u.\n", delete_id, mgr_p->count);
	else
		printf("[Error] Product not found.\n");
}

void component_ui_show_all(struct component_manager *mgr_p)
{
	if (!mgr_p || mgr_p->count == 0)
	{
		printf("No components available.\n");
		return;
	}
	printf("\n=== ALL COMPONENTS ===\n");
	for (uint32_t i = 0; i < mgr_p->count; i++)
		component_ui_show_single(&mgr_p->components[i]);
}

void component_ui_track_low_stock(struct component_manager *mgr_p)
{
	if (!mgr_p || mgr_p->count == 0)
		return;
	printf("\n=== LOW STOCK WARNING (STOCK <= %u) ===\n", LOW_STOCK_THRESHOLD);
	int found = 0;
	for (uint32_t i = 0; i < mgr_p->count; i++)
	{
		if (mgr_p->components[i].stock <= LOW_STOCK_THRESHOLD)
		{
			component_ui_show_single(&mgr_p->components[i]);
			found = 1;
		}
	}
	if (!found)
		printf("All items are well-stocked.\n");
}

void component_ui_sort_menu(struct component_manager *mgr_p)
{
	char line[64];
	uint32_t choice = 0;
	do
	{
		printf("\n--- SORT COMPONENT MENU ---\n");
		printf("1. Sort by Stock (Descending)\n");
		printf("2. Sort by Warranty Period (Descending)\n");
		printf("0. Return\n");
		printf("Choice: ");

		if (ui_safe_readline(line, sizeof(line)) != 0)
			break;
		if (ui_parse_uint32(line, &choice) != 0)
			choice = 999;

		if (choice == 1)
		{
			component_sort_by_stock_desc(mgr_p);
			component_ui_show_all(mgr_p);
		}
		else if (choice == 2)
		{
			component_sort_by_period_desc(mgr_p);
			component_ui_show_all(mgr_p);
		}
	} while (choice != 0);
}