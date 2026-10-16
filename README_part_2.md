# Quản lý sân bóng mini — PBL2

## 1. Giới thiệu

Đây là dự án PBL2 **Xây dựng phần mềm quản lý sân bóng mini** bằng C++.

Phạm vi trọng tâm của hệ thống là:

- Quản lý tài khoản **chủ sân** và **người thuê**.
- Quản lý sân bóng, khung giờ, giá thuê và trạng thái sân.
- Cho phép người thuê **tìm kiếm/liên hệ và đặt sân**, đồng thời hỗ trợ hủy sân.
- Quản lý thông tin khách hàng.
- Theo dõi số lần đặt/hủy để hỗ trợ **ưu đãi** và **danh sách đen**.
- Quản lý đánh giá sân.
- Cung cấp thống kê/báo cáo cho chủ sân.

### Tính năng phụ

`Tournament` (quản lý giải đấu) chỉ là **tính năng đi kèm**, không phải nghiệp vụ trung tâm của hệ thống. Vì vậy module này được thiết kế đơn giản và chỉ mở rộng khi phần quản lý sân và đặt sân đã hoàn thiện.

---

# 2. Kiến trúc tổng thể

Dự án tổ chức theo hướng **3 tầng** kết hợp với các lớp Domain Model:

```text
┌─────────────────────────────────────────────┐
│                 PRESENTATION                │
│                     UI                      │
│  LoginMenu / OwnerMenu / RenterMenu / ...   │
└──────────────────────┬──────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────┐
│              BUSINESS / SERVICE             │
│                                             │
│ AccountService                              │
│ FieldService                                │
│ BookingService       ← NGHIỆP VỤ TRỌNG TÂM │
│ CustomerService                             │
│ ReportService                               │
└──────────────────────┬──────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────┐
│              DATA ACCESS / REPOSITORY       │
│                                             │
│ AccountRepository                           │
│ FieldRepository                             │
│ BookingRepository                           │
│ CustomerRepository                          │
└──────────────────────┬──────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────┐
│                   STORAGE                   │
│                 FileManager                 │
└──────────────────────┬──────────────────────┘
                       │
                       ▼
                    data/*.txt
```

Các lớp Domain Model (`Account`, `Field`, `Booking`, `Customer`, ...) là các đối tượng dữ liệu mà Service và Repository thao tác.

---

# 3. Cấu trúc thư mục đề xuất

Cấu trúc chính thức nên hướng tới:

```text
PBL2-Quan_ly_san_bong_mini/
│
├── include/
│   │
│   ├── Core/
│   │   ├── LinkedList.h
│   │   ├── HashTable.h
│   │   ├── Queue.h
│   │   └── DateTime.h
│   │
│   ├── Account/
│   │   ├── User.h
│   │   ├── Owner.h
│   │   └── Renter.h
│   │
│   ├── Field/
│   │   ├── Field.h
│   │   └── TimeSlot.h
│   │
│   ├── Booking/
│   │   ├── Booking.h
│   │   └── BookingDetail.h
│   │
│   ├── Customer/
│   │   ├── Customer.h
│   │   ├── Blacklist.h
│   │   ├── Discount.h
│   │   └── Review.h
│   │
│   ├── Tournament/
│   │   ├── Tournament.h
│   │   ├── Team.h
│   │   ├── TournamentMatch.h
│   │   └── TournamentResult.h
│   │
│   ├── Service/
│   │   ├── AccountService.h
│   │   ├── FieldService.h
│   │   ├── BookingService.h
│   │   ├── CustomerService.h
│   │   └── ReportService.h
│   │
│   ├── Repository/
│   │   ├── AccountRepository.h
│   │   ├── FieldRepository.h
│   │   ├── BookingRepository.h
│   │   └── CustomerRepository.h
│   │
│   ├── Storage/
│   │   └── FileManager.h
│   │
│   └── UI/
│       ├── Menu.h
│       ├── LoginMenu.h
│       ├── OwnerMenu.h
│       ├── RenterMenu.h
│       ├── FieldMenu.h
│       ├── BookingMenu.h
│       ├── CustomerMenu.h
│       └── ReportMenu.h
│
├── src/
│   ├── Core/
│   ├── Account/
│   ├── Field/
│   ├── Booking/
│   ├── Customer/
│   ├── Tournament/
│   ├── Service/
│   ├── Repository/
│   ├── Storage/
│   ├── UI/
│   └── main.cpp
│
├── data/
│   ├── accounts.txt
│   ├── fields.txt
│   ├── bookings.txt
│   ├── customers.txt
│   ├── blacklist.txt
│   ├── discounts.txt
│   ├── reviews.txt
│   ├── tournaments.txt
│   ├── teams.txt
│   ├── matches.txt
│   └── ...
│
├── tests/
│   ├── testLinkedList.cpp
│   ├── testHashTable.cpp
│   ├── testQueue.cpp
│   ├── testField.cpp
│   ├── testBooking.cpp
│   └── testCustomer.cpp
│
├── docs/
│   ├── diagrams/
│   ├── requirements/
│   └── report/
│
├── build/
├── Makefile
└── README.md
```

