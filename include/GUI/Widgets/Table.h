#pragma once

#include <functional>
#include <string>
#include <vector>
#include <imgui.h>

namespace Widgets {

// Cấu hình một cột trong bảng
struct TableColumn {
    std::string tenCot;
    float chieuRong = 0.0f; // 0: tự động co giãn theo tỷ lệ
    ImGuiTableColumnFlags flags = 0;
};

// Dữ liệu của một dòng trong bảng
struct TableRow {
    std::string id;
    std::vector<std::string> oDuLieu;
    void* duLieuTuyChon = nullptr;
};

// Trạng thái tìm kiếm, sắp xếp và phân trang của bảng
struct TableState {
    std::string tuKhoaTimKiem;
    int soDongMoiTrang = 10;
    int trangHienTai = 1;
    int cotSapXep = 0;
    bool sapXepTangDan = true;
    int dongDangChon = -1; // Index trong danh sách gốc
};

// LÝ DO CHỌN: Bảng dữ liệu chuyên nghiệp bọc ImGuiTable kết hợp tìm kiếm tức thời,
// sắp xếp theo từng cột, tô xen kẽ dòng, phân trang linh hoạt và callback khi click dòng.
bool Table(
    const char* id,
    const std::vector<TableColumn>& cacCot,
    const std::vector<TableRow>& danhSachDong,
    TableState& state,
    const std::function<void(const TableRow&, int cot)>& customRenderCell = nullptr,
    const std::function<void(const TableRow&)>& onDongClicked = nullptr,
    ImGuiTableFlags extraFlags = ImGuiTableFlags_None
);

}  // namespace Widgets
