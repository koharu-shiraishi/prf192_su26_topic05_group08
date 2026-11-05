#ifndef COMPONENT_SORT_H
#define COMPONENT_SORT_H

#include <stdbool.h>

#include "component_data.h"
#include "component_status.h"

enum component_sort_order {
	COMPONENT_SORT_NAME_ASCENDING = 0,
	COMPONENT_SORT_STOCK_ASCENDING,
	COMPONENT_SORT_STOCK_DESCENDING,
	COMPONENT_SORT_WARRANTY_DESCENDING
};

/*
 * Sắp xếp mảng trong RAM bằng insertion sort ổn định.
 * So sánh tên bỏ qua hoa/thường ASCII; byte ngoài ASCII giữ nguyên.
 * Bản ghi bằng khóa giữ thứ tự tương đối ban đầu.
 *
 * changed phải trỏ tới biến riêng ngoài manager: true nếu thứ tự ID
 * thay đổi, false nếu không. Khi lỗi, manager và *changed không thay đổi.
 * Không tự ghi file.
 */
enum component_status component_sort(
	struct component_manager *manager, enum component_sort_order order,
	bool *changed);

#endif /* COMPONENT_SORT_H */
