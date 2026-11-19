#pragma once

namespace Widgets {

// Khung nội dung giới hạn bề rộng tối đa, tự co theo chiều cao nội dung.
// LÝ DO CHỌN: form trải hết 1600px trên màn hình rộng rất khó đọc; giới hạn ~800-1000px
// giúp mắt quét ngắn hơn. Luôn gọi EndNoiDungHep() tương ứng.
void BeginNoiDungHep(const char* id, float beRongToiDa);
void EndNoiDungHep();

// Hai cột cạnh nhau (rộng theo tỷ lệ), mỗi cột tự co chiều cao. Dùng thay cho Table/Columns
// để Card (vẽ bằng ImDrawListSplitter riêng) hoạt động an toàn bên trong.
void BeginCot(const char* id, float beRong);
void EndCot();

}  // namespace Widgets
