#include "component_ui.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "component_manager.h"
#include "component_search.h"
#include "component_sort.h"
#include "component_status.h"
#include "component_validation.h"

#define UI_LINE_CAPACITY 1026U

enum input_line_status {
	INPUT_LINE_OK = 0,
	INPUT_LINE_EOF,
	INPUT_LINE_TOO_LONG,
	INPUT_LINE_INVALID,
	INPUT_LINE_IO_ERROR
};

/* Đọc hết một dòng, kể cả khi dòng dài hoặc chứa NUL. */
static enum input_line_status read_line(const char *prompt, char *output,
					size_t capacity)
{
	size_t length = 0;
	bool saw_input = false;
	bool too_long = false;
	bool has_nul = false;
	int character;

	if (fputs(prompt, stdout) == EOF || fflush(stdout) != 0)
		return INPUT_LINE_IO_ERROR;

	for (;;) {
		character = fgetc(stdin);
		if (character == EOF) {
			if (ferror(stdin))
				return INPUT_LINE_IO_ERROR;
			if (!saw_input)
				return INPUT_LINE_EOF;
			break;
		}
		saw_input = true;
		if (character == '\n')
			break;
		if (character == '\0') {
			has_nul = true;
			continue;
		}
		if (length + 1U < capacity)
			output[length++] = (char)(unsigned char)character;
		else
			too_long = true;
	}

	if (length > 0U && output[length - 1U] == '\r')
		length--;
	output[length] = '\0';
	if (has_nul)
		return INPUT_LINE_INVALID;
	return too_long ? INPUT_LINE_TOO_LONG : INPUT_LINE_OK;
}

static bool is_padding(unsigned char character)
{
	return character == ' ' || character == '\t';
}

static unsigned char ascii_lower(unsigned char character)
{
	if (character >= 'A' && character <= 'Z')
		return (unsigned char)(character + ('a' - 'A'));
	return character;
}

static bool equal_command(const char *text, const char *command)
{
	const unsigned char *cursor = (const unsigned char *)text;
	const unsigned char *expected = (const unsigned char *)command;

	while (is_padding(*cursor))
		cursor++;
	while (*cursor != '\0' && *expected != '\0' &&
	       ascii_lower(*cursor) == ascii_lower(*expected)) {
		cursor++;
		expected++;
	}
	if (*expected != '\0')
		return false;
	while (is_padding(*cursor))
		cursor++;
	return *cursor == '\0';
}

enum component_ui_result component_ui_show_message(const char *message)
{
	if (message == NULL)
		return COMPONENT_UI_INVALID_ARGUMENT;
	if (fprintf(stdout, "%s\n", message) < 0 || fflush(stdout) != 0)
		return COMPONENT_UI_IO_ERROR;
	return COMPONENT_UI_OK;
}

static enum component_ui_result read_prompt(const char *prompt, char *output,
					    size_t capacity)
{
	for (;;) {
		enum input_line_status result = read_line(prompt, output, capacity);

		if (result == INPUT_LINE_EOF)
			return COMPONENT_UI_EOF;
		if (result == INPUT_LINE_IO_ERROR)
			return COMPONENT_UI_IO_ERROR;
		if (result == INPUT_LINE_TOO_LONG || result == INPUT_LINE_INVALID) {
			if (component_ui_show_message("Input is invalid or too long.") !=
			    COMPONENT_UI_OK)
				return COMPONENT_UI_IO_ERROR;
			continue;
		}
		if (equal_command(output, ":cancel"))
			return COMPONENT_UI_CANCELLED;
		return COMPONENT_UI_OK;
	}
}

static enum component_ui_result prompt_number(const char *prompt,
					      uint32_t minimum, uint32_t maximum,
					      bool keep_on_empty, uint32_t *value)
{
	char line[UI_LINE_CAPACITY];

	for (;;) {
		uint32_t parsed;
		enum component_ui_result result = read_prompt(prompt, line, sizeof(line));

		if (result != COMPONENT_UI_OK)
			return result;
		if (keep_on_empty && line[0] == '\0')
			return COMPONENT_UI_OK;
		if (component_parse_uint32(line, &parsed) == COMPONENT_STATUS_OK &&
		    parsed >= minimum && parsed <= maximum) {
			*value = parsed;
			return COMPONENT_UI_OK;
		}
		if (component_ui_show_message("Please enter a number in the allowed range.") !=
		    COMPONENT_UI_OK)
			return COMPONENT_UI_IO_ERROR;
	}
}