> `build/` chỉ chứa sản phẩm build và file tạm; không commit các file `.o` hoặc executable lên Git.

---

# 4. Trách nhiệm của từng thư mục

## 4.1. `Core/`

Chứa các cấu trúc dữ liệu tự cài đặt và các tiện ích dùng chung.

### `LinkedList`

Dùng cho các danh sách cần thêm/xóa linh hoạt, ví dụ:

- Danh sách đánh giá.
- Danh sách đen.
- Danh sách booking.
- Các danh sách đối tượng khác khi phù hợp.

### `HashTable`

Dùng cho các bài toán tra cứu nhanh, ví dụ:

- Tra cứu khách hàng theo số điện thoại.
- Tra cứu đối tượng theo ID.

Không ép mọi dữ liệu phải dùng HashTable.

### `Queue`

Dùng cho nghiệp vụ cần xử lý theo thứ tự FIFO khi thật sự phù hợp.

### `DateTime`

Chứa các hàm/kiểu dùng chung để kiểm tra:

- Ngày hợp lệ.
- Giờ hợp lệ.
- So sánh thời gian.
- Kiểm tra khoảng thời gian.

---

# 5. Domain Model

## 5.1. `Account/`

Mô hình hóa tài khoản.

```text
User
├── Owner
└── Renter
```

`User` chứa thông tin chung của tài khoản.

`Owner` đại diện cho chủ sân.

`Renter` đại diện cho người thuê.

### Lưu ý quan trọng

`Owner` **không nên chứa thông tin cụ thể của một sân** như:

```text
field_name
field_address
field_count
open_time
close_time
```

Các thông tin này thuộc `Field`.

Quan hệ nên là:

```text
Owner 1 ───────── N Field
```

Một chủ sân có thể quản lý nhiều sân.

Ngoài ra, Domain Model không nên trực tiếp phụ thuộc vào UI. Vì vậy menu của Owner/Renter nên được xử lý bởi `OwnerMenu` và `RenterMenu`, thay vì để `User` chịu trách nhiệm hiển thị menu.

---

# 6. `Field/`

Đây là một trong các module cốt lõi.

### `Field`

Quản lý:

- Mã sân.
- Tên sân.
- Loại sân: 5/7/11.
- Thông tin sân.
- Giá thuê.
- Quy định.
- Chủ sân.
- Trạng thái.

### `TimeSlot`

Quản lý thông tin về khung giờ thuê.

Mục tiêu quan trọng nhất của module này là hỗ trợ kiểm tra:

```text
Sân S01
Ngày 10/09

18:00 - 19:00  Đã đặt
19:00 - 20:00  Còn trống
20:00 - 21:00  Còn trống
```

---

# 7. `Booking/`

Đây là **nghiệp vụ trung tâm của hệ thống**.

Booking kết nối:

```text
Customer
    │
    │ đặt
    ▼
 Booking
    │
    │ chọn
    ▼
 Field
    │
    ▼
 TimeSlot
```

