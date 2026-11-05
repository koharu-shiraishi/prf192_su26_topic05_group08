#ifndef COMPONENT_APP_H
#define COMPONENT_APP_H

/* Kết quả của module component, không phải lựa chọn trong menu. */
enum component_app_status {
	COMPONENT_APP_OK = 0,
	COMPONENT_APP_INVALID_ARGUMENT,
	COMPONENT_APP_IO_ERROR,
	COMPONENT_APP_OUT_OF_MEMORY,
	COMPONENT_APP_UNSAVED_ON_EOF
};

/*
 * Điểm vào module; main() cung cấp đường dẫn CSV khác rỗng có '\0'.
 * Module tự giữ manager, điều phối UI và trả kết quả cho main().
 * Không gọi exit() và không tự lưu đè khi gặp lỗi nạp hoặc EOF.
 * Nếu nạp lỗi, không cho ghi đè đường dẫn đó tới khi kiểm tra/nạp lại
 * thành công. File không tồn tại là lỗi; không tự tạo file mới.
 *
 * Nếu EOF khi còn thay đổi chưa lưu: cảnh báo, không tự lưu và trả
 * UNSAVED_ON_EOF; các thay đổi trong RAM sẽ mất khi module kết thúc.
 * Khi chọn thoát bình thường và có thay đổi, hỏi lưu/bỏ/hủy thoát.
 */
enum component_app_status component_app_run(const char *csv_path);

#endif /* COMPONENT_APP_H */
