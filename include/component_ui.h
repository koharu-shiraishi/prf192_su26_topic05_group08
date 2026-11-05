#ifndef COMPONENT_UI_H
#define COMPONENT_UI_H

#include <stdbool.h>
#include <stdint.h>

#include "component_csv.h"
#include "component_data.h"

#define COMPONENT_DEFAULT_LOW_STOCK_LIMIT 5U

enum component_menu_choice {
	COMPONENT_MENU_EXIT = 0,
	COMPONENT_MENU_LOAD = 1,
	COMPONENT_MENU_SHOW_ALL = 2,
	COMPONENT_MENU_ADD = 3,
	COMPONENT_MENU_UPDATE = 4,
	COMPONENT_MENU_DELETE = 5,
	COMPONENT_MENU_LOW_STOCK = 6,
	COMPONENT_MENU_SORT = 7,
	COMPONENT_MENU_SAVE = 8,
	COMPONENT_MENU_SEARCH = 9
};

enum component_exit_choice {
	COMPONENT_EXIT_CANCEL = 0,
	COMPONENT_EXIT_SAVE = 1,
	COMPONENT_EXIT_DISCARD = 2
};

/* CHANGED nghĩa là manager đã thay đổi và cần được đánh dấu chưa lưu. */
enum component_ui_result {
	COMPONENT_UI_OK = 0,
	COMPONENT_UI_CHANGED,
	COMPONENT_UI_CANCELLED,
	COMPONENT_UI_EOF,
	COMPONENT_UI_IO_ERROR,
	COMPONENT_UI_INVALID_ARGUMENT
};

/*
 * UI đọc một dòng tại một thời điểm; EOF và lỗi I/O phải được trả về,
 * không được tự tạo dữ liệu mặc định hoặc lặp vô hạn.
 * Với các hàm đọc, tham số output không đổi nếu kết quả không phải OK.
 */
enum component_ui_result component_ui_read_menu(
	enum component_menu_choice *choice);

enum component_ui_result component_ui_read_exit_choice(
	enum component_exit_choice *choice);

/* Nếu trả OK: confirmed là true cho có, false cho không. */
enum component_ui_result component_ui_confirm(
	const char *question, bool *confirmed);

/* Các hàm chỉ đọc sau không sửa manager và không ghi CSV. */
enum component_ui_result component_ui_show_all(
	const struct component_manager *manager);

enum component_ui_result component_ui_show_low_stock(
	const struct component_manager *manager, uint32_t limit);

enum component_ui_result component_ui_search(
	const struct component_manager *manager);

/*
 * Hỏi dữ liệu và gọi manager/sort; UI không tự sửa các trường manager.
 * CHANGED khi dữ liệu hoặc thứ tự thực sự thay đổi; OK khi không đổi.
 * EOF/cancel/lỗi trước khi hoàn thành không được cập nhật dở dang.
 */
enum component_ui_result component_ui_add(
	struct component_manager *manager);

enum component_ui_result component_ui_update(
	struct component_manager *manager);

enum component_ui_result component_ui_delete(
	struct component_manager *manager);

enum component_ui_result component_ui_sort(
	struct component_manager *manager);

/* Hiển thị một thông báo kết thúc bằng '\0', không sửa dữ liệu. */
enum component_ui_result component_ui_show_message(const char *message);

/* report có thể NULL khi báo lỗi lưu; khi báo lỗi nạp nên truyền report. */
enum component_ui_result component_ui_show_csv_status(
	enum component_csv_status status,
	const struct component_csv_report *report);

#endif /* COMPONENT_UI_H */