`BookingService` phải chịu trách nhiệm cho các logic như:

- Kiểm tra khách hàng.
- Kiểm tra sân.
- Kiểm tra ngày/giờ.
- Kiểm tra giờ mở/đóng cửa.
- Kiểm tra sân có bị trùng lịch.
- Kiểm tra điều kiện đặt sân.
- Tạo booking.
- Hủy booking.
- Cập nhật trạng thái booking.
- Cập nhật các thông tin liên quan sau khi đặt/hủy.

`BookingRepository` chỉ chịu trách nhiệm truy xuất/lưu trữ booking.

---

# 8. `Customer/`

Quản lý thông tin và lịch sử của khách hàng.

### `Customer`

Thông tin khách hàng.

### `Blacklist`

Quản lý khách bị đưa vào danh sách đen theo quy tắc nghiệp vụ.

### `Discount`

Quản lý thông tin/điều kiện ưu đãi.

### `Review`

Quản lý đánh giá của khách hàng đối với sân.

Các quy tắc như:

```text
Khách đặt nhiều lần → đủ điều kiện ưu đãi

Khách hủy nhiều lần → có thể bị đưa vào blacklist
```

là **business logic**, vì vậy nên xử lý trong `CustomerService`/`BookingService`, không xử lý trong Repository.

---

# 9. `Service/`

Service là tầng xử lý nghiệp vụ.

## `AccountService`

Phụ trách:

- Đăng ký.
- Đăng nhập.
- Kiểm tra tài khoản.
- Kiểm tra loại tài khoản.
- Cập nhật thông tin tài khoản.

## `FieldService`

Phụ trách:

- Tạo sân.
- Sửa sân.
- Xóa sân.
- Kiểm tra sân.
- Kiểm tra giờ mở/đóng cửa.
- Kiểm tra khung giờ.
- Kiểm tra trạng thái sân.

## `BookingService`

**Service quan trọng nhất.**

Phụ trách toàn bộ logic đặt/hủy sân và kiểm tra điều kiện đặt sân.

## `CustomerService`

Phụ trách:

- Quản lý khách hàng.
- Theo dõi số lần đặt.
- Theo dõi số lần hủy.
- Xử lý blacklist.
- Xử lý điều kiện ưu đãi.
- Quản lý đánh giá.

## `ReportService`

Không dùng để kiểm tra xem một booking có hợp lệ hay không.

Nó dùng để **tổng hợp dữ liệu đã có** thành báo cáo:

- Doanh thu.
- Số lượt đặt sân.
- Số lượt hủy.
- Sân được đặt nhiều.
- Khách hàng đặt nhiều.
- Thống kê theo ngày/tháng.

Ví dụ:

```text
ReportService
      │
      ├── BookingRepository
      ├── CustomerRepository
      └── FieldRepository
              │
              ▼
          Tổng hợp
              │
              ▼
            Report
```

---

# 10. `Repository/`

Repository là tầng truy xuất dữ liệu theo từng loại đối tượng.

Ví dụ:

```text
BookingService
      │
      ▼
BookingRepository
      │
      ▼
FileManager
      │
      ▼
bookings.txt
```

## Repository làm gì?

```text
findById()
findAll()
save()
update()
remove()
```

Tùy từng entity có thể bổ sung hàm tìm kiếm phù hợp.

## Repository KHÔNG làm gì?

Không đặt các quy tắc nghiệp vụ như:

```text
Khách hủy 3 lần → blacklist
Sân đã được đặt → từ chối booking
Khách đặt đủ số lần → giảm giá
```

Những việc này thuộc Service.

---

# 11. `Storage/`

## `FileManager`

Chịu trách nhiệm làm việc với file:

- Mở file.
- Đọc dữ liệu.
- Ghi dữ liệu.
- Thêm dữ liệu.
- Kiểm tra lỗi file.

`FileManager` không cần biết nghiệp vụ của sân bóng.

Ví dụ:

```text
FileManager
    │
    ├── accounts.txt
    ├── fields.txt
    ├── bookings.txt
    └── customers.txt
```

