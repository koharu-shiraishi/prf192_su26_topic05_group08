#include "component_sort.h"

#include <stdbool.h>
#include <stddef.h>

#include "component_manager.h"

static unsigned char ascii_lower(unsigned char character)
{
	if (character >= 'A' && character <= 'Z')
		return (unsigned char)(character + ('a' - 'A'));
	return character;
}

static int compare_names(const char *left, const char *right)
{
	const unsigned char *first = (const unsigned char *)left;
	const unsigned char *second = (const unsigned char *)right;

	while (*first != '\0' && *second != '\0') {
		unsigned char a = ascii_lower(*first);
		unsigned char b = ascii_lower(*second);

		if (a < b)
			return -1;
		if (a > b)
			return 1;
		first++;
		second++;
	}
	if (*first == '\0' && *second == '\0')
		return 0;
	return *first == '\0' ? -1 : 1;
}

static int compare_components(const struct component *left,
			      const struct component *right,
			      enum component_sort_order order)
{
	switch (order) {
	case COMPONENT_SORT_NAME_ASCENDING:
		return compare_names(left->name, right->name);
	case COMPONENT_SORT_STOCK_ASCENDING:
		return (left->stock > right->stock) - (left->stock < right->stock);
	case COMPONENT_SORT_STOCK_DESCENDING:
		return (left->stock < right->stock) - (left->stock > right->stock);
	case COMPONENT_SORT_WARRANTY_DESCENDING:
		return (left->warranty_months < right->warranty_months) -
			(left->warranty_months > right->warranty_months);
	}
	return 0;
}

enum component_status component_sort(
	struct component_manager *manager, enum component_sort_order order,
	bool *changed)
{
	enum component_status status;
	bool moved = false;

	if (manager == NULL || changed == NULL)
		return COMPONENT_STATUS_INVALID_ARGUMENT;
	if ((unsigned int)order > COMPONENT_SORT_WARRANTY_DESCENDING)
		return COMPONENT_STATUS_INVALID_SORT_ORDER;
	status = component_manager_validate(manager);
	if (status != COMPONENT_STATUS_OK)
		return status;

	/* Chèn từng phần tử vào đoạn bên trái đã có thứ tự. */
	for (size_t index = 1; index < manager->count; index++) {
		struct component current = manager->items[index];
		size_t position = index;

		while (position > 0U &&
		       compare_components(&manager->items[position - 1U],
					  &current, order) > 0) {
			manager->items[position] = manager->items[position - 1U];
			position--;
			moved = true;
		}
		manager->items[position] = current;
	}
	*changed = moved;
	return COMPONENT_STATUS_OK;
}
