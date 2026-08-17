# Quản lý sân bóng mini — PBL2

## Cấu trúc thư mục

```
QuanLySanBongMini/
├── include/            # Header (.h) — khai báo class, chia theo module
│   ├── Core/           # Cấu trúc dữ liệu tự cài đặt (LinkedList, HashTable, Queue...)
│   ├── Account/        # NguoiDung (base), ChuSan, NguoiThue (kế thừa + đa hình)
│   ├── Field/           # San (sân bóng), KhungGio
│   ├── Booking/        # DatSan (đặt/hủy sân)
│   ├── Customer/       # KhachHang, danh sách đen, ưu đãi, đánh giá
│   ├── Tournament/     # Giai (giải đấu)
│   ├── Storage/        # FileManager — đọc/ghi file dùng chung
│   └── UI/             # Menu — giao diện dòng lệnh
├── src/                # File .cpp cài đặt tương ứng, cùng cấu trúc với include/
│   └── main.cpp
├── data/               # File dữ liệu lưu trữ (.txt) — accounts, fields, bookings...
├── docs/               # Báo cáo, sơ đồ lớp (class diagram), slide
├── build/              # File thực thi sau khi build (không commit lên git)
└── Makefile
```

## Cách build và chạy

```bash
make run       # build và chạy chương trình
make clean     # dọn file .o và file thực thi
```

## Thứ tự triển khai đề xuất

Đừng làm tất cả module cùng lúc. Thầy gợi ý làm theo đúng thứ tự này vì các module sau
phụ thuộc vào module trước:

1. **Core** — cài đặt xong `LinkedList`, `HashTable` trước tiên (đây là "công cụ" mà
   mọi module khác sẽ dùng lại). Viết unit test nhỏ để chắc chắn nó chạy đúng trước khi
   dùng ở nơi khác.
2. **Account** — cài `NguoiDung` (lớp cơ sở) rồi mới cài `ChuSan`, `NguoiThue` kế thừa.
   Đây là chỗ thầy khuyên em làm kỹ vì GVHD sẽ hỏi về tính kế thừa/đa hình ở đây.
3. **Storage** — cài `FileManager` để đọc/ghi được ngay, vì từ bước 4 trở đi module nào
   cũng cần lưu file.
4. **Field & Booking** — quản lý sân và đặt sân là lõi nghiệp vụ chính, nên làm kỹ và
   test nhiều nhất.
5. **Customer** — dùng lại `HashTable` (đã làm ở bước 1) để tra cứu khách theo SĐT.
6. **Tournament** — module đơn giản nhất, làm sau cùng.
7. **UI** — sau khi các module nghiệp vụ đã chạy được độc lập (test qua `main.cpp` đơn
   giản), mới ghép UI để điều phối toàn bộ menu.

## Lưu ý khi làm báo cáo

Đề bài yêu cầu giải thích **tại sao chọn cấu trúc dữ liệu đó**. Em nên ghi chú ngay
trong code (comment) và note lại lý do trong lúc code, để khi viết báo cáo khỏi phải nhớ lại:
- Vì sao dùng `HashTable` cho tra cứu khách hàng / tình trạng đặt sân? (độ phức tạp O(1))
- Vì sao dùng `LinkedList` cho danh sách đánh giá / danh sách đen? (thêm/xóa linh hoạt,
  không cần truy cập ngẫu nhiên)
- Vì sao `NguoiDung` là lớp trừu tượng với hàm ảo thuần túy `hienThiMenu()`?

## Việc cần làm tiếp (đánh dấu TODO trong code)

Các file trong `include/` hiện chỉ là khung (khai báo), **chưa có phần cài đặt** (`src/*.cpp`
tương ứng còn thiếu — em cần tự viết). Đây là thiết kế có chủ đích: thầy để em tự viết phần
logic để hiểu sâu, thay vì đưa sẵn code hoàn chỉnh.
