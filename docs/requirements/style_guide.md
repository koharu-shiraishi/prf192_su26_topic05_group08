# Quy ước viết code cho Dự án PRF192 (Topic 5)

## 1. Môi trường và Trình biên dịch

- Hệ điều hành mục tiêu: Unix-like (Môi trường chấm bài của giảng viên).
- Cấm sử dụng các thư viện độc quyền Windows: `<conio.h>`, `getch()`, `system("cls")`, `fflush(stdin)`.
- Thay thế xóa màn hình bằng hàm chung (sẽ định nghĩa trong `common.h`).

## 2. Quy định về Tệp dữ liệu (File CSV)

- Tên file: Viết thường toàn bộ (`components.csv`, `invoices.csv`).
- Dấu phân cách: Bắt buộc dùng dấu phẩy `,`. Không dùng dấu chấm phẩy `;`.
- Cuối dòng: Chú ý cắt bỏ ký tự `\r` khi đọc file từ Windows.

## 3. Quản lý bộ nhớ (Bắt buộc)

- Không dùng mảng tĩnh `[1000]`. Tất cả các mảng quản lý danh sách phải dùng `malloc` khi khởi tạo và `realloc` khi tăng kích thước.
- Mọi vùng nhớ `malloc/realloc` phải được `free()` đầy đủ trước khi thoát chương trình để vượt qua bài test Memory Leak của máy chấm.