static bool has_nonblank_name(const char *text)
{
	for (const unsigned char *cursor = (const unsigned char *)text;
	     *cursor != '\0'; cursor++) {
		if (*cursor != ' ' && *cursor != '\t')
			return true;
	}
	return false;
}

static enum component_ui_result prompt_text(const char *prompt, char *output,
					    size_t capacity, bool require_name,
					    bool keep_on_empty, bool allow_clear)
{
	char line[UI_LINE_CAPACITY];

	for (;;) {
		size_t length;
		enum component_ui_result result = read_prompt(prompt, line, sizeof(line));

		if (result != COMPONENT_UI_OK)
			return result;
		if (keep_on_empty && line[0] == '\0')
			return COMPONENT_UI_OK;
		if (allow_clear && equal_command(line, ":clear")) {
			output[0] = '\0';
			return COMPONENT_UI_OK;
		}
		length = strlen(line);
		if (length >= capacity || (require_name && !has_nonblank_name(line))) {
			if (component_ui_show_message("Text is empty or exceeds the field size.") !=
			    COMPONENT_UI_OK)
				return COMPONENT_UI_IO_ERROR;
			continue;
		}
		memcpy(output, line, length + 1U);
		return COMPONENT_UI_OK;
	}
}

static enum component_ui_result prompt_category(enum component_category *category)
{
	uint32_t selected = 0;
	enum component_ui_result result;

	if (component_ui_show_message("Categories:") != COMPONENT_UI_OK)
		return COMPONENT_UI_IO_ERROR;
	for (unsigned int index = 0; index < COMPONENT_CATEGORY_COUNT; index++) {
		const char *name = component_category_name((enum component_category)index);

		if (fprintf(stdout, "  %u. %s\n", index + 1U, name) < 0)
			return COMPONENT_UI_IO_ERROR;
	}
	result = prompt_number("Category: ", 1U, COMPONENT_CATEGORY_COUNT,
			       false, &selected);
	if (result != COMPONENT_UI_OK)
		return result;
	*category = (enum component_category)(selected - 1U);
	return COMPONENT_UI_OK;
}

static enum component_ui_result read_component_fields(struct component *draft,
						       bool updating)
{
	enum component_ui_result result;

	if (!updating) {
		result = prompt_category(&draft->category);
		if (result != COMPONENT_UI_OK)
			return result;
	}

	result = prompt_text(updating ? "Name [Enter keeps]: " : "Name: ",
			     draft->name, sizeof(draft->name), true, updating, false);
	if (result != COMPONENT_UI_OK)
		return result;
	result = prompt_text(updating ? "Socket [Enter keeps, :clear empties]: " :
			     "Socket [Enter for empty]: ",
			     draft->socket, sizeof(draft->socket), false,
			     updating, updating);
	if (result != COMPONENT_UI_OK)
		return result;

	if (draft->category == COMPONENT_CATEGORY_RAM ||
	    draft->category == COMPONENT_CATEGORY_MAINBOARD) {
		result = prompt_number(updating ? "DDR (4/5) [Enter keeps]: " :
				       "DDR (4/5): ",
				       COMPONENT_DDR_4, COMPONENT_DDR_5,
				       updating, &draft->ddr_generation);
		if (result != COMPONENT_UI_OK)
			return result;
	} else {
		draft->ddr_generation = COMPONENT_DDR_NOT_APPLICABLE;
	}

	result = prompt_number(updating ? "Warranty months [Enter keeps]: " :
			       "Warranty months: ", 0U,
			       COMPONENT_MAX_WARRANTY_MONTHS, updating,
			       &draft->warranty_months);
	if (result != COMPONENT_UI_OK)
		return result;
	return prompt_number(updating ? "Stock [Enter keeps]: " : "Stock: ",
			     0U, COMPONENT_MAX_STOCK, updating, &draft->stock);
}

static enum component_ui_result show_component(const struct component *item)
{
	const char *category;

