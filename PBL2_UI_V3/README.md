# PBL2 UI/UX V3 — Visual & Layout Refinement

Dành cho bản đã áp dụng UI V2 (17 patch trước đó). Không áp dụng lại V1 hoặc V2. Không thêm chức năng, không thay đổi Service/Repository/Domain/dữ liệu. Bản V3 gồm 7 patch (18–24), hoặc một patch tổng hợp `v3-from-v2.patch`.

## Áp dụng trên Windows CMD

Đóng ứng dụng đang chạy, IDE và trình build. Kiểm tra `git status --short`. Nếu V2 chưa commit, hãy lưu V2 trước (chỉ stage các thư mục mã nguồn, không stage thư mục patch, dữ liệu runtime hay binary).

```cmd
git add src include tests
git commit -m "chore: save UI V2 baseline"
git switch -c refactor/ui-v3
```

Nếu V2 đã commit, bỏ qua hai lệnh đầu. Giải nén thư mục PBL2_UI_V3 tại thư mục repository. Chạy từng lệnh riêng:

```cmd
git -c core.whitespace=cr-at-eol apply --check --ignore-whitespace PBL2_UI_V3\v3-from-v2.patch
git -c core.whitespace=cr-at-eol apply --ignore-whitespace PBL2_UI_V3\v3-from-v2.patch
git diff --stat
mingw32-make gui
build\QuanLySanBong_GUI.exe
```

Chỉ chạy apply nếu --check thành công. Nếu có lỗi quyền ghi, đóng phần mềm giữ file; kiểm tra trạng thái trước khi thử lại. Không chạy git restore trên bản V2 chưa commit. Nếu đã thêm `(void)accentColor;` thủ công hoặc sửa V2 khác bản cung cấp, patch có thể cần điều chỉnh context; gửi kết quả --check, không dùng --reject.

## Nội dung

- 18: font regular 17, small 15, title 21, heading 30, metric 32; tăng tương phản màu chữ/nền.
- 19: sidebar xanh đậm, menu 46px, nút đăng xuất có nền và icon; topbar 84px.
- 20: KPI có accent ngữ nghĩa, giá trị lớn tự giảm font nếu cần, chart rõ hơn; dùng lại accentColor nên hết warning tương ứng.
- 21: card và bố cục đặt sân, nút CTA cùng hàng.
- 22: bảng tận dụng chiều cao, chỉnh form hồ sơ, giải đấu, đánh giá.
- 23: card sân gọn, tên/địa chỉ dài được giới hạn một dòng và có tooltip, căn giữa banner auth, KPI gọn.
- 24: breakpoint lưới 3 cột ở màn hình mặc định và 2 cột ở màn hình nhỏ.

## Kiểm tra

- Áp dụng tổng hợp trên bản V2 sạch, áp dụng tuần tự và so sánh: PASS.
- Biên dịch các translation unit GUI (trừ Application.cpp phụ thuộc GLFW), liên kết harness EGL/OpenGL ngoài cửa sổ: PASS. Calendar.cpp còn cảnh báo snprintf có từ trước.
- Render 12 màn hình ở 1320x800 và 1024x680: PASS. Ảnh trong preview/.
- Chưa full build MinGW, chưa thao tác toàn bộ wizard/modal và chưa render 1920x1080. Scroll dọc vẫn cần khi nội dung vượt chiều cao. Không khẳng định tất cả lỗi bố cục ở môi trường Windows đã được giải quyết.
