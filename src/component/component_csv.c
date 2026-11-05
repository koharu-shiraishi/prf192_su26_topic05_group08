#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "component_csv.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "component_manager.h"
#include "component_validation.h"

#define CSV_FIELD_COUNT 7U

enum line_result {
	LINE_OK = 0,
	LINE_END,
	LINE_TOO_LONG,
	LINE_BAD_FORMAT,
	LINE_READ_ERROR
};

struct temporary_file {
	FILE *stream;
	char *path;
};

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

static int compare_ascii_names(const char *left, const char *right)
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

/* Header cho phép space/tab đệm, kể cả trong một ô được đặt nháy. */
static bool equal_header_field(const char *text, const char *expected)
{
	const unsigned char *cursor = (const unsigned char *)text;
	const unsigned char *wanted = (const unsigned char *)expected;

	while (is_padding(*cursor))
		cursor++;
	while (*cursor != '\0' && *wanted != '\0' &&
	       ascii_lower(*cursor) == ascii_lower(*wanted)) {
		cursor++;
		wanted++;
	}
	if (*wanted != '\0')
		return false;
	while (is_padding(*cursor))
		cursor++;
	return *cursor == '\0';
}

/*
 * Dành thêm một byte cho CR trước LF và một byte cho '\0'.
 * Đọc từng byte để phát hiện NUL và dòng quá dài, không cắt âm thầm.
 */
static enum line_result read_line(FILE *stream,
				  char line[COMPONENT_CSV_MAX_LINE_BYTES + 2U])
{
	size_t length = 0;
	int character;

	for (;;) {
		character = fgetc(stream);
		if (character == EOF) {
			if (ferror(stream))
				return LINE_READ_ERROR;
			if (length == 0U)
				return LINE_END;
			break;
		}
		if (character == '\n')
			break;
		if (character == '\0')
			return LINE_BAD_FORMAT;
		if (length >= COMPONENT_CSV_MAX_LINE_BYTES + 1U)
			return LINE_TOO_LONG;
		line[length++] = (char)(unsigned char)character;
	}

	if (length > 0U && line[length - 1U] == '\r')
		length--;
	if (length > COMPONENT_CSV_MAX_LINE_BYTES)
		return LINE_TOO_LONG;
	if (memchr(line, '\r', length) != NULL)
		return LINE_BAD_FORMAT;
	line[length] = '\0';
	return LINE_OK;
}

static void remove_utf8_bom(char *line)
{
	const unsigned char *bytes = (const unsigned char *)line;

	if (bytes[0] == 0xEFU && bytes[1] == 0xBBU && bytes[2] == 0xBFU)
		memmove(line, line + 3, strlen(line + 3) + 1U);
}

static bool is_blank_line(const char *line)
{
	const unsigned char *cursor = (const unsigned char *)line;

	while (is_padding(*cursor))
		cursor++;
	return *cursor == '\0';
}

static bool is_separator_hint(const char *line)
{
	const unsigned char *cursor = (const unsigned char *)line;

	while (is_padding(*cursor))
		cursor++;
	if (strncmp((const char *)cursor, "sep=,", 5U) != 0)
		return false;
	cursor += 5;
	while (is_padding(*cursor))
		cursor++;
	return *cursor == '\0';
}

/*
 * Trường có nháy giữ nội dung bên trong, kể cả space/tab đầu và cuối.
 * Trường không có nháy được bỏ space/tab ở hai đầu.
 */
static bool split_csv_fields(
	const char *line,
	char fields[CSV_FIELD_COUNT][COMPONENT_CSV_MAX_LINE_BYTES + 1U])
{
	const char *cursor = line;

	for (size_t field = 0; field < CSV_FIELD_COUNT; field++) {
		size_t length = 0;

		while (is_padding((unsigned char)*cursor))
			cursor++;

		if (*cursor == '"') {
			bool closed = false;

			cursor++;
			while (*cursor != '\0') {
				if (*cursor == '"' && cursor[1] == '"') {
					fields[field][length++] = '"';
					cursor += 2;
				} else if (*cursor == '"') {
					cursor++;
					closed = true;
					break;
				} else {
					fields[field][length++] = *cursor++;
				}
			}
			if (!closed)
				return false;
			while (is_padding((unsigned char)*cursor))
				cursor++;
			if (*cursor != ',' && *cursor != '\0')
				return false;
		} else {
			while (*cursor != ',' && *cursor != '\0') {
				if (*cursor == '"')
					return false;
				fields[field][length++] = *cursor++;
			}
			while (length > 0U &&
			       is_padding((unsigned char)fields[field][length - 1U]))
				length--;
		}
		fields[field][length] = '\0';

		if (field + 1U < CSV_FIELD_COUNT) {
			if (*cursor != ',')
				return false;
			cursor++;
		} else if (*cursor != '\0') {
			return false;
		}
	}
	return true;
}