	if (component_validate_fields(item) != COMPONENT_STATUS_OK)
		return COMPONENT_UI_INVALID_ARGUMENT;
	category = component_category_name(item->category);
	if (category == NULL)
		return COMPONENT_UI_INVALID_ARGUMENT;
	if (fprintf(stdout,
		    "ID=%" PRIu32 " | %-9s | %s | socket=%s | DDR=%" PRIu32
		    " | warranty=%" PRIu32 " months | stock=%" PRIu32 "\n",
		    item->id, category, item->name, item->socket,
		    item->ddr_generation, item->warranty_months, item->stock) < 0)
		return COMPONENT_UI_IO_ERROR;
	return COMPONENT_UI_OK;
}

static enum component_ui_result show_manager_error(enum component_status status)
{
	return component_ui_show_message(component_status_message(status));
}

enum component_ui_result component_ui_read_menu(
	enum component_menu_choice *choice)
{
	uint32_t selected = 0;
	enum component_ui_result result;

	if (choice == NULL)
		return COMPONENT_UI_INVALID_ARGUMENT;
	if (component_ui_show_message(
		"\n=== COMPONENT MANAGER ===\n"
		"1. Load CSV          2. Show all\n"
		"3. Add               4. Update\n"
		"5. Delete            6. Low stock\n"
		"7. Sort              8. Save CSV\n"
		"9. Search by name    0. Exit\n"
		"Type :cancel to cancel a prompt.") != COMPONENT_UI_OK)
		return COMPONENT_UI_IO_ERROR;
	result = prompt_number("Choice: ", 0U, 9U, false, &selected);
	if (result != COMPONENT_UI_OK)
		return result;
	*choice = (enum component_menu_choice)selected;
	return COMPONENT_UI_OK;
}

enum component_ui_result component_ui_read_exit_choice(
	enum component_exit_choice *choice)
{
	uint32_t selected = 0;
	enum component_ui_result result;

	if (choice == NULL)
		return COMPONENT_UI_INVALID_ARGUMENT;
	if (component_ui_show_message(
		"Unsaved changes: 1. Save  2. Discard  0. Cancel exit") !=
	    COMPONENT_UI_OK)
		return COMPONENT_UI_IO_ERROR;
	result = prompt_number("Choice: ", 0U, 2U, false, &selected);
	if (result != COMPONENT_UI_OK)
		return result;
	*choice = (enum component_exit_choice)selected;
	return COMPONENT_UI_OK;
}

enum component_ui_result component_ui_confirm(
	const char *question, bool *confirmed)
{
	char line[UI_LINE_CAPACITY];

	if (question == NULL || confirmed == NULL)
		return COMPONENT_UI_INVALID_ARGUMENT;
	for (;;) {
		enum component_ui_result result;

		if (fprintf(stdout, "%s", question) < 0)
			return COMPONENT_UI_IO_ERROR;
		result = read_prompt(" [y/n]: ", line, sizeof(line));
		if (result != COMPONENT_UI_OK)
			return result;
		if (equal_command(line, "y") || equal_command(line, "yes")) {
			*confirmed = true;
			return COMPONENT_UI_OK;
		}
		if (equal_command(line, "n") || equal_command(line, "no")) {
			*confirmed = false;
			return COMPONENT_UI_OK;
		}
		if (component_ui_show_message("Please answer y or n.") != COMPONENT_UI_OK)
			return COMPONENT_UI_IO_ERROR;
	}
}

enum component_ui_result component_ui_show_all(
	const struct component_manager *manager)
{
	enum component_status status = component_manager_validate(manager);

	if (status == COMPONENT_STATUS_INVALID_ARGUMENT)
		return COMPONENT_UI_INVALID_ARGUMENT;
	if (status != COMPONENT_STATUS_OK)
		return show_manager_error(status);
	if (fprintf(stdout, "Components: %zu\n", manager->count) < 0)
		return COMPONENT_UI_IO_ERROR;
	for (size_t index = 0; index < manager->count; index++) {
		if (show_component(&manager->items[index]) != COMPONENT_UI_OK)
			return COMPONENT_UI_IO_ERROR;
	}
	return fflush(stdout) == 0 ? COMPONENT_UI_OK : COMPONENT_UI_IO_ERROR;
}

