#ifndef COMPONENT_VALIDATION_H
#define COMPONENT_VALIDATION_H

#include <stdint.h>

#include "component_data.h"
#include "component_status.h"

/*
 * Kiểm tra các trường của một linh kiện, không kiểm tra trường id.
 *
 * - name phải có '\0' trong mảng, không rỗng hoặc chỉ có dấu cách/tab,
 *   và không chứa '\r' hay '\n'.
 * - socket phải có '\0' trong mảng; có thể rỗng, không chứa '\r'/'\n'.
 * - RAM và MAINBOARD: DDR phải là 4 hoặc 5; loại khác: DDR phải là 0.
 * - stock và warranty_months không vượt quá các giới hạn trong data.h.
 *
 * Không tự thay dữ liệu sai. Trả INVALID_ARGUMENT nếu component là NULL.
 */
enum component_status component_validate_fields(
	const struct component *component);

/*
 * Chuyển số thập phân không dấu trong chuỗi thành uint32_t.
 * Cho phép khoảng trắng ASCII ở hai đầu; không chấp nhận dấu +/-,
 * chuỗi rỗng, ký tự thừa hoặc số lớn hơn UINT32_MAX.
 * Khi lỗi, không thay *value. text phải là chuỗi có '\0'.
 */
enum component_status component_parse_uint32(const char *text,
					     uint32_t *value);

/*
 * Tên hợp lệ: CPU, MAINBOARD, PSU, RAM, SSD, GPU, CASE.
 * Bỏ qua khoảng trắng ASCII hai đầu và chữ hoa/thường ASCII khi so sánh.
 * Khi lỗi, không thay *category. text phải là chuỗi có '\0'.
 */
enum component_status component_category_from_string(
	const char *text, enum component_category *category);

/*
 * Trả chuỗi tên cố định, hoặc NULL nếu category không hợp lệ.
 * Người gọi không được sửa hoặc free chuỗi trả về.
 */
const char *component_category_name(enum component_category category);

#endif /* COMPONENT_VALIDATION_H */
