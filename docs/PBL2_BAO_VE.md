# Tài liệu hỗ trợ bảo vệ PBL2

Tài liệu này mô tả code hiện tại, không phải thiết kế mục tiêu. Những giới hạn đã biết
được giữ lại rõ ràng để phần trình bày không khẳng định chức năng chưa được chứng minh.

## 1. OOP

### Abstraction

- `User` định nghĩa hợp đồng trừu tượng chung cho tài khoản bằng các hàm ảo thuần túy
  `show_menu()` và `get_account_type()`.
- `Booking`, `Tournament`, `Review` mô hình hóa khái niệm nghiệp vụ thay vì để GUI thao
  tác trực tiếp lên từng trường dữ liệu.

### Encapsulation

- `Booking` giữ trạng thái, thông tin sân/khách và danh sách `BookingDetail` ở private;
  các chuyển trạng thái hủy/hoàn thành đi qua method nghiệp vụ.
- `Tournament`, `Review` và các entity khác đóng gói dữ liệu qua getters/setters.
- `LinkedList` sở hữu các node; getter node trả về con trỏ chỉ đọc. Cấu trúc nội bộ
  `head`, `tail`, `count` không công khai.

### Inheritance và polymorphism

- `Owner` và `Account::Customer` kế thừa `User`, vì cả hai là loại tài khoản có thông
  tin chung và hành vi/menu riêng.
- `AccountService` lưu tài khoản dưới dạng `shared_ptr<User>` và gọi hành vi qua kiểu cơ
  sở. `User` có virtual destructor để giải phóng đúng kiểu đối tượng dẫn xuất.
- `Owner` chứa `FacilityConfig` bằng composition; cấu hình cơ sở không được mô hình hóa
  thành một loại tài khoản khác.
- `features/Customer/Customer.h` là header tương thích trỏ tới `Account::Customer`, không
  phải một entity Customer thứ hai.

### Vì sao dùng

Mô hình này dùng kế thừa ở quan hệ “là một” giữa loại tài khoản với `User`; các dữ liệu
phụ như cấu hình cơ sở và chi tiết booking dùng composition để giới hạn trách nhiệm.

## 2. Cấu trúc dữ liệu

| Cấu trúc | Vị trí sử dụng | Độ phức tạp chính | Lý do / đánh đổi |
|---|---|---|---|
| `LinkedList<T>` | Danh sách bản ghi được repository nạp, `BookingDetail`, dữ liệu đầu ra | Thêm đầu/cuối O(1); tìm/xóa theo giá trị O(N); `pop_back` O(N); copy O(N), move O(1) | Cấu trúc tự cài đặt phục vụ mục tiêu PBL2; không cần truy cập ngẫu nhiên. Mỗi node có chi phí cấp phát và locality kém hơn vector. |
| `HashTable<K,V>` | Index customer theo user ID và số điện thoại trong `CustomerRepository` | Tra cứu/thêm/xóa trung bình O(1), xấu nhất O(N); rehash O(N); `get_all()` O(B+N), O(N) bộ nhớ | Separate chaining xử lý va chạm; index tránh dựng lại bảng cho mỗi lần tra cứu sau lần nạp đầu. |
| `Queue<T>` | Thứ tự xử lý các trận trong `TournamentService::buildMatchScheduleQueue` | enqueue/dequeue/front O(1); copy O(N), move O(1) | FIFO phù hợp việc phát lịch trận theo thứ tự; không dùng để thay thế cấu trúc tra cứu. |

Với customer index, lần tra cứu đầu đọc và phân tích F dòng file rồi dựng index N khách:
O(F+N). Khi index đã tải thành công, tra cứu trung bình O(1), xấu nhất O(N) nếu va
chạm hash. Không tính thời gian đọc file vào O(1).

## 3. Kiến trúc

```text
Presentation / Dear ImGui
        ↓
GUI::App::Facade (DTO, ViewModel boundary)
        ↓
Service (business rules and orchestration)
        ↓
Repository (serialization and queries)
        ↓
FileManager / text files
```

- `Application` quản lý tài nguyên GUI và vòng đời ứng dụng; `ScreenManager` giữ/đổi
  screen; mỗi màn hình nằm trong `include/GUI` và `src/GUI`.
- GUI gọi `Facade`; `Facade` chuyển tiếp đến service và ánh xạ dữ liệu sang DTO.
- Các service nhận repository/service phụ thuộc qua tham chiếu constructor, ví dụ
  `BookingService` nhận `BookingRepository`, `CustomerService` và `VenueService`.