enum component_ui_result component_ui_show_low_stock(
	const struct component_manager *manager, uint32_t limit)
{
	enum component_status status = component_manager_validate(manager);
	size_t found = 0;

	if (status == COMPONENT_STATUS_INVALID_ARGUMENT)
		return COMPONENT_UI_INVALID_ARGUMENT;
	if (status != COMPONENT_STATUS_OK)
		return show_manager_error(status);
	if (fprintf(stdout, "Low stock (<= %" PRIu32 "):\n", limit) < 0)
		return COMPONENT_UI_IO_ERROR;
	for (size_t index = 0; index < manager->count; index++) {
		if (manager->items[index].stock <= limit) {
			if (show_component(&manager->items[index]) != COMPONENT_UI_OK)
				return COMPONENT_UI_IO_ERROR;
			found++;
		}
	}
	if (found == 0U && fputs("No matching components.\n", stdout) == EOF)
		return COMPONENT_UI_IO_ERROR;
	return fflush(stdout) == 0 ? COMPONENT_UI_OK : COMPONENT_UI_IO_ERROR;
}

enum component_ui_result component_ui_search(
	const struct component_manager *manager)
{
	struct component_search_results matches;
	char keyword[UI_LINE_CAPACITY];
	enum component_ui_result result;
	enum component_status status;

	if (manager == NULL)
		return COMPONENT_UI_INVALID_ARGUMENT;
	result = read_prompt("Search name: ", keyword, sizeof(keyword));
	if (result != COMPONENT_UI_OK)
		return result;
	status = component_search_by_name(manager, keyword, &matches);
	if (status != COMPONENT_STATUS_OK)
		return show_manager_error(status);

	if (fprintf(stdout, "Matches: %zu\n", matches.count) < 0)
		return COMPONENT_UI_IO_ERROR;
	for (size_t index = 0; index < matches.count; index++) {
		struct component item;

		status = component_manager_get_by_id(manager, matches.ids[index], &item);
		if (status != COMPONENT_STATUS_OK)
			return show_manager_error(status);
		if (show_component(&item) != COMPONENT_UI_OK)
			return COMPONENT_UI_IO_ERROR;
	}
	return fflush(stdout) == 0 ? COMPONENT_UI_OK : COMPONENT_UI_IO_ERROR;
}

enum component_ui_result component_ui_add(struct component_manager *manager)
{
	struct component draft = {0};
	enum component_ui_result result;
	enum component_status status;

	if (manager == NULL)
		return COMPONENT_UI_INVALID_ARGUMENT;
	status = component_manager_validate(manager);
	if (status != COMPONENT_STATUS_OK)
		return show_manager_error(status);
	if (manager->count == COMPONENT_MAX_COUNT)
		return show_manager_error(COMPONENT_STATUS_FULL);
	if (manager->next_id == 0U)
		return show_manager_error(COMPONENT_STATUS_ID_EXHAUSTED);

	result = read_component_fields(&draft, false);
	if (result != COMPONENT_UI_OK)
		return result;
	status = component_manager_add(manager, &draft, NULL);
	if (status != COMPONENT_STATUS_OK)
		return show_manager_error(status);
	return COMPONENT_UI_CHANGED;
}

static bool same_component_fields(const struct component *left,
				  const struct component *right)
{
	return left->category == right->category &&
		strcmp(left->name, right->name) == 0 &&
		strcmp(left->socket, right->socket) == 0 &&
		left->ddr_generation == right->ddr_generation &&
		left->warranty_months == right->warranty_months &&
		left->stock == right->stock;
}

enum component_ui_result component_ui_update(struct component_manager *manager)
{
	struct component original;
	struct component draft;
	uint32_t id = 0;
	enum component_ui_result result;
	enum component_status status;

	if (manager == NULL)
		return COMPONENT_UI_INVALID_ARGUMENT;
	status = component_manager_validate(manager);
	if (status != COMPONENT_STATUS_OK)
		return show_manager_error(status);
	result = prompt_number("ID to update: ", 1U, UINT32_MAX, false, &id);
	if (result != COMPONENT_UI_OK)
		return result;
	status = component_manager_get_by_id(manager, id, &original);
	if (status != COMPONENT_STATUS_OK)
		return show_manager_error(status);
	if (show_component(&original) != COMPONENT_UI_OK || fflush(stdout) != 0)
		return COMPONENT_UI_IO_ERROR;
	if (component_ui_show_message("Category stays unchanged during update.") !=
	    COMPONENT_UI_OK)
		return COMPONENT_UI_IO_ERROR;

