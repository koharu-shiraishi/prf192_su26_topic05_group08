#include "component_app.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "component_csv.h"
#include "component_manager.h"
#include "component_ui.h"

/*
 * Không lưu khi lần nạp gần nhất thất bại. Kiểm tra lại file còn mở đọc
 * được trước khi lưu; đây không phải khóa chống hai tiến trình cùng sửa.
 */
static enum component_ui_result save_components(
	const struct component_manager *manager, const char *csv_path,
	bool *path_verified, bool *dirty, bool *saved)
{
	FILE *existing;
	enum component_csv_status status;

	*saved = false;
	if (!*path_verified) {
		return component_ui_show_message(
			"Save blocked: fix/create the CSV yourself and load it successfully first.");
	}

	existing = fopen(csv_path, "rb");
	if (existing == NULL) {
		*path_verified = false;
		return component_ui_show_message(
			"Save blocked: the CSV is missing or cannot be opened. Load it again.");
	}
	if (fclose(existing) != 0) {
		*path_verified = false;
		return component_ui_show_message(
			"Save blocked: could not close the existing CSV. Load it again.");
	}

	status = component_csv_save(manager, csv_path);
	if (status == COMPONENT_CSV_OK) {
		*dirty = false;
		*saved = true;
	}
	return component_ui_show_csv_status(status, NULL);
}

static enum component_app_status finish_on_input_error(
	enum component_ui_result result, bool dirty)
{
	if (result == COMPONENT_UI_EOF) {
		if (dirty) {
			fprintf(stderr,
				"Input ended with unsaved changes. Nothing was auto-saved; "
				"changes in memory are lost.\n");
			return COMPONENT_APP_UNSAVED_ON_EOF;
		}
		return COMPONENT_APP_OK;
	}
	if (dirty)
		fprintf(stderr, "I/O error: unsaved changes in memory are lost.\n");
	else
		fprintf(stderr, "Component UI could not continue.\n");
	return result == COMPONENT_UI_INVALID_ARGUMENT ?
		COMPONENT_APP_INVALID_ARGUMENT : COMPONENT_APP_IO_ERROR;
}

static bool is_fatal_ui_result(enum component_ui_result result)
{
	return result == COMPONENT_UI_EOF || result == COMPONENT_UI_IO_ERROR ||
		result == COMPONENT_UI_INVALID_ARGUMENT;
}

enum component_app_status component_app_run(const char *csv_path)
{
	struct component_manager *manager;
	struct component_csv_report report;
	enum component_csv_status csv_status;
	enum component_ui_result ui_status;
	enum component_app_status app_status = COMPONENT_APP_OK;
	bool path_verified = false;
	bool dirty = false;

	if (csv_path == NULL || csv_path[0] == '\0')
		return COMPONENT_APP_INVALID_ARGUMENT;
	manager = malloc(sizeof(*manager));
	if (manager == NULL)
		return COMPONENT_APP_OUT_OF_MEMORY;
	component_manager_init(manager);

	/* File không tồn tại là lỗi, không bị tạo ngầm. */
	csv_status = component_csv_load(manager, csv_path, &report);
	path_verified = csv_status == COMPONENT_CSV_OK;
	ui_status = component_ui_show_csv_status(csv_status, &report);
	if (ui_status != COMPONENT_UI_OK) {
		app_status = finish_on_input_error(ui_status, dirty);
		goto cleanup;
	}
	if (!path_verified && component_ui_show_message(
		"CSV load failed. Existing memory was kept; saving this path is blocked.") !=
	    COMPONENT_UI_OK) {
		app_status = finish_on_input_error(COMPONENT_UI_IO_ERROR, dirty);
		goto cleanup;
	}

	for (;;) {
		enum component_menu_choice choice;

		ui_status = component_ui_read_menu(&choice);
		if (is_fatal_ui_result(ui_status)) {
			app_status = finish_on_input_error(ui_status, dirty);
			break;
		}
		if (ui_status == COMPONENT_UI_CANCELLED)
			continue;

		switch (choice) {
		case COMPONENT_MENU_LOAD: {
			bool confirmed = true;

			if (dirty) {
				ui_status = component_ui_confirm(
					"Load again and discard unsaved memory changes?",
					&confirmed);
				if (ui_status != COMPONENT_UI_OK || !confirmed)
					break;
			}
			csv_status = component_csv_load(manager, csv_path, &report);
			path_verified = csv_status == COMPONENT_CSV_OK;
			if (path_verified)
				dirty = false;
			ui_status = component_ui_show_csv_status(csv_status, &report);
			if (ui_status == COMPONENT_UI_OK && !path_verified)
				ui_status = component_ui_show_message(
					"Load failed; memory was kept and saving is blocked.");
			break;
		}
		case COMPONENT_MENU_SHOW_ALL:
			ui_status = component_ui_show_all(manager);
			break;
		case COMPONENT_MENU_ADD:
			ui_status = component_ui_add(manager);
			break;
		case COMPONENT_MENU_UPDATE:
			ui_status = component_ui_update(manager);
			break;
		case COMPONENT_MENU_DELETE:
			ui_status = component_ui_delete(manager);
			break;
		case COMPONENT_MENU_LOW_STOCK:
			ui_status = component_ui_show_low_stock(
				manager, COMPONENT_DEFAULT_LOW_STOCK_LIMIT);
			break;
		case COMPONENT_MENU_SORT:
			ui_status = component_ui_sort(manager);
			break;
		case COMPONENT_MENU_SAVE: {
			bool saved;

			ui_status = save_components(manager, csv_path, &path_verified,
						    &dirty, &saved);
			break;
		}
		case COMPONENT_MENU_SEARCH:
			ui_status = component_ui_search(manager);
			break;
		case COMPONENT_MENU_EXIT:
			if (!dirty)
				goto cleanup;
			{
				enum component_exit_choice exit_choice;

				ui_status = component_ui_read_exit_choice(&exit_choice);
				if (ui_status != COMPONENT_UI_OK)
					break;
				if (exit_choice == COMPONENT_EXIT_DISCARD)
					goto cleanup;
				if (exit_choice == COMPONENT_EXIT_SAVE) {
					bool saved;

					ui_status = save_components(manager, csv_path,
								    &path_verified,
								    &dirty, &saved);
					if (ui_status == COMPONENT_UI_OK && saved)
						goto cleanup;
				}
			}
			break;
		default:
			ui_status = COMPONENT_UI_INVALID_ARGUMENT;
			break;
		}

		/* Đánh dấu trước mọi lần in tiếp theo sau khi manager đã đổi. */
		if (ui_status == COMPONENT_UI_CHANGED) {
			dirty = true;
			ui_status = component_ui_show_message(
				"Changed in memory. Save to keep these changes.");
		}
		if (is_fatal_ui_result(ui_status)) {
			app_status = finish_on_input_error(ui_status, dirty);
			break;
		}
	}

cleanup:
	free(manager);
	return app_status;
}