static bool is_csv_header(
	char fields[CSV_FIELD_COUNT][COMPONENT_CSV_MAX_LINE_BYTES + 1U])
{
	static const char *const expected[CSV_FIELD_COUNT] = {
		"id", "category", "name", "socket", "ddr", "warrantyPeriod",
		"stock"
	};

	for (size_t index = 0; index < CSV_FIELD_COUNT; index++) {
		if (!equal_header_field(fields[index], expected[index]))
			return false;
	}
	return true;
}

static enum component_csv_status parse_component(
	char fields[CSV_FIELD_COUNT][COMPONENT_CSV_MAX_LINE_BYTES + 1U],
	struct component *item, enum component_status *validation_status)
{
	enum component_status status;
	size_t length;

	memset(item, 0, sizeof(*item));
	status = component_parse_uint32(fields[0], &item->id);
	if (status != COMPONENT_STATUS_OK || item->id == 0U)
		return COMPONENT_CSV_BAD_ID;

	status = component_category_from_string(fields[1], &item->category);
	if (status != COMPONENT_STATUS_OK) {
		*validation_status = status;
		return COMPONENT_CSV_INVALID_DATA;
	}

	length = strlen(fields[2]);
	if (length >= sizeof(item->name)) {
		*validation_status = COMPONENT_STATUS_INVALID_NAME;
		return COMPONENT_CSV_INVALID_DATA;
	}
	memcpy(item->name, fields[2], length + 1U);

	length = strlen(fields[3]);
	if (length >= sizeof(item->socket)) {
		*validation_status = COMPONENT_STATUS_INVALID_SOCKET;
		return COMPONENT_CSV_INVALID_DATA;
	}
	memcpy(item->socket, fields[3], length + 1U);

	status = component_parse_uint32(fields[4], &item->ddr_generation);
	if (status == COMPONENT_STATUS_OK)
		status = component_parse_uint32(fields[5], &item->warranty_months);
	if (status == COMPONENT_STATUS_OK)
		status = component_parse_uint32(fields[6], &item->stock);
	if (status != COMPONENT_STATUS_OK) {
		*validation_status = status;
		return COMPONENT_CSV_INVALID_DATA;
	}

	status = component_validate_fields(item);
	if (status != COMPONENT_STATUS_OK) {
		*validation_status = status;
		return COMPONENT_CSV_INVALID_DATA;
	}
	return COMPONENT_CSV_OK;
}

static void fill_duplicate_report(struct component_csv_report *report,
				  const struct component *previous,
				  size_t previous_line,
				  const struct component *current,
				  size_t current_line)
{
	const struct component *first = previous;
	const struct component *second = current;
	size_t first_line = previous_line;
	size_t second_line = current_line;

	if (compare_ascii_names(previous->name, current->name) > 0) {
		first = current;
		second = previous;
		first_line = current_line;
		second_line = previous_line;
	}

	report->duplicate_id = current->id;
	report->duplicate_rows[0].line_number = first_line;
	report->duplicate_rows[1].line_number = second_line;
	memcpy(report->duplicate_rows[0].name, first->name, sizeof(first->name));
	memcpy(report->duplicate_rows[1].name, second->name, sizeof(second->name));
}

enum component_csv_status component_csv_load(
	struct component_manager *manager, const char *file_path,
	struct component_csv_report *report)
{
	struct component_manager *candidate;
	size_t source_lines[COMPONENT_MAX_COUNT];
	char line[COMPONENT_CSV_MAX_LINE_BYTES + 2U];
	char fields[CSV_FIELD_COUNT][COMPONENT_CSV_MAX_LINE_BYTES + 1U];
	size_t line_number = 0;
	uint32_t highest_id = 0;
	bool saw_separator = false;
	bool saw_header_or_data = false;
	enum component_csv_status result = COMPONENT_CSV_OK;
	FILE *stream;

	if (manager == NULL || file_path == NULL || file_path[0] == '\0' ||
	    report == NULL)
		return COMPONENT_CSV_INVALID_ARGUMENT;
	memset(report, 0, sizeof(*report));

	stream = fopen(file_path, "rb");
	if (stream == NULL)
		return COMPONENT_CSV_OPEN_ERROR;

	candidate = malloc(sizeof(*candidate));
	if (candidate == NULL) {
		fclose(stream);
		return COMPONENT_CSV_OUT_OF_MEMORY;
	}
	component_manager_init(candidate);

