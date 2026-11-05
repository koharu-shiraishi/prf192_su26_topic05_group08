#ifndef COMPONENT_DATA_H
#define COMPONENT_DATA_H

#include <stddef.h>
#include <stdint.h>

/* Kích thước chuỗi bao gồm cả ký tự kết thúc '\0'. */
#define COMPONENT_MAX_COUNT 1000U
#define COMPONENT_NAME_CAPACITY 128U
#define COMPONENT_SOCKET_CAPACITY 32U

/* Giữ giới hạn nghiệp vụ của phiên bản trước. */
#define COMPONENT_MAX_STOCK 1000000U
#define COMPONENT_MAX_WARRANTY_MONTHS 1200U

/* Giá trị DDR được dùng trong mô hình dữ liệu của project. */
#define COMPONENT_DDR_NOT_APPLICABLE 0U
#define COMPONENT_DDR_4 4U
#define COMPONENT_DDR_5 5U

enum component_category {
	COMPONENT_CATEGORY_CPU = 0,
	COMPONENT_CATEGORY_MAINBOARD,
	COMPONENT_CATEGORY_PSU,
	COMPONENT_CATEGORY_RAM,
	COMPONENT_CATEGORY_SSD,
	COMPONENT_CATEGORY_GPU,
	COMPONENT_CATEGORY_CASE,
	/* Giá trị đếm, không phải loại linh kiện để lưu. */
	COMPONENT_CATEGORY_COUNT
};

/*
 * Một linh kiện đã nằm trong manager phải có ID khác 0.
 * Một bản nháp để thêm mới có thể có ID 0; manager sẽ cấp ID.
 * Khởi tạo struct trước khi dùng, ví dụ: struct component draft = {0};
 * Kích thước của name và socket được tính bằng byte, không phải số chữ.
 */
struct component {
	uint32_t id;
	enum component_category category;
	char name[COMPONENT_NAME_CAPACITY];
	char socket[COMPONENT_SOCKET_CAPACITY];
	uint32_t ddr_generation;
	uint32_t warranty_months;
	uint32_t stock;
};

/*
 * Chỉ các hàm quản lý, nạp CSV và sắp xếp được thay đổi mảng này.
 * Các phần khác đọc dữ liệu qua API; không tự sửa count hoặc next_id.
 * next_id == 0 nghĩa là đã hết dải ID tăng dần ở trạng thái hiện tại.
 * Lần khởi tạo/nạp lại tiếp theo có thể tính lại next_id.
 */
struct component_manager {
	struct component items[COMPONENT_MAX_COUNT];
	size_t count;
	uint32_t next_id;
};

#endif /* COMPONENT_DATA_H */
