# Quản lý sân bóng mini (PBL2)

Ứng dụng desktop C++17 hỗ trợ chủ sân quản lý sân, lịch đặt và khách hàng; người thuê
có thể tìm sân, gửi yêu cầu đặt sân và theo dõi các nghiệp vụ liên quan. Dữ liệu được
lưu cục bộ trong các tệp văn bản dưới `data/`.

## Mục tiêu

- Áp dụng OOP và các cấu trúc dữ liệu tự cài đặt trong một ứng dụng nghiệp vụ.
- Tách giao diện khỏi quy tắc nghiệp vụ và thao tác lưu trữ.
- Thực hành kiểm thử logic, lưu trữ tệp và các luồng cần đồng bộ nhiều bản ghi.

## Chức năng hiện có

- Tài khoản Owner/Customer: đăng nhập, đăng ký và cập nhật hồ sơ.
- Sân: cấu hình sân, loại sân, tình trạng, khung giờ và giá thuê.
- Booking: kiểm tra điều kiện đặt sân, trùng lịch, hủy và hoàn thành.
- Khách hàng: hồ sơ, thống kê booking, blacklist và ưu đãi.
- Hóa đơn/thanh toán: tạo hóa đơn và cập nhật trạng thái thủ công qua service.
- Tournament: yêu cầu, duyệt/từ chối, đội tham gia, lịch trận đơn giản và kết quả.
- Review: lưu nhận xét, rating và tổng hợp điểm trung bình.

GUI hiện có các màn hình xác thực, dashboard Owner, quản lý sân/khách/giải và các màn
hình Customer cho đặt sân, lịch sử, hồ sơ, ưu đãi, yêu cầu giải và đánh giá. Chưa có
màn hình riêng cho toàn bộ nghiệp vụ hóa đơn Owner; một số chức năng có service nhưng
chưa có luồng GUI độc lập.

## Công nghệ

- C++17.
- Dear ImGui, GLFW và OpenGL3 cho GUI desktop; không dùng Qt.
- Makefile/GNU Make; MinGW trên Windows.
- Không dùng framework kiểm thử ngoài: các test C++ hiện có chạy độc lập.

## Cấu trúc repository

```text
include/
  Core/                 LinkedList, HashTable, Queue, DateTime
  features/              Domain: Account, Booking, Customer, Field, Payment,
                         Storage, Tournament
  Repository/            Đọc/ghi và chuyển đổi dữ liệu
  Service/               Quy tắc nghiệp vụ và điều phối
  GUI/                   Application, ScreenManager, Theme, Widgets, Screens
src/
  Core/, features/, Repository/, Service/, GUI/
  main_gui.cpp           Điểm vào ứng dụng desktop
tests/
  core/                  Kiểm thử cấu trúc dữ liệu
  test_*.cpp             Unit/integration tests nghiệp vụ, lưu trữ và Facade
data/                    Dữ liệu mẫu/runtime đang sử dụng
data_backup/             Bản sao lưu dữ liệu được giữ nguyên trong repository
assets/                  Icon và font chữ
third_party/             Dear ImGui, GLFW và tài nguyên phụ thuộc đã vendored
build/                   Đầu ra build cục bộ; không cần commit binary
Makefile
```

## Kiến trúc

```text
Dear ImGui Screens → GUI::App::Facade → Service → Repository → FileManager → data/*.txt
```

- `Application` quản lý vòng đời GLFW/OpenGL/ImGui; `ScreenManager` điều phối màn hình.
- `Facade` cung cấp DTO và điểm vào GUI, đồng thời gọi các service.
- Service thực hiện quy tắc nghiệp vụ và nhận dependency qua constructor.
- Repository chuyển đổi bản ghi thành entity và lưu trữ qua `FileManager`.
- `FileManager` đọc/ghi văn bản và thay thế tệp qua tệp tạm để hạn chế ghi dở.

Một số phần tương thích cũ chưa đồng nhất hoàn toàn: `AccountRepository` vẫn gọi các
hàm tĩnh tương thích của `FileManager`. Xem thêm [tài liệu bảo vệ PBL2](docs/PBL2_BAO_VE.md)
để biết các giới hạn đã xác nhận.

## Build và chạy

Chạy lệnh từ thư mục gốc để các đường dẫn tương đối `data/` và `assets/` được phân giải
đúng.

Windows (MinGW):

```powershell
mingw32-make gui
.\build\QuanLySanBong_GUI.exe
```

Linux (GNU Make, g++, GLFW và OpenGL development libraries đã cài):

```sh
make gui
./build/QuanLySanBong_GUI
```

Xóa các object, executable và response file được sinh trong `build/` mà không đụng tới
`build/data/`:

```sh
mingw32-make clean   # Windows
make clean           # Linux
```

## Kiểm thử

Chạy toàn bộ 16 chương trình test tuần tự:

```powershell
mingw32-make test
```

```sh
make test
```

Có thể chạy riêng các nhóm, ví dụ `make test-core`, `make test-booking`,
`make test-customer`, `make test-file-manager` hoặc `make test-facade`. Test integration
có thể sửa các tệp trong `data/`; chỉ chạy trên dữ liệu mẫu/working copy có thể khôi
phục, không dùng dữ liệu vận hành thật.

## Lưu trữ và dữ liệu mẫu

Các repository đọc/ghi các tệp `.txt` như `accounts.txt`, `fields.txt`, `bookings.txt`,
`invoices.txt`, `tournaments.txt`, `teams.txt`, `matches.txt` và `reviews.txt`.
Bản ghi thường dùng dấu `|` phân cách. `data/` là dữ liệu mà ứng dụng chạy từ thư mục
gốc sử dụng; giữ nguyên dữ liệu này khi dọn build.

Dữ liệu mẫu có các ID Owner `US01`, `US06`, `US09` và Customer `US02`–`US05`,
`US07`–`US08`. GUI đăng nhập bằng số điện thoại và mật khẩu của bản ghi tương ứng.
Không ghi mật khẩu mẫu vào README: định dạng tài khoản hiện tại lưu trường mật khẩu
plaintext, nên chỉ dùng dữ liệu demo cục bộ và tuyệt đối không dùng thông tin thật.

## Giới hạn cần biết trước khi bảo vệ

- Mật khẩu hiện được lưu/so sánh dạng plaintext; không phù hợp cho triển khai thật.
- Một số màn hình/nghiệp vụ chưa được bao phủ bằng luồng GUI độc lập.
- Định dạng ngày trong dữ liệu legacy chưa đồng nhất hoàn toàn giữa ISO và
  `dd/MM/yyyy`; một số service chuyển đổi khi so sánh.
- `README_part_2.md` là tài liệu đề xuất ban đầu; trạng thái code hiện tại được mô tả
  ở README này và tài liệu bảo vệ.
