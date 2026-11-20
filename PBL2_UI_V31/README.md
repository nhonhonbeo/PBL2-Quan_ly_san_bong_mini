# PBL2 UI V3.1 — Responsive & Typography

Dành cho mã nguồn đã áp dụng V3. Không áp dụng trên V2 hoặc repo gốc. Đóng chương trình trước khi sửa.

1. Trong thư mục repo, kiểm tra `git status --short`, lưu/commit các thay đổi V3 (không cần commit thư mục patch).
2. Giải nén ZIP vào thư mục repo. Chạy trong CMD:

```cmd
git switch -c refactor/ui-v31
git -c core.whitespace=cr-at-eol apply --check --ignore-whitespace PBL2_UI_V31\v31-from-v3.patch
git -c core.whitespace=cr-at-eol apply --ignore-whitespace PBL2_UI_V31\v31-from-v3.patch
mingw32-make gui
build\QuanLySanBong_GUI.exe
```

Nếu check lỗi: dừng lại, không dùng --reject. Các lệnh chạy riêng từng dòng.

## Nội dung
- Font: dùng các face Segoe UI Bold/Semibold có sẵn trên Windows hoặc face tương ứng của font hệ thống trên Linux. Khi thiếu sẽ dùng font regular. Không cung cấp tệp font. Tiêu đề, giá trị KPI, nút và nhãn form phân cấp rõ hơn.
- Wizard: container 1440px tối đa, card loại sân có minh họa vector, tự đổi số cột, nút lớn hơn và vùng nhập quy định cao hơn.
- Hồ sơ: hai panel cạnh nhau trên vùng rộng từ 1040px, xếp dọc trên vùng nhỏ; số liệu nổi bật, chiều rộng hữu hạn.
- Booking: nhóm thông tin rõ, bộ chọn ngày gọn, lưới tối đa sáu cột, thời gian và giá tách dòng. Giữ nguyên logic chọn ca.
- Auth: hero lớn hơn, typography đậm, pattern trang trí, form tối đa 600px.

## Kiểm tra
- Patch check trên bản V3 sạch: PASS.
- Biên dịch các file CPP đã thay đổi và link chương trình render offscreen: PASS.
- Render 12 màn hình chính + bước 3 booking tại 1024x680, 1320x800, 1920x1080: 39 ảnh. Font Linux có thể khác font Windows.
- Chưa chạy full build MinGW hoặc kiểm thử tương tác tất cả modal/wizard. Wizard màn hình nhỏ vẫn cuộn dọc; footer chưa ghim cố định. Không thêm tính năng nghiệp vụ. DatePicker dùng chung được giới hạn chiều rộng 420px; cần kiểm tra trên Windows.
