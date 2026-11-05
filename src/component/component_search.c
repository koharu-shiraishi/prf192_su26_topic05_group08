#include "component_search.h"

#include <stdbool.h>
#include <stddef.h>

#include "component_manager.h"

static unsigned char ascii_lower(unsigned char character)
{
	if (character >= 'A' && character <= 'Z')
		return (unsigned char)(character + ('a' - 'A'));
	return character;
}

static bool is_ascii_space(unsigned char character)
{
	return character == ' ' || character == '\t' || character == '\n' ||
		character == '\r' || character == '\f' || character == '\v';
}

static bool contains_search_text(const char *name, const char *keyword)
{
	const unsigned char *text = (const unsigned char *)name;
	const unsigned char *term = (const unsigned char *)keyword;

	for (size_t start = 0; text[start] != '\0'; start++) {
		size_t offset = 0;

		while (term[offset] != '\0' && text[start + offset] != '\0' &&
		       ascii_lower(term[offset]) == ascii_lower(text[start + offset]))
			offset++;
		if (term[offset] == '\0')
			return true;
	}
	return false;
}

enum component_status component_search_by_name(
	const struct component_manager *manager, const char *keyword,
	struct component_search_results *results)
{
	struct component_search_results found = {0};
	const unsigned char *cursor;
	enum component_status status;

	if (manager == NULL || keyword == NULL || results == NULL)
		return COMPONENT_STATUS_INVALID_ARGUMENT;

	cursor = (const unsigned char *)keyword;
	while (is_ascii_space(*cursor))
		cursor++;
	if (*cursor == '\0')
		return COMPONENT_STATUS_INVALID_SEARCH_TEXT;

	status = component_manager_validate(manager);
	if (status != COMPONENT_STATUS_OK)
		return status;

	for (size_t index = 0; index < manager->count; index++) {
		if (contains_search_text(manager->items[index].name, keyword))
			found.ids[found.count++] = manager->items[index].id;
	}
	*results = found;
	return COMPONENT_STATUS_OK;
}