Repository sẽ quyết định dữ liệu nào thuộc file nào.

---

# 12. `UI/`

UI chỉ chịu trách nhiệm:

- Hiển thị menu.
- Nhận input.
- Hiển thị kết quả/lỗi cho người dùng.
- Gọi Service tương ứng.

Ví dụ:

```text
BookingMenu
     │
     ▼
BookingService
```

Không nên đặt toàn bộ logic kiểm tra booking trong `BookingMenu`.

---

# 13. `Tournament/`

`Tournament` là **module phụ trợ**.

Mục đích của module này là hỗ trợ chủ sân/người thuê quản lý thông tin giải đấu và liên hệ đặt sân cho sự kiện.

Không nên để Tournament trở thành một hệ thống lớn hơn nghiệp vụ đặt sân.

Có thể giữ:

```text
Tournament/
├── Tournament.h
├── Team.h
├── TournamentMatch.h
└── TournamentResult.h
```

Nhưng các `TournamentService` và `TournamentRepository` **chưa cần triển khai ngay**.

Chỉ thêm chúng khi logic Tournament thực sự đủ lớn để cần tách riêng.

---

# 14. Những module KHÔNG cần ưu tiên hiện tại

Trong phiên bản hiện tại, không cần mở rộng ngay thành:

```text
PaymentService
PaymentRepository
InvoiceService
TournamentService
TournamentRepository
...
```

Đặc biệt, hệ thống hiện tập trung vào **liên hệ/đặt sân**, không phải hệ thống thanh toán trực tuyến.

Nếu sau này yêu cầu đề tài có thanh toán thì mới phát triển:

```text
Payment/
├── Payment.h
└── Invoice.h

Service/
└── PaymentService.h
```

---

# 15. Quy tắc phụ thuộc giữa các tầng

Nên giữ nguyên nguyên tắc:

```text
UI
 ↓
Service
 ↓
Repository
 ↓
Storage
 ↓
Data
```

Domain Model được sử dụng bởi các tầng này.

### Không nên

```text
UI → FileManager
UI → .txt
UI → Repository để tự xử lý nghiệp vụ

Entity → UI
Entity → FileManager
Entity → .txt
```

### Nên

```text
UI
 ↓
Service
 ↓
Repository
 ↓
FileManager
 ↓
.txt
```

---

# 16. Ví dụ flow đặt sân

```text
Người thuê
    │
    ▼
RenterMenu
    │
    ▼
BookingMenu
    │
    ▼
BookingService
    │
    ├── kiểm tra Customer
    ├── kiểm tra Field
    ├── kiểm tra TimeSlot
    ├── kiểm tra giờ mở/đóng
    ├── kiểm tra trùng lịch
    ├── kiểm tra blacklist
    │
    ▼
Tạo Booking
    │
    ▼
BookingRepository
    │
    ▼
FileManager
    │
    ▼
bookings.txt
```

Đây là flow quan trọng nhất cần hoàn thiện và kiểm thử.

---

# 17. Sử dụng cấu trúc dữ liệu

Các cấu trúc dữ liệu tự cài đặt phải được sử dụng có lý do.

| Cấu trúc | Mục đích dự kiến |
|---|---|
| `HashTable` | Tra cứu nhanh theo ID/SĐT |
| `LinkedList` | Danh sách có thêm/xóa linh hoạt |
| `Queue` | Các nghiệp vụ cần FIFO |
| `DateTime` | Kiểm tra và xử lý thời gian |

Không nên sử dụng một cấu trúc dữ liệu chỉ để "có sử dụng".

Trong báo cáo cần giải thích:

1. Vì sao chọn cấu trúc đó?
2. Nó giải quyết bài toán nào?
3. Độ phức tạp của các thao tác chính?
4. Tại sao không dùng cấu trúc khác?

---

# 18. Thứ tự triển khai

Không nên code tất cả module cùng lúc.

## Giai đoạn 1 — Core

Hoàn thiện và test:

