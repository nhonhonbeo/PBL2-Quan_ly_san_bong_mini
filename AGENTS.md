# BỐI CẢNH DỰ ÁN
Đồ án PBL2 "Quản lý sân bóng mini" bằng C++17. Giao diện ĐỒ HỌA dạng ứng dụng desktop,
dùng Dear ImGui + GLFW + OpenGL3 (đã đặt sẵn trong third_party/). KHÔNG dùng thư viện
nào khác cho giao diện. Phần nghiệp vụ và cấu trúc dữ liệu là code tự cài đặt, không phụ
thuộc ImGui/GLFW.

# CẤU TRÚC THƯ MỤC
QuanLySanBongMini/
├── include/{Core,Account,Field,Booking,Customer,Tournament,Storage}/   # nghiệp vụ
├── include/GUI/   src/GUI/     # toàn bộ giao diện đồ họa nằm ở đây
│   ├── App/ (Application, ScreenManager, Screen)
│   ├── Theme/ (Theme, Fonts, Colors)
│   ├── Widgets/ (Button, Card, Table, Modal, Toast, Calendar, SlotGrid, Form...)
│   └── Screens/ (Auth, Owner/*, Renter/*)
├── third_party/{imgui,glfw}/   assets/fonts/   data/   docs/   build/   Makefile
├── src/main.cpp (bản console cũ, nếu có, giữ nguyên) và src/main_gui.cpp (điểm vào GUI)

# NGHIỆP VỤ
2 loại tài khoản:
1) Chủ sân: đăng nhập/đăng ký; tạo sân (loại 5/7/11, số lượng, quy định, thông tin chủ sân);
   quản lý sân (mã sân, khung giờ, đặt giá, còn chỗ/đã đặt, giờ mở/đóng cửa);
   quản lý khách hàng (danh bạ SĐT, số lần đặt -> ưu đãi, hủy nhiều lần -> danh sách đen,
   xem đánh giá); quản lý giải (tên giải, khách đặt sự kiện, lịch diễn ra).
2) Người thuê: đăng nhập/đăng ký; thông tin cá nhân; đặt/hủy sân; tạo giải (liên hệ chủ sân);
   xem số lần đặt & ưu đãi; đánh giá sân.

# QUY TẮC BẮT BUỘC
1. GUI chỉ GỌI nghiệp vụ qua class/hàm đã khai báo trong include/. KHÔNG viết logic
   nghiệp vụ trong GUI. KHÔNG sửa include/Core. Nghiệp vụ không được #include bất kỳ
   header nào của ImGui/GLFW.
2. Trước khi code, ĐỌC toàn bộ header liên quan trong include/. Hàm nghiệp vụ còn thiếu:
   KHÔNG tự cài logic. Khai báo trong include/GUI/App/Facade.h (lớp trung gian
   "Facade" mà GUI gọi), để phần thân là TODO có comment rõ, liệt kê ở báo cáo cuối task.
   (Giảng viên muốn sinh viên tự viết nghiệp vụ.)
3. Mỗi màn hình là một class kế thừa Screen (đa hình): onEnter(), render(), onExit().
   ScreenManager quản lý stack màn hình. Không viết hàm render khổng lồ,
   tách thành các hàm con theo từng khối giao diện.
4. Mô hình: ImGui là immediate-mode, nên MỖI FRAME không được đọc file hay tính toán nặng.
   Dữ liệu cần hiển thị được nạp vào "ViewModel" khi onEnter() hoặc khi có sự kiện thay đổi,
   render() chỉ vẽ từ ViewModel.
5. Mọi comment bằng tiếng Việt. Mỗi lựa chọn thiết kế/cấu trúc dữ liệu thêm comment
   "// LÝ DO CHỌN: ..." (dùng cho báo cáo).
6. Build bằng `make gui`, chạy được trên Windows (MinGW) và Linux. Code phụ thuộc hệ điều
   hành gói trong #ifdef và đặt ở một chỗ. Không dùng `using namespace std;` trong header.
   Không rò rỉ bộ nhớ.
7. Văn bản hiển thị tiếng Việt có dấu UTF-8. Nạp font hỗ trợ glyph range Vietnamese.
8. Sau mỗi task: chạy make, sửa hết warning (-Wall -Wextra), rồi báo cáo: file tạo/sửa,
   cách test thủ công, danh sách TODO còn lại.