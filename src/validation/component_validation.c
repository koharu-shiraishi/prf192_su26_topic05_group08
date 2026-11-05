#include "component_validation.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const char *const category_names[COMPONENT_CATEGORY_COUNT] = {
	"CPU", "MAINBOARD", "PSU", "RAM", "SSD", "GPU", "CASE"
};

static bool is_ascii_space(unsigned char character)
{
	return character == ' ' || character == '\t' || character == '\n' ||
		character == '\r' || character == '\f' || character == '\v';
}

static unsigned char ascii_lower(unsigned char character)
{
	if (character >= 'A' && character <= 'Z')
		return (unsigned char)(character + ('a' - 'A'));
	return character;
}

/* Kiểm tra ký tự kết thúc trước khi dùng các hàm chuỗi thông thường. */
static bool valid_text_field(const char *text, size_t capacity,
			     bool require_nonblank)
{
	const char *end = memchr(text, '\0', capacity);
	bool has_visible_character = false;

	if (end == NULL)
		return false;

	for (const char *cursor = text; cursor < end; cursor++) {
		if (*cursor == '\r' || *cursor == '\n')
			return false;
		if (*cursor != ' ' && *cursor != '\t')
			has_visible_character = true;
	}

	return !require_nonblank || has_visible_character;
}

enum component_status component_validate_fields(
	const struct component *component)
{
	if (component == NULL)
		return COMPONENT_STATUS_INVALID_ARGUMENT;

	if ((unsigned int)component->category >= COMPONENT_CATEGORY_COUNT)
		return COMPONENT_STATUS_INVALID_CATEGORY;
	if (!valid_text_field(component->name, sizeof(component->name), true))
		return COMPONENT_STATUS_INVALID_NAME;
	if (!valid_text_field(component->socket, sizeof(component->socket), false))
		return COMPONENT_STATUS_INVALID_SOCKET;

	if (component->category == COMPONENT_CATEGORY_RAM ||
	    component->category == COMPONENT_CATEGORY_MAINBOARD) {
		if (component->ddr_generation != COMPONENT_DDR_4 &&
		    component->ddr_generation != COMPONENT_DDR_5)
			return COMPONENT_STATUS_INVALID_DDR;
	} else if (component->ddr_generation != COMPONENT_DDR_NOT_APPLICABLE) {
		return COMPONENT_STATUS_INVALID_DDR;
	}

	if (component->stock > COMPONENT_MAX_STOCK)
		return COMPONENT_STATUS_INVALID_STOCK;
	if (component->warranty_months > COMPONENT_MAX_WARRANTY_MONTHS)
		return COMPONENT_STATUS_INVALID_WARRANTY;

	return COMPONENT_STATUS_OK;
}

enum component_status component_parse_uint32(const char *text,
					     uint32_t *value)
{
	const unsigned char *cursor;
	uint32_t number = 0;
	bool has_digit = false;

	if (text == NULL || value == NULL)
		return COMPONENT_STATUS_INVALID_ARGUMENT;

	cursor = (const unsigned char *)text;
	while (is_ascii_space(*cursor))
		cursor++;

	while (*cursor >= '0' && *cursor <= '9') {
		uint32_t digit = (uint32_t)(*cursor - '0');

		has_digit = true;
		if (number > (UINT32_MAX - digit) / 10U)
			return COMPONENT_STATUS_INVALID_NUMBER;
		number = number * 10U + digit;
		cursor++;
	}

	if (!has_digit)
		return COMPONENT_STATUS_INVALID_NUMBER;
	while (is_ascii_space(*cursor))
		cursor++;
	if (*cursor != '\0')
		return COMPONENT_STATUS_INVALID_NUMBER;

	*value = number;
	return COMPONENT_STATUS_OK;
}

enum component_status component_category_from_string(
	const char *text, enum component_category *category)
{
	const unsigned char *start;

	if (text == NULL || category == NULL)
		return COMPONENT_STATUS_INVALID_ARGUMENT;

	start = (const unsigned char *)text;
	while (is_ascii_space(*start))
		start++;

	for (unsigned int index = 0; index < COMPONENT_CATEGORY_COUNT; index++) {
		const unsigned char *input = start;
		const unsigned char *expected =
			(const unsigned char *)category_names[index];

		while (*input != '\0' && *expected != '\0' &&
		       ascii_lower(*input) == ascii_lower(*expected)) {
			input++;
			expected++;
		}
		if (*expected != '\0')
			continue;
		while (is_ascii_space(*input))
			input++;
		if (*input == '\0') {
			*category = (enum component_category)index;
			return COMPONENT_STATUS_OK;
		}
	}

	return COMPONENT_STATUS_INVALID_CATEGORY;
}

const char *component_category_name(enum component_category category)
{
	if ((unsigned int)category >= COMPONENT_CATEGORY_COUNT)
		return NULL;
	return category_names[(unsigned int)category];
}
