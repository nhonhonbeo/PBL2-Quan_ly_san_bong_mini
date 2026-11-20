# PBL2 UI/UX V2 — áp dụng trên mã nguồn gốc

Bộ patch được xây dựng từ đúng bản ZIP `PBL2-Quan_ly_san_bong_mini.zip` đã gửi. Không sửa trực tiếp repository GitHub. Giữ C++17, Dear ImGui, dữ liệu, thuật toán và quy tắc nghiệp vụ (ngoại trừ sửa đọc CRLF ở Phase 1). Không thêm nghiệp vụ mới.

## Cách dùng (Windows PowerShell)

Giải nén bộ patch ngoài thư mục dự án, ví dụ `C:\PBL2_UI_V2`. Mở PowerShell ở gốc dự án. Đảm bảo working tree sạch, lưu/commit công việc đang làm nếu có.

```powershell
git status
git switch -c refactor/ui-v2
$patch = 'C:\PBL2_UI_V2\all-from-original.patch'
git -c core.whitespace=cr-at-eol apply --check $patch
# Chỉ tiếp tục nếu lệnh trên thành công (exit 0).
git -c core.whitespace=cr-at-eol apply $patch
git diff --stat
mingw32-make gui
# Chỉ chạy nếu build thành công.
.\build\QuanLySanBong_GUI.exe
```

Nếu muốn áp dụng từng phần: thay bằng các patch được đánh số 01–17, theo đúng thứ tự. Chạy `git -c core.whitespace=cr-at-eol apply --check <path>` trước mỗi patch, dừng nếu lỗi. Không dùng đồng thời patch tổng hợp và patch riêng. `phase2-only.patch` chỉ dành cho mã nguồn đã áp dụng đầy đủ Phase 1 (01–07). Không dùng `--reject` hoặc cưỡng ép nếu source khác phiên bản.

## Nội dung

| Patch | Thay đổi |
|---|---|
| 01–07 | Sửa bảng, KPI responsive, dữ liệu hard-code, khung giờ, trạng thái booking, CRLF và tinh chỉnh layout |
| 08 | Bảng màu, typography 13/15/18/26/30 px, spacing; giữ font có sẵn |
| 09 | Sidebar 208 px, tiêu đề/TopBar ngắn gọn, avatar vector |
| 10 | Card ít trang trí, badge mềm, button hover, giới hạn chiều rộng mật khẩu, wrap lỗi |
| 11 | Bảng dùng đường phân cách ngang, hàng cao hơn, trạng thái rỗng, ẩn phân trang một trang |
| 12 | KPI 30 px, layout linh hoạt, empty state biểu đồ |
| 13 | Stepper không giả button, bỏ progress dư thừa, thẻ cụm sân minh họa vector, giá và CTA rõ |
| 14 | Rút gọn banner đăng nhập, cân đối chiều cao form |
| 15 | Form đổi mật khẩu gọn, bật điều hướng bàn phím ImGui |
| 16 | Icon sidebar vector, avatar/tên dài, icon mật khẩu, microcopy form giải và đánh giá |
| 17 | Header bảng cố định khi cuộn, thẻ cụm sân hoạt động được ưu tiên, tinh chỉnh chiều cao |

## Kiểm tra đã thực hiện

- `git apply --check` và apply patch tổng hợp trên bản giải nén sạch.
- Kiểm tra và áp dụng tuần tự 17 patch trên bản giải nén sạch khác; đối chiếu thư mục src/include/tests: giống hệt bản tổng hợp.
- Biên dịch lại các file GUI thay đổi ngoại trừ Application.cpp (thiếu GLFW/OpenGL dev headers), liên kết với các object nghiệp vụ và ImGui của bản Phase 1.
- Render trực tiếp 12 màn hình ở 1320x800 và 1024x680 bằng EGL/OpenGL offscreen; ảnh cuối nằm ở `preview/`. Dữ liệu demo từ bản gốc; không tạo số liệu giả.
- Chạy FileManager regression test đã build ở Phase 1: PASS.
- Chưa chạy file EXE Windows, chưa thực hiện full rebuild bằng MinGW, toàn bộ test suite hoặc kiểm thử tương tác mọi màn hình/modal/wizard. Các bảng rộng vẫn có cuộn ngang, các trang dài vẫn cuộn dọc. Các patch có thể có cảnh báo whitespace không ảnh hưởng áp dụng do newline hỗn hợp của bản gốc.

## Kiểm tra thủ công trước khi merge

1. Đăng nhập/đăng ký, thử Tab/Shift+Tab, hiện/ẩn mật khẩu, dữ liệu hợp lệ và không hợp lệ.
2. Đổi giữa các màn hình chủ sân/người thuê; kiểm tra 1320x800 và 1024x680.
3. Thực hiện thêm/sửa/xóa sân, xem lịch, thao tác giải đấu.
4. Thực hiện đặt sân 4 bước, thử ca trùng, ngày quá khứ, xác nhận và hủy theo trạng thái.
5. Kiểm tra đánh giá, hồ sơ, thông báo lỗi, modal xác nhận, các bảng có tên dài và nhiều trang.
6. Chạy `mingw32-make test` trên bản sao dữ liệu demo (test có thể sửa dữ liệu).

## Phạm vi chưa triển khai

Không đổi sang font mới, không tạo ảnh sân thật hoặc dữ liệu seed mới, không gộp tất cả các cột hay chuyển các hành động thành menu ba chấm, chưa thiết kế lại từng modal/form/wizard từ đầu. Ưu tiên giữ khả năng nhận diện và truy cập các chức năng hiện có. Không coi ảnh render là kiểm thử UX tương tác hoặc cam kết đạt một điểm UI/UX nhất định.
