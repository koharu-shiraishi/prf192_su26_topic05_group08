#ifndef COMPONENT_MANAGER_H
#define COMPONENT_MANAGER_H

#include <stddef.h>
#include <stdint.h>

#include "component_data.h"
#include "component_status.h"

/*
 * Các con trỏ là bắt buộc, trừ created_id được nói rõ bên dưới.
 * Các thao tác khác init cần một manager đã được khởi tạo hợp lệ.
 *
 * Khởi tạo lại manager rỗng, count = 0 và next_id = 1; dữ liệu cũ
 * trong manager bị bỏ. Không gọi lại tùy tiện khi dữ liệu chưa lưu.
 */
enum component_status component_manager_init(struct component_manager *manager);

/*
 * Kiểm tra count, ID, next_id và các trường của mọi linh kiện.
 * next_id = 0 là trạng thái hết ID tăng dần hợp lệ; nếu khác 0,
 * next_id phải lớn hơn tất cả ID đang có. Không sửa manager.
 */
enum component_status component_manager_validate(
	const struct component_manager *manager);

/*
 * Thêm bản sao của new_component vào cuối mảng.
 * Bỏ qua new_component->id và cấp ID tăng dần; ID 0 không được cấp.
 * created_id có thể là NULL; nếu khác NULL, phải trỏ tới biến riêng
 * ngoài manager và chỉ được ghi ID khi thành công.
 * new_component có thể trỏ vào một phần tử trong manager.
 * Nếu lỗi, manager và biến created_id trỏ tới (nếu có) không thay đổi.
 */
enum component_status component_manager_add(
	struct component_manager *manager,
	const struct component *new_component,
	uint32_t *created_id);

/*
 * Thay dữ liệu của linh kiện có id; ID cũ được giữ nguyên.
 * Bỏ qua replacement->id. replacement có thể trỏ vào manager.
 * Nếu lỗi, manager không thay đổi.
 */
enum component_status component_manager_update(
	struct component_manager *manager, uint32_t id,
	const struct component *replacement);

/* Xóa theo ID và dồn mảng; không giảm hoặc cấp lại next_id. */
enum component_status component_manager_delete(
	struct component_manager *manager, uint32_t id);

/*
 * Hai hàm sau sao chép dữ liệu ra output, không trả con trỏ vào mảng.
 * output phải trỏ tới một struct riêng, không nằm trong manager.
 * Nếu lỗi, output không thay đổi. Index bắt đầu từ 0.
 */
enum component_status component_manager_get_by_id(
	const struct component_manager *manager, uint32_t id,
	struct component *output);

enum component_status component_manager_get_by_index(
	const struct component_manager *manager, size_t index,
	struct component *output);

#endif /* COMPONENT_MANAGER_H */