	for (;;) {
		enum line_result line_status = read_line(stream, line);
		struct component item;

		if (line_status == LINE_END)
			break;
		line_number++;
		if (line_status != LINE_OK) {
			report->error_line = line_number;
			if (line_status == LINE_TOO_LONG)
				result = COMPONENT_CSV_LINE_TOO_LONG;
			else if (line_status == LINE_BAD_FORMAT)
				result = COMPONENT_CSV_BAD_FORMAT;
			else
				result = COMPONENT_CSV_READ_ERROR;
			break;
		}

		if (line_number == 1U)
			remove_utf8_bom(line);
		if (is_blank_line(line))
			continue;
		if (!saw_header_or_data && !saw_separator &&
		    is_separator_hint(line)) {
			saw_separator = true;
			continue;
		}

		if (!split_csv_fields(line, fields)) {
			report->error_line = line_number;
			result = COMPONENT_CSV_BAD_FORMAT;
			break;
		}
		if (!saw_header_or_data && equal_header_field(fields[0], "id")) {
			if (!is_csv_header(fields)) {
				report->error_line = line_number;
				result = COMPONENT_CSV_BAD_FORMAT;
				break;
			}
			saw_header_or_data = true;
			continue;
		}
		saw_header_or_data = true;

		if (candidate->count == COMPONENT_MAX_COUNT) {
			report->error_line = line_number;
			result = COMPONENT_CSV_TOO_MANY_RECORDS;
			break;
		}
		result = parse_component(fields, &item, &report->validation_status);
		if (result != COMPONENT_CSV_OK) {
			report->error_line = line_number;
			break;
		}

		for (size_t index = 0; index < candidate->count; index++) {
			if (candidate->items[index].id == item.id) {
				fill_duplicate_report(report, &candidate->items[index],
						      source_lines[index], &item,
						      line_number);
				report->error_line = line_number;
				result = COMPONENT_CSV_DUPLICATE_ID;
				break;
			}
		}
		if (result != COMPONENT_CSV_OK)
			break;

		candidate->items[candidate->count] = item;
		source_lines[candidate->count] = line_number;
		candidate->count++;
		report->records_read++;
		if (item.id > highest_id)
			highest_id = item.id;
	}

	if (fclose(stream) != 0 && result == COMPONENT_CSV_OK)
		result = COMPONENT_CSV_CLOSE_ERROR;
	if (result == COMPONENT_CSV_OK) {
		candidate->next_id = highest_id == UINT32_MAX ? 0U : highest_id + 1U;
		*manager = *candidate;
	}
	free(candidate);
	return result;
}

static void discard_temporary(struct temporary_file *temporary)
{
	if (temporary->stream != NULL) {
		fclose(temporary->stream);
		temporary->stream = NULL;
	}
	if (temporary->path != NULL) {
		remove(temporary->path);
		free(temporary->path);
		temporary->path = NULL;
	}
}

#ifndef _WIN32
static enum component_csv_status open_temporary_file(
	const char *target, struct temporary_file *temporary)
{
	static const char suffix[] = ".tmp.XXXXXX";
	size_t length = strlen(target);
	int descriptor;

	if (length > SIZE_MAX - sizeof(suffix))
		return COMPONENT_CSV_OUT_OF_MEMORY;
	temporary->path = malloc(length + sizeof(suffix));
	if (temporary->path == NULL)
		return COMPONENT_CSV_OUT_OF_MEMORY;
	memcpy(temporary->path, target, length);
	memcpy(temporary->path + length, suffix, sizeof(suffix));

	descriptor = mkstemp(temporary->path);
	if (descriptor < 0) {
		free(temporary->path);
		temporary->path = NULL;
		return COMPONENT_CSV_OPEN_ERROR;
	}
	temporary->stream = fdopen(descriptor, "wb");
	if (temporary->stream == NULL) {
		close(descriptor);
		discard_temporary(temporary);
		return COMPONENT_CSV_OPEN_ERROR;
	}
	return COMPONENT_CSV_OK;
}