- Repository đảm nhiệm đọc/ghi, parse/serialize. `FileManager` tập trung I/O và hỗ trợ
  ghi/thay thế file.
- DI chưa tuyệt đối đồng nhất: `AccountRepository` có dependency `FileManager` nhưng
  triển khai vẫn dùng static helper tương thích cho tài khoản.

## 4. Một số business flow

### Booking

`BookingService` kiểm tra khách hàng tồn tại, thông tin định danh và yêu cầu đặt; `VenueService`/repository
kiểm tra sân, thời gian hoạt động và lịch trùng trước khi lưu. Luồng đặt nhiều ca
theo dõi các ID để rollback toàn bộ booking đã tạo nếu một bước thất bại.

Hủy booking kiểm tra quyền khách hàng và trạng thái trước khi lưu trạng thái booking.
Hoàn thành đi qua kiểm tra quyền Owner và quy tắc trạng thái domain.

### Tournament

`TournamentService` kiểm tra tên/ngày, xung đột với giải khác và booking đang hoạt động,
điều phối yêu cầu/duyệt/từ chối. `Queue` được dùng để tạo lịch ghép cặp cơ bản theo FIFO,
không phải bracket hoặc thuật toán loại trực tiếp.

### Review

`CustomerService::addReview` kiểm tra rating 1–5, nội dung, booking đã hoàn thành và
booking đã được đánh giá hay chưa. Điểm trung bình được tổng hợp từ các review đã lưu.
Tuy vậy, code còn `addOrUpdateReview`, một đường tương thích được Facade sử dụng mà
không lặp lại đầy đủ kiểm tra booking/quyền của `addReview`; không nên xem quy tắc Review
đã được bảo đảm cho mọi luồng GUI trước khi đường gọi này được siết chặt.

## 5. Lưu trữ

- Runtime files nằm dưới `data/`, đường dẫn tương đối theo working directory.
- Bản ghi là text, thường dùng `|` phân cách; repository chịu trách nhiệm chuyển đổi
  entity và định dạng lưu trữ.
- `FileManager` phân biệt file không tồn tại/rỗng/lỗi qua `FileReadStatus` ở API trạng
  thái; ghi file qua file tạm cùng thư mục và thay thế đích.
- Có thư mục `data_backup/` được giữ nguyên như dữ liệu hỗ trợ; không xóa khi dọn build.

## 6. Kiểm thử và cách trình bày demo

Chạy toàn bộ suite:

```sh
make test
```

Windows MinGW:

```powershell
mingw32-make test
```

Các nhóm test bao gồm cấu trúc dữ liệu, booking consistency, customer, file I/O,
domain/service responsibilities, dependency injection, tournament và GUI Facade.
Một số integration tests thao tác trên `data/`; chỉ chạy trên working copy có thể khôi
phục, rồi xác nhận dữ liệu mẫu không bị thay đổi.

Thứ tự demo gợi ý:

1. Build và mở GUI từ thư mục gốc.
2. Đăng nhập bằng tài khoản demo Owner/Customer có sẵn trong `data/accounts.txt`.
3. Cho thấy màn hình tương ứng role, một thao tác đọc dữ liệu thật, rồi một thao tác lưu
   nếu luồng đó có sẵn.
4. Đóng/mở lại ứng dụng và kiểm tra dữ liệu đã lưu.
5. Chạy `make test` để minh họa kiểm thử business và file storage.

Không đưa mật khẩu từ `data/accounts.txt` vào slide hoặc tài liệu: file hiện lưu mật khẩu
plaintext. Chỉ sử dụng tài khoản demo cục bộ; tuyệt đối không thay bằng thông tin thật.

## 7. Rủi ro và phần chưa hoàn thiện

- Mật khẩu plaintext và getter password vẫn tồn tại; đây là rủi ro nghiêm trọng nếu dùng
  ngoài môi trường demo.
- Một số nghiệp vụ có service nhưng chưa có màn hình quản lý riêng; không nên trình bày
  là mọi thao tác đã được nối qua GUI.
- Dữ liệu ngày legacy có cả `dd/MM/yyyy` và ISO ở các luồng khác nhau; chuyển đổi có
  trong một số phép so sánh, chưa phải chuẩn hóa toàn hệ thống.
- Đường gọi `addOrUpdateReview` của Facade chưa đảm bảo toàn bộ quy tắc Review như
  `addReview`.
- `README_part_2.md` giữ lại đề xuất kiến trúc ban đầu; README hiện tại và tài liệu này
  là nguồn mô tả trạng thái code mới hơn.
