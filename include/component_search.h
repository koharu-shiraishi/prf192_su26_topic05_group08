#ifndef COMPONENT_SEARCH_H
#define COMPONENT_SEARCH_H

#include <stddef.h>
#include <stdint.h>

#include "component_data.h"
#include "component_status.h"

/* Kết quả lưu ID, không giữ con trỏ vào mảng có thể bị xóa/sắp xếp. */
struct component_search_results {
	uint32_t ids[COMPONENT_MAX_COUNT];
	size_t count;
};

/*
 * Tìm chuỗi con trong tên, theo thứ tự hiện tại của manager.
 * Bỏ qua hoa/thường ASCII A-Z; byte ngoài ASCII được so sánh nguyên vẹn.
 * Đây không phải tìm kiếm không phân biệt hoa/thường Unicode.
 *
 * keyword phải kết thúc bằng '\0', không rỗng hoặc chỉ có khoảng trắng
 * ASCII. results phải là vùng nhớ riêng, không nằm trong manager.
 * Không tìm thấy là thành công với results->count = 0.
 * Nếu lỗi, results không thay đổi. Tìm lại sau khi xóa, sort hoặc nạp
 * lại để cập nhật danh sách và thứ tự.
 */
enum component_status component_search_by_name(
	const struct component_manager *manager, const char *keyword,
	struct component_search_results *results);

#endif /* COMPONENT_SEARCH_H */
