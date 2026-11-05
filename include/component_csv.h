#ifndef COMPONENT_CSV_H
#define COMPONENT_CSV_H

#include <stddef.h>
#include <stdint.h>

#include "component_data.h"
#include "component_status.h"

/* Số byte tối đa của một dòng vật lý, không tính CR/LF cuối dòng. */
#define COMPONENT_CSV_MAX_LINE_BYTES 1024U

enum component_csv_status {
	COMPONENT_CSV_OK = 0,
	COMPONENT_CSV_INVALID_ARGUMENT,
	COMPONENT_CSV_OPEN_ERROR,
	COMPONENT_CSV_READ_ERROR,
	COMPONENT_CSV_WRITE_ERROR,
	COMPONENT_CSV_CLOSE_ERROR,
	COMPONENT_CSV_REPLACE_ERROR,
	COMPONENT_CSV_LINE_TOO_LONG,
	COMPONENT_CSV_BAD_FORMAT,
	COMPONENT_CSV_BAD_ID,
	COMPONENT_CSV_DUPLICATE_ID,
	COMPONENT_CSV_INVALID_DATA,
	COMPONENT_CSV_TOO_MANY_RECORDS,
	COMPONENT_CSV_OUT_OF_MEMORY
};

/* Thông tin một dòng trong cặp ID trùng, chỉ dùng khi nạp CSV. */
struct component_csv_duplicate_row {
	size_t line_number;
	char name[COMPONENT_NAME_CAPACITY];
};

/*
 * records_read: số dòng dữ liệu hợp lệ đã phân tích trước lỗi; nếu lỗi,
 * các dòng này chưa được đưa vào manager. Dòng vật lý đánh số từ 1.
 * error_line = 0 nếu lỗi không gắn với một dòng.
 * validation_status chỉ khác OK khi kết quả là CSV_INVALID_DATA.
 * Khi CSV_DUPLICATE_ID, duplicate_id và hai duplicate_rows có giá trị.
 */
struct component_csv_report {
	size_t records_read;
	size_t error_line;
	enum component_status validation_status;
	uint32_t duplicate_id;
	struct component_csv_duplicate_row duplicate_rows[2];
};

/*
 * Nạp CSV vào vùng tạm rồi mới thay manager khi toàn bộ dữ liệu hợp lệ.
 * manager phải đã được khởi tạo. Cả ba con trỏ là bắt buộc; file_path
 * là chuỗi không rỗng có '\0'.
 * report là struct riêng, không nằm trong manager; report được đặt lại
 * tại đầu mỗi lần gọi hợp lệ.
 * Nếu lỗi: manager và file nguồn không thay đổi.
 *
 * Gặp ID trùng: dừng và báo một cặp dòng trùng; không tự đổi ID.
 * Hai tên trong cặp được sắp tăng dần, bỏ qua hoa/thường ASCII; nếu tên
 * bằng nhau thì dòng xuất hiện trước đứng trước. Với nhiều cặp, sửa rồi
 * nạp lại để xem lỗi tiếp theo.
 *
 * Thứ tự cột: id,category,name,socket,ddr,warrantyPeriod,stock.
 * Hỗ trợ header, dòng sep=, tùy chọn, BOM UTF-8, CRLF/LF, khoảng trắng
 * đệm và trường có dấu nháy kép theo quy tắc CSV; không hỗ trợ xuống
 * dòng bên trong trường. Chi tiết quy tắc có trong tài liệu kèm theo.
 */
enum component_csv_status component_csv_load(
	struct component_manager *manager, const char *file_path,
	struct component_csv_report *report);

/*
 * Chỉ lưu manager hợp lệ. Tạo file tạm riêng trong cùng thư mục, không
 * ghi đè file tạm đã tồn tại; kiểm tra lỗi ghi/đóng rồi mới thử thay
 * file đích. Không xóa file đích trước khi thay.
 * Trước bước thay file, nếu lỗi thì file đích không đổi.
 * Không cam kết atomic hay bền vững trên mọi hệ điều hành/filesystem.
 * Không tự động đồng bộ hai tiến trình cùng ghi một file.
 */
enum component_csv_status component_csv_save(
	const struct component_manager *manager, const char *file_path);

/*
 * Chuỗi trả về thuộc chương trình; người gọi không được sửa hoặc free.
 * Nếu status không hợp lệ, trả thông báo lỗi không xác định, không NULL.
 */
const char *component_csv_status_message(enum component_csv_status status);

#endif /* COMPONENT_CSV_H */