	draft = original;
	result = read_component_fields(&draft, true);
	if (result != COMPONENT_UI_OK)
		return result;
	if (same_component_fields(&original, &draft))
		return COMPONENT_UI_OK;
	status = component_manager_update(manager, id, &draft);
	if (status != COMPONENT_STATUS_OK)
		return show_manager_error(status);
	return COMPONENT_UI_CHANGED;
}

enum component_ui_result component_ui_delete(struct component_manager *manager)
{
	struct component item;
	uint32_t id = 0;
	bool confirmed = false;
	enum component_ui_result result;
	enum component_status status;

	if (manager == NULL)
		return COMPONENT_UI_INVALID_ARGUMENT;
	status = component_manager_validate(manager);
	if (status != COMPONENT_STATUS_OK)
		return show_manager_error(status);
	result = prompt_number("ID to delete: ", 1U, UINT32_MAX, false, &id);
	if (result != COMPONENT_UI_OK)
		return result;
	status = component_manager_get_by_id(manager, id, &item);
	if (status != COMPONENT_STATUS_OK)
		return show_manager_error(status);
	if (show_component(&item) != COMPONENT_UI_OK || fflush(stdout) != 0)
		return COMPONENT_UI_IO_ERROR;
	result = component_ui_confirm("Delete this component?", &confirmed);
	if (result != COMPONENT_UI_OK)
		return result;
	if (!confirmed)
		return COMPONENT_UI_CANCELLED;
	status = component_manager_delete(manager, id);
	if (status != COMPONENT_STATUS_OK)
		return show_manager_error(status);
	return COMPONENT_UI_CHANGED;
}

enum component_ui_result component_ui_sort(struct component_manager *manager)
{
	uint32_t selected = 0;
	bool changed = false;
	enum component_ui_result result;
	enum component_status status;
	enum component_sort_order order;

	if (manager == NULL)
		return COMPONENT_UI_INVALID_ARGUMENT;
	if (component_ui_show_message(
		"Sort: 1. Name A-Z  2. Stock ascending  3. Stock descending\n"
		"      4. Warranty descending  0. Cancel") != COMPONENT_UI_OK)
		return COMPONENT_UI_IO_ERROR;
	result = prompt_number("Sort choice: ", 0U, 4U, false, &selected);
	if (result != COMPONENT_UI_OK)
		return result;
	if (selected == 0U)
		return COMPONENT_UI_CANCELLED;
	order = (enum component_sort_order)(selected - 1U);
	status = component_sort(manager, order, &changed);
	if (status != COMPONENT_STATUS_OK)
		return show_manager_error(status);
	return changed ? COMPONENT_UI_CHANGED : COMPONENT_UI_OK;
}

enum component_ui_result component_ui_show_csv_status(
	enum component_csv_status status,
	const struct component_csv_report *report)
{
	if (fprintf(stdout, "%s\n", component_csv_status_message(status)) < 0)
		return COMPONENT_UI_IO_ERROR;
	if (report != NULL && report->error_line != 0U &&
	    fprintf(stdout, "Line: %zu\n", report->error_line) < 0)
		return COMPONENT_UI_IO_ERROR;
	if (report != NULL && status == COMPONENT_CSV_INVALID_DATA &&
	    fprintf(stdout, "Reason: %s\n",
		    component_status_message(report->validation_status)) < 0)
		return COMPONENT_UI_IO_ERROR;
	if (report != NULL && status == COMPONENT_CSV_DUPLICATE_ID) {
		if (fprintf(stdout, "Duplicate ID: %" PRIu32 "\n", report->duplicate_id) < 0)
			return COMPONENT_UI_IO_ERROR;
		for (size_t index = 0; index < 2U; index++) {
			if (fprintf(stdout, "  Line %zu: %s\n",
				    report->duplicate_rows[index].line_number,
				    report->duplicate_rows[index].name) < 0)
				return COMPONENT_UI_IO_ERROR;
		}
	}
	return fflush(stdout) == 0 ? COMPONENT_UI_OK : COMPONENT_UI_IO_ERROR;
}