```text
LinkedList
HashTable
Queue
DateTime
```

## Giai đoạn 2 — Account

Hoàn thiện:

```text
User
Owner
Renter
AccountService
AccountRepository
```

## Giai đoạn 3 — Storage

Hoàn thiện:

```text
FileManager
```

## Giai đoạn 4 — Field

Hoàn thiện:

```text
Field
TimeSlot
FieldService
FieldRepository
```

## Giai đoạn 5 — Booking

Đây là **giai đoạn quan trọng nhất**:

```text
Booking
BookingDetail
BookingService
BookingRepository
```

Test kỹ:

- Đặt sân.
- Hủy sân.
- Trùng giờ.
- Ngoài giờ mở cửa.
- Sân không tồn tại.
- Khách không tồn tại.
- Khách trong blacklist.

## Giai đoạn 6 — Customer

Hoàn thiện:

```text
Customer
Blacklist
Discount
Review
CustomerService
CustomerRepository
```

## Giai đoạn 7 — Report

Sau khi có đủ dữ liệu:

```text
ReportService
ReportMenu
```

## Giai đoạn 8 — Tournament

Làm sau cùng:

```text
Tournament
Team
TournamentMatch
TournamentResult
```

## Giai đoạn 9 — UI hoàn chỉnh

Ghép các Service vào menu.

## Giai đoạn 10 — Testing + Documentation

Hoàn thiện:

```text
tests/
docs/
README.md
```

---

# 19. Những chỉnh sửa quan trọng so với cấu trúc thử nghiệm hiện tại

Phiên bản hiện tại đã có khá nhiều thư mục đúng hướng, nhưng đang **hơi dư module so với phạm vi đề tài**.

Các thay đổi được đề xuất:

### Giữ

```text
Core/
Account/
Field/
Booking/
Customer/
Tournament/
Storage/
UI/
Service/
Repository/
```

### Giữ nhưng ưu tiên thấp

```text
Tournament/
```

### Tạm thời chưa cần triển khai

```text
Payment/
PaymentService.h
```

### Nên bỏ hoặc để dành

```text
Repository/TournamentRepository.h
Service/TournamentService.h
```

cho tới khi Tournament thực sự cần tầng riêng.

### Nên thêm

```text
tests/
```

### Nên sửa

`Owner` không nên lưu thông tin của một Field cụ thể.

`Renter.h` cần thống nhất tên class với module. Nếu file là `Renter.h` thì class nên là `Renter`, không nên khai báo `Customer`.

---

# 20. Lưu ý về Makefile

Khi bắt đầu có `.cpp` cho `Service` và `Repository`, Makefile phải include các thư mục tương ứng.

Ví dụ:

```make
SRC_DIRS = src \
           src/Core \
           src/Account \
           src/Field \
           src/Booking \
           src/Customer \
           src/Tournament \
           src/Service \
           src/Repository \
           src/Storage \
           src/UI
```

Không nên quên `Service` và `Repository`, nếu không các file `.cpp` trong hai thư mục này sẽ không được build.

---

# 21. Nguyên tắc thiết kế quan trọng nhất

Hệ thống nên được xây dựng với tư duy:

```text
               ⭐ BOOKING ⭐
                    │
          ┌─────────┼─────────┐
          │         │         │
       Customer   Field     Account
          │         │
          └────┬────┘
               │
            Booking
               │
         ┌─────┴─────┐
         │           │
      Report     Tournament
                  (phụ trợ)
```

**Đừng làm Tournament, Payment hay các tính năng phụ trước khi nghiệp vụ Field + Booking chạy ổn định.**

Mục tiêu cuối cùng của kiến trúc là:

```text
UI
 │
 ▼
Service
 │
 ├── kiểm tra logic nghiệp vụ
 │
 ▼
Repository
 │
 ├── truy xuất dữ liệu
 │
 ▼
FileManager
 │
 ▼
.txt
```

Nhờ cách tổ chức này, mỗi file có một trách nhiệm rõ ràng, dễ test, dễ mở rộng và dễ giải thích trong báo cáo PBL2.