static bool replace_file(const char *temporary, const char *target)
{
	return rename(temporary, target) == 0;
}
#else
static enum component_csv_status open_temporary_file(
	const char *target, struct temporary_file *temporary)
{
	size_t length = strlen(target);
	size_t capacity;

	if (length > SIZE_MAX - 32U)
		return COMPONENT_CSV_OUT_OF_MEMORY;
	capacity = length + 32U;
	temporary->path = malloc(capacity);
	if (temporary->path == NULL)
		return COMPONENT_CSV_OUT_OF_MEMORY;

	for (unsigned int attempt = 0; attempt < 10000U; attempt++) {
		HANDLE handle;
		int descriptor;

		snprintf(temporary->path, capacity, "%s.tmp.%u", target, attempt);
		handle = CreateFileA(temporary->path, GENERIC_WRITE, 0, NULL,
				     CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (handle == INVALID_HANDLE_VALUE) {
			DWORD error = GetLastError();

			if (error == ERROR_FILE_EXISTS || error == ERROR_ALREADY_EXISTS)
				continue;
			free(temporary->path);
			temporary->path = NULL;
			return COMPONENT_CSV_OPEN_ERROR;
		}

		descriptor = _open_osfhandle((intptr_t)handle, _O_WRONLY | _O_BINARY);
		if (descriptor < 0) {
			CloseHandle(handle);
			discard_temporary(temporary);
			return COMPONENT_CSV_OPEN_ERROR;
		}
		temporary->stream = _fdopen(descriptor, "wb");
		if (temporary->stream == NULL) {
			_close(descriptor);
			discard_temporary(temporary);
			return COMPONENT_CSV_OPEN_ERROR;
		}
		return COMPONENT_CSV_OK;
	}

	free(temporary->path);
	temporary->path = NULL;
	return COMPONENT_CSV_OPEN_ERROR;
}

static bool replace_file(const char *temporary, const char *target)
{
	return MoveFileExA(temporary, target, MOVEFILE_REPLACE_EXISTING) != 0;
}
#endif

static bool write_quoted(FILE *stream, const char *text)
{
	if (fputc('"', stream) == EOF)
		return false;
	for (const char *cursor = text; *cursor != '\0'; cursor++) {
		if (*cursor == '"' && fputc('"', stream) == EOF)
			return false;
		if (fputc((unsigned char)*cursor, stream) == EOF)
			return false;
	}
	return fputc('"', stream) != EOF;
}

static bool write_components(FILE *stream,
			     const struct component_manager *manager)
{
	if (fputs("id,category,name,socket,ddr,warrantyPeriod,stock\n", stream) == EOF)
		return false;

	for (size_t index = 0; index < manager->count; index++) {
		const struct component *item = &manager->items[index];
		const char *category = component_category_name(item->category);

		if (category == NULL ||
		    fprintf(stream, "%" PRIu32 ",%s,", item->id, category) < 0 ||
		    !write_quoted(stream, item->name) || fputc(',', stream) == EOF ||
		    !write_quoted(stream, item->socket) ||
		    fprintf(stream, ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 "\n",
			    item->ddr_generation, item->warranty_months,
			    item->stock) < 0)
			return false;
	}
	return true;
}

enum component_csv_status component_csv_save(
	const struct component_manager *manager, const char *file_path)
{
	struct temporary_file temporary = {0};
	enum component_csv_status status;

	if (manager == NULL || file_path == NULL || file_path[0] == '\0')
		return COMPONENT_CSV_INVALID_ARGUMENT;
	if (component_manager_validate(manager) != COMPONENT_STATUS_OK)
		return COMPONENT_CSV_INVALID_DATA;

	status = open_temporary_file(file_path, &temporary);
	if (status != COMPONENT_CSV_OK)
		return status;

	if (!write_components(temporary.stream, manager) ||
	    fflush(temporary.stream) != 0)
		status = COMPONENT_CSV_WRITE_ERROR;
	if (fclose(temporary.stream) != 0 && status == COMPONENT_CSV_OK)
		status = COMPONENT_CSV_CLOSE_ERROR;
	temporary.stream = NULL;

	if (status == COMPONENT_CSV_OK &&
	    !replace_file(temporary.path, file_path))
		status = COMPONENT_CSV_REPLACE_ERROR;
	if (status != COMPONENT_CSV_OK)
		discard_temporary(&temporary);
	else
		free(temporary.path);
	return status;
}

const char *component_csv_status_message(enum component_csv_status status)
{
	switch (status) {
	case COMPONENT_CSV_OK:
		return "CSV operation completed";
	case COMPONENT_CSV_INVALID_ARGUMENT:
		return "Invalid CSV argument";
	case COMPONENT_CSV_OPEN_ERROR:
		return "Could not open the file";
	case COMPONENT_CSV_READ_ERROR:
		return "Could not read the file";
	case COMPONENT_CSV_WRITE_ERROR:
		return "Could not write the temporary file";
	case COMPONENT_CSV_CLOSE_ERROR:
		return "Could not close the file";
	case COMPONENT_CSV_REPLACE_ERROR:
		return "Could not replace the target file";
	case COMPONENT_CSV_LINE_TOO_LONG:
		return "CSV line is too long";
	case COMPONENT_CSV_BAD_FORMAT:
		return "Invalid CSV format";
	case COMPONENT_CSV_BAD_ID:
		return "Invalid component ID in CSV";
	case COMPONENT_CSV_DUPLICATE_ID:
		return "Duplicate component ID in CSV";
	case COMPONENT_CSV_INVALID_DATA:
		return "Invalid component data in CSV or manager";
	case COMPONENT_CSV_TOO_MANY_RECORDS:
		return "CSV has too many component records";
	case COMPONENT_CSV_OUT_OF_MEMORY:
		return "Not enough memory for the CSV operation";
	}
	return "Unknown CSV status";
}
