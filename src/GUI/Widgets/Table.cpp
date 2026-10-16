#include "GUI/Widgets/Table.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/InputFields.h"

namespace Widgets {

namespace {

// Chuyển chuỗi về chữ thường để tìm kiếm không phân biệt hoa thường
std::string toLowerString(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

// Kiểm tra và chuyển đổi số nguyên hoặc số thực nếu có thể để sắp xếp số học chính xác
bool tryParseNumber(const std::string& s, double& outVal) {
    if (s.empty()) return false;
    std::stringstream ss(s);
    ss >> outVal;
    return !ss.fail() && ss.eof();
}

}  // namespace

bool Table(
    const char* id,
    const std::vector<TableColumn>& cacCot,
    const std::vector<TableRow>& danhSachDong,
    TableState& state,
    const std::function<void(const TableRow&, int cot)>& customRenderCell,
    const std::function<void(const TableRow&)>& onDongClicked
) {
    if (cacCot.empty()) {
        return false;
    }

    ImGui::PushID(id);
    bool clickedRowEvent = false;

    // ==========================================
    // 1. THANH CÔNG CỤ TRÊN BẢNG (Tìm kiếm & Phân trang)
    // ==========================================
    float availWidth = ImGui::GetContentRegionAvail().x;
    float searchWidth = 240.0f;
    float pageSizeWidth = 140.0f;

    // Ô tìm kiếm
    ImGui::SetNextItemWidth(searchWidth);
    InputText(nullptr, "##table_search", state.tuKhoaTimKiem, nullptr, u8"Tìm kiếm...");

    ImGui::SameLine();
    ImGui::SetCursorPosX(availWidth - pageSizeWidth);

    // Hộp chọn số dòng mỗi trang (10 / 20 / 50)
    const char* pageSizeLabels[] = {u8"10 dòng/trang", u8"20 dòng/trang", u8"50 dòng/trang"};
    int pageSizeValues[] = {10, 20, 50};
    int currentSizeIdx = 0;
    if (state.soDongMoiTrang == 20) currentSizeIdx = 1;
    else if (state.soDongMoiTrang == 50) currentSizeIdx = 2;

    ImGui::SetNextItemWidth(pageSizeWidth);
    if (ImGui::BeginCombo("##page_size", pageSizeLabels[currentSizeIdx])) {
        for (int i = 0; i < 3; ++i) {
            bool isSelected = (currentSizeIdx == i);
            if (ImGui::Selectable(pageSizeLabels[i], isSelected)) {
                state.soDongMoiTrang = pageSizeValues[i];
                state.trangHienTai = 1;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();

    // ==========================================
    // 2. LỌC DỮ LIỆU THEO TỪ KHÓA
    // ==========================================
    std::string keywordLower = toLowerString(state.tuKhoaTimKiem);
    std::vector<size_t> filteredIndices;
    filteredIndices.reserve(danhSachDong.size());

    for (size_t i = 0; i < danhSachDong.size(); ++i) {
        if (keywordLower.empty()) {
            filteredIndices.push_back(i);
        } else {
            bool match = false;
            for (const auto& cell : danhSachDong[i].oDuLieu) {
                if (toLowerString(cell).find(keywordLower) != std::string::npos) {
                    match = true;
                    break;
                }
            }
            if (match) {
                filteredIndices.push_back(i);
            }
        }
    }

    // ==========================================
    // 3. SẮP XẾP DỮ LIỆU
    // ==========================================
    if (state.cotSapXep >= 0 && state.cotSapXep < static_cast<int>(cacCot.size())) {
        int sortCol = state.cotSapXep;
        bool asc = state.sapXepTangDan;

        std::stable_sort(filteredIndices.begin(), filteredIndices.end(), [&](size_t a, size_t b) {
            const auto& rowA = danhSachDong[a];
            const auto& rowB = danhSachDong[b];

            std::string valA = (sortCol < static_cast<int>(rowA.oDuLieu.size())) ? rowA.oDuLieu[sortCol] : "";
            std::string valB = (sortCol < static_cast<int>(rowB.oDuLieu.size())) ? rowB.oDuLieu[sortCol] : "";

            double numA = 0.0, numB = 0.0;
            if (tryParseNumber(valA, numA) && tryParseNumber(valB, numB)) {
                return asc ? (numA < numB) : (numA > numB);
            }
            return asc ? (valA < valB) : (valA > valB);
        });
    }

    // ==========================================
    // 4. TÍNH TOÁN PHÂN TRANG
    // ==========================================
    int totalItems = static_cast<int>(filteredIndices.size());
    int totalPages = (totalItems + state.soDongMoiTrang - 1) / state.soDongMoiTrang;
    if (totalPages < 1) totalPages = 1;
    if (state.trangHienTai > totalPages) state.trangHienTai = totalPages;
    if (state.trangHienTai < 1) state.trangHienTai = 1;

    int startIndex = (state.trangHienTai - 1) * state.soDongMoiTrang;
    int endIndex = std::min(startIndex + state.soDongMoiTrang, totalItems);

    // ==========================================
    // 5. HIỂN THỊ IMGUI TABLE
    // ==========================================
    ImGuiTableFlags flags = ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_BordersInnerV |
                            ImGuiTableFlags_BordersOuter |
                            ImGuiTableFlags_Resizable |
                            ImGuiTableFlags_Reorderable |
                            ImGuiTableFlags_Sortable |
                            ImGuiTableFlags_SizingStretchProp;

    if (ImGui::BeginTable("##main_table", static_cast<int>(cacCot.size()), flags)) {
        // Thiết lập header các cột
        for (size_t colIdx = 0; colIdx < cacCot.size(); ++colIdx) {
            const auto& col = cacCot[colIdx];
            ImGui::TableSetupColumn(col.tenCot.c_str(), col.flags, col.chieuRong);
        }
        ImGui::TableHeadersRow();

        // Xử lý sự kiện click header cột để sắp xếp
        if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
            if (sortSpecs->SpecsDirty && sortSpecs->SpecsCount > 0) {
                state.cotSapXep = sortSpecs->Specs[0].ColumnIndex;
                state.sapXepTangDan = (sortSpecs->Specs[0].SortDirection == ImGuiSortDirection_Ascending);
                sortSpecs->SpecsDirty = false;
            }
        }

        // Vẽ các dòng thuộc trang hiện tại
        for (int i = startIndex; i < endIndex; ++i) {
            size_t origIdx = filteredIndices[i];
            const auto& row = danhSachDong[origIdx];

            ImGui::TableNextRow();
            bool isRowSelected = (state.dongDangChon == static_cast<int>(origIdx));

            for (size_t colIdx = 0; colIdx < cacCot.size(); ++colIdx) {
                ImGui::TableSetColumnIndex(static_cast<int>(colIdx));

                if (colIdx == 0) {
                    // Cột đầu tiên sử dụng Selectable kéo dài toàn bộ hàng để chọn dòng
                    std::string rowSelectableId = "##row_sel_" + std::to_string(origIdx);
                    if (ImGui::Selectable(rowSelectableId.c_str(), isRowSelected,
                                          ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
                        state.dongDangChon = static_cast<int>(origIdx);
                        if (onDongClicked) {
                            onDongClicked(row);
                        }
                        clickedRowEvent = true;
                    }
                    ImGui::SameLine();
                }

                if (customRenderCell) {
                    customRenderCell(row, static_cast<int>(colIdx));
                } else {
                    std::string cellText = (colIdx < row.oDuLieu.size()) ? row.oDuLieu[colIdx] : "";
                    ImGui::TextUnformatted(cellText.c_str());
                }
            }
        }

        ImGui::EndTable();
    }

    ImGui::Spacing();

    // ==========================================
    // 6. THANH ĐIỀU HƯỚNG PHÂN TRANG (Pagination)
    // ==========================================
    int startDisplay = totalItems > 0 ? (startIndex + 1) : 0;
    int endDisplay   = endIndex;
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
    ImGui::Text(u8"Hiển thị %d - %d trên tổng %d kết quả", startDisplay, endDisplay, totalItems);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    float navWidth = 260.0f;
    ImGui::SetCursorPosX(availWidth - navWidth);

    // Nút phân trang: Đầu, Trước, Trang X/Y, Sau, Cuối
    bool disablePrev = (state.trangHienTai <= 1);
    bool disableNext = (state.trangHienTai >= totalPages);

    if (disablePrev) ImGui::BeginDisabled();
    if (ImGui::Button("<<##first")) {
        state.trangHienTai = 1;
    }
    ImGui::SameLine();
    if (ImGui::Button("<##prev")) {
        state.trangHienTai--;
    }
    if (disablePrev) ImGui::EndDisabled();

    ImGui::SameLine();
    std::string pageInfo = std::to_string(state.trangHienTai) + " / " + std::to_string(totalPages);
    float textW = ImGui::CalcTextSize(pageInfo.c_str()).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (80.0f - textW) * 0.5f);
    ImGui::TextUnformatted(pageInfo.c_str());
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (80.0f - textW) * 0.5f);

    if (disableNext) ImGui::BeginDisabled();
    if (ImGui::Button(">##next")) {
        state.trangHienTai++;
    }
    ImGui::SameLine();
    if (ImGui::Button(">>##last")) {
        state.trangHienTai = totalPages;
    }
    if (disableNext) ImGui::EndDisabled();

    ImGui::PopID();
    return clickedRowEvent;
}

}  // namespace Widgets
