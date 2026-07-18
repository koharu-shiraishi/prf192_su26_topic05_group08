# 📑 Hướng Dẫn Sử Dụng Git & Làm Việc Nhóm (Team Git Manual)

Tài liệu này hướng dẫn cách cấu hình, kết nối và quy trình đẩy code lên Repository chung của nhóm, đảm bảo code không bị xung đột và lịch sử Git sạch đẹp.

---

## 🛠️ BƯỚC 1: Cài Đặt & Cấu Hình Ban Đầu (Chỉ làm 1 lần duy nhất)

### 1. Cài đặt Git

- **Windows:** Tải và cài đặt [Git for Windows](https://git-scm.com/). Khi cài xong, bạn sẽ có công cụ **Git Bash** (Terminal giả lập Linux để gõ lệnh).

- **Linux:** Chạy lệnh `sudo apt install git` (Ubuntu/Debian) hoặc `sudo dnf install git` (Fedora).

### 2. Cấu hình danh tính (Git Config)

Mở Git Bash (Windows) hoặc Terminal (Linux) lên và gõ 2 lệnh sau để cấu hình tên và email hiển thị trên GitHub (thay bằng thông tin của bạn):

```bash
git config --global user.name "Tên Thật Của Bạn"
git config --global user.email "email_dang_ki__github_cua_bạn@example.com"
```

## 🚀 BƯỚC 2: Kết Nối Với Repository Lần Đầu Tiên

Do Repo của dự án đang để chế độ Public, bạn có thể dễ dàng lấy code về máy bằng phương pháp sao chép (Clone):

1. Tạo một thư mục trống trên máy tính của bạn (nơi bạn muốn lưu dự án).

2. Click chuột phải vào thư mục đó -> Chọn Git Bash Here (hoặc mở Terminal trên Linux và cd tới thư mục đó).

3. Chạy lệnh clone (thay URL bằng link repo GitHub của nhóm):

 ```bash
 git clone https://github.com/username/ten-repo-cua-nhom.git
 ```

4. Di chuyển vào bên trong thư mục dự án vừa tải về:

```bash
cd ten-repo-cua-nhom
```

## 📊 BƯỚC 3: Quy Trình Làm Việc Hằng Ngày (Luồng Nhánh - Branching)

⚠️ QUY TẮC VÀNG: Tuyệt đối không được phép chỉnh sửa và commit thẳng trên nhánh main. Nhánh main chỉ chứa code đã chạy ổn định 100%. Mỗi khi làm tính năng mới hoặc sửa bug, bắt buộc phải tạo nhánh riêng!

Quy trình 4 bước hằng ngày:

1. Lấy code mới nhất về trước khi làm

Trước khi bắt đầu gõ code, hãy đảm bảo máy của bạn đang có code mới nhất từ mọi người:

```bash
git checkout main
git pull origin main
```

2. Tạo nhánh mới để làm việc

Đặt tên nhánh theo cấu trúc: `loai-nhánh/ten-tinh-nang` (Dùng dấu gạch ngang, viết liền không dấu).

* Nhánh tính năng mới: `feat/`

* Nhánh sửa lỗi: `fix/`

* Nhánh làm tài liệu: `docs/` 

Ví dụ tạo nhánh làm chức năng Menu:

```bash
git checkout -b feat/implement-menu
```

3. Viết code và Lưu lại (Commit)Sau khi viết code xong trên Dev-C++ hoặc VS Code, bạn kiểm tra các file đã sửa:

```bash
git status
```

Nạp các file đã sửa vào vùng chuẩn bị:

```bash
git add .
```

Tiến hành commit với thông điệp chuẩn chỉ (Xem quy định ở Mục 4 phía dưới):

```bash
git commit -m "feat(menu): implement main interactive loop and router"
```

4. Đẩy nhánh lên GitHub để ReviewSau khi commit xong, đẩy nhánh này lên GitHub:

```bash
git push origin feat/implement-menu
```

Lúc này, bạn lên giao diện web của GitHub, bạn sẽ thấy nút Compare & Pull Request. Bấm vào đó để người vận hành Repo (Maintainer) kiểm tra code trước khi gộp vào nhánh `main`.

## 🍳 BƯỚC 4: Quy Chuẩn "Nấu" Commit Đẹp (Conventional Commits)

Để lịch sử dự án chuyên nghiệp, tất cả thành viên KHÔNG viết commit chung chung như: "fix bug", "update", "commit folder". Hãy viết theo công thức sau:

$$\text{type}(\text{scope}): \text{description}$$

1. Các `type` (Loại commit) bắt buộc dùng:

- `feat`: Khi thêm một chức năng mới (ví dụ: thêm hàm tìm kiếm, thêm bộ lọc).

- `fix`: Khi sửa một lỗi logic hoặc lỗi làm sập chương trình.
- `docs`: Khi chỉ sửa file Markdown, file `.txt`, comment giải thích hoặc sơ đồ dòng chảy (Flowchart).

- `refactor`: Khi tối ưu lại cấu trúc code cũ cho sạch hơn nhưng không thay đổi tính năng.

2. Ví dụ thực tế cho dự án:

- `feat(component): add merge sort function for stock sorting`

- `fix(file-io): fix segmentation fault by validating null file pointer` 

- `docs(readme): update repository layout diagram and setup instructions`

## 🎯 BƯỚC 5: Quản Lý Công Việc Bằng GitHub Issues

Khi phát hiện lỗi hoặc phân chia công việc, nhóm sẽ không nhắn tin rời rạc mà sẽ tạo Issue trên GitHub:

- Vào mục Issues trên thanh công cụ của Repo -> Bấm New Issue.

- Đặt tiêu đề rõ ràng. Ví dụ: `[Bug] Hàm load file CSV bị đọc lệch cột`.

- Trong phần mô tả, ghi rõ lỗi xuất hiện ở đâu, hoặc tính năng này cần những hàm nào.

- Tại mục Assignees bên phải, chọn thành viên sẽ chịu trách nhiệm giải quyết việc này.

- Khi bạn làm xong nhánh và tạo Pull Request để sửa lỗi này, chỉ cần ghi thêm chữ `Closes #số_hiệu_issue` trong phần mô tả Pull Request, GitHub sẽ tự động đóng Issue đó khi code được duyệt!
