#include "GUI/Widgets/SlotGrid.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/DatePicker.h"

namespace Widgets {

namespace {

// Hàm tiện ích cộng/trừ ngày cho chuỗi dd/MM/yyyy
std::string thayDoiNgay(const std::string& dateStr, int deltaDays) {
    int d = 1, m = 1, y = 2026;
    if (dateStr.length() == 10 && dateStr[2] == '/' && dateStr[5] == '/') {
        try {
            d = std::stoi(dateStr.substr(0, 2));
            m = std::stoi(dateStr.substr(3, 2));
            y = std::stoi(dateStr.substr(6, 4));
        } catch (...) {
            return dateStr;
        }
    } else {
        return dateStr;
    }

    std::tm timeInfo{};
    timeInfo.tm_mday = d;
    timeInfo.tm_mon  = m - 1;
    timeInfo.tm_year = y - 1900;
    timeInfo.tm_isdst = -1;

    std::time_t tt = std::mktime(&timeInfo);
    if (tt == -1) return dateStr;

    tt += deltaDays * 86400; // 86400 giây = 1 ngày

    std::tm newTime{};
#if defined(_WIN32)
    localtime_s(&newTime, &tt);
#else
    localtime_r(&tt, &newTime);
#endif

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << newTime.tm_mday << "/"
        << std::setfill('0') << std::setw(2) << (newTime.tm_mon + 1) << "/"
        << (newTime.tm_year + 1900);
    return oss.str();
}

}  // namespace

bool SlotGrid::Render(
    const char* id,
    std::string& ngayHienTai,
    const std::vector<std::string>& danhSachMaSan,
    const std::vector<std::string>& danhSachKhungGio,
    const std::map<std::pair<std::string, std::string>, SlotInfo>& maTranSlot,
    bool choPhepChon,
    const std::string& maSanDangChon,
    const std::string& khungGioDangChon,
    const std::function<void(const std::string&, const std::string&)>& onSlotClicked
) {
    ImGui::PushID(id);
    bool coThayDoi = false;

    // ==========================================
    // 1. THANH ĐIỀU HƯỚNG NGÀY & CHÚ GIẢI MÀU (Legend)
    // ==========================================
    ImGui::BeginGroup();

    // Nút lùi 1 ngày
    if (Widgets::NutPhu("<##prev_day", ImVec2(32.0f, 32.0f))) {
        ngayHienTai = thayDoiNgay(ngayHienTai, -1);
        coThayDoi = true;
    }
    ImGui::SameLine();

    // Nút tiến 1 ngày
    if (Widgets::NutPhu(">##next_day", ImVec2(32.0f, 32.0f))) {
        ngayHienTai = thayDoiNgay(ngayHienTai, 1);
        coThayDoi = true;
    }
    ImGui::SameLine();

    // DatePicker chọn ngày trực tiếp
    ImGui::SetNextItemWidth(220.0f);
    if (Widgets::DatePicker(nullptr, "##slot_date_picker", ngayHienTai)) {
        coThayDoi = true;
    }

    ImGui::EndGroup();

    // Chú giải màu bên phải
    ImGui::SameLine(0.0f, 30.0f);
    ImGui::BeginGroup();

    auto veChuGiai = [](const char* label, ImU32 mauNen, ImU32 mauVien) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        ImVec2 p1 = ImVec2(p0.x + 14.0f, p0.y + 14.0f);
        dl->AddRectFilled(p0, p1, mauNen, 3.0f);
        dl->AddRect(p0, p1, mauVien, 3.0f, 0, 1.0f);
        ImGui::SetCursorScreenPos(ImVec2(p0.x + 20.0f, p0.y - 2.0f));
        ImGui::TextUnformatted(label);
        ImGui::SameLine(0.0f, 16.0f);
    };

    veChuGiai(u8"Trống",
              GUI::Theme::Colors::toU32(ImVec4(0.88f, 0.96f, 0.91f, 1.0f)),
              GUI::Theme::Colors::toU32(GUI::Theme::Colors::GrassGreen));

    veChuGiai(u8"Đã đặt",
              GUI::Theme::Colors::toU32(ImVec4(0.99f, 0.88f, 0.89f, 1.0f)),
              GUI::Theme::Colors::toU32(GUI::Theme::Colors::RedDanger));

    veChuGiai(u8"Bảo trì",
              GUI::Theme::Colors::toU32(ImVec4(0.92f, 0.93f, 0.94f, 1.0f)),
              GUI::Theme::Colors::toU32(GUI::Theme::Colors::Gray));

    if (choPhepChon) {
        veChuGiai(u8"Đang chọn",
                  GUI::Theme::Colors::toU32(ImVec4(1.0f, 0.96f, 0.85f, 1.0f)),
                  GUI::Theme::Colors::toU32(GUI::Theme::Colors::GoldAccent));
    }

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ==========================================
    // 2. MA TRẬN KHUNG GIỜ x MÃ SÂN (SlotGrid Table)
    // ==========================================
    if (danhSachMaSan.empty() || danhSachKhungGio.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextUnformatted(u8"Chưa có dữ liệu sân bóng hoặc khung giờ.");
        ImGui::PopStyleColor();
        ImGui::PopID();
        return coThayDoi;
    }

    int totalCols = static_cast<int>(danhSachMaSan.size()) + 1;
    ImGuiTableFlags tblFlags = ImGuiTableFlags_Borders |
                               ImGuiTableFlags_RowBg |
                               ImGuiTableFlags_SizingStretchProp |
                               ImGuiTableFlags_ScrollX;

    if (ImGui::BeginTable("##slot_table", totalCols, tblFlags)) {
        // Cột 0: Khung giờ
        ImGui::TableSetupColumn(u8"Khung giờ", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        for (const auto& maSan : danhSachMaSan) {
            ImGui::TableSetupColumn(
                maSan.c_str(),
                ImGuiTableColumnFlags_WidthStretch,
                1.0f
            );
        }
        ImGui::TableHeadersRow();

        // Từng hàng là 1 khung giờ
        for (size_t rowIdx = 0; rowIdx < danhSachKhungGio.size(); ++rowIdx) {
            const auto& khungGio = danhSachKhungGio[rowIdx];
            ImGui::TableNextRow();

            // Cột khung giờ
            ImGui::TableSetColumnIndex(0);
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
            ImGui::TextUnformatted(khungGio.c_str());
            ImGui::PopStyleColor();

            // Các cột sân bóng
            for (size_t colIdx = 0; colIdx < danhSachMaSan.size(); ++colIdx) {
                const auto& maSan = danhSachMaSan[colIdx];
                ImGui::TableSetColumnIndex(static_cast<int>(colIdx + 1));

                auto key = std::make_pair(maSan, khungGio);
                SlotInfo info;
                auto it = maTranSlot.find(key);
                if (it != maTranSlot.end()) {
                    info = it->second;
                }

                bool isSelected = (choPhepChon && maSan == maSanDangChon && khungGio == khungGioDangChon);
                SlotStatus hienThiStatus = isSelected ? SlotStatus::DangChon : info.status;

                // Xác định màu sắc theo trạng thái ô
                ImVec4 bgCol, borderCol, textCol;
                const char* statusLabel = "";

                switch (hienThiStatus) {
                    case SlotStatus::DangChon:
                        bgCol = ImVec4(1.0f, 0.96f, 0.85f, 1.0f);
                        borderCol = GUI::Theme::Colors::GoldAccent;
                        textCol = ImVec4(0.78f, 0.45f, 0.05f, 1.0f);
                        statusLabel = u8"ĐANG CHỌN";
                        break;
                    case SlotStatus::DaDat:
                        bgCol = ImVec4(0.99f, 0.88f, 0.89f, 1.0f);
                        borderCol = GUI::Theme::Colors::RedDanger;
                        textCol = GUI::Theme::Colors::RedDanger;
                        statusLabel = u8"ĐÃ ĐẶT";
                        break;
                    case SlotStatus::BaoTri:
                        bgCol = ImVec4(0.92f, 0.93f, 0.94f, 1.0f);
                        borderCol = GUI::Theme::Colors::Gray;
                        textCol = GUI::Theme::Colors::Gray;
                        statusLabel = u8"BẢO TRÌ";
                        break;
                    case SlotStatus::Trong:
                    default:
                        bgCol = ImVec4(0.88f, 0.96f, 0.91f, 1.0f);
                        borderCol = GUI::Theme::Colors::GrassGreen;
                        textCol = GUI::Theme::Colors::GrassGreen;
                        statusLabel = u8"TRỐNG";
                        break;
                }

                // Vẽ ô nút trạng thái
                std::string btnId = "##slot_" + maSan + "_" + std::to_string(rowIdx);
                ImGui::PushStyleColor(ImGuiCol_Button, bgCol);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(bgCol.x * 0.95f, bgCol.y * 0.95f, bgCol.z * 0.95f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgCol);
                ImGui::PushStyleColor(ImGuiCol_Text, textCol);
                ImGui::PushStyleColor(ImGuiCol_Border, borderCol);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

                if (ImGui::Button((std::string(statusLabel) + btnId).c_str(), ImVec2(-1.0f, 32.0f))) {
                    if (choPhepChon && info.status == SlotStatus::Trong) {
                        if (onSlotClicked) {
                            onSlotClicked(maSan, khungGio);
                        }
                        coThayDoi = true;
                    }
                }

                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(5);

                // Tooltip hiển thị thông tin chi tiết khi rê chuột (hover)
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
                    ImGui::Text(u8"Sân: %s  |  Khung giờ: %s", maSan.c_str(), khungGio.c_str());
                    ImGui::Separator();
                    if (info.status == SlotStatus::DaDat) {
                        ImGui::Text(u8"Trạng thái: Đã đặt chỗ");
                        if (!info.tenKhachHang.empty()) {
                            ImGui::Text(u8"Khách hàng: %s", info.tenKhachHang.c_str());
                        }
                        if (!info.sdt.empty()) {
                            ImGui::Text(u8"Số ĐT: %s", info.sdt.c_str());
                        }
                        if (info.giaTien > 0) {
                            ImGui::Text(u8"Giá tiền: %d VNĐ", info.giaTien);
                        }
                    } else if (info.status == SlotStatus::BaoTri) {
                        ImGui::Text(u8"Trạng thái: Sân đang tạm khóa để bảo trì kỹ thuật");
                        if (!info.ghiChu.empty()) {
                            ImGui::Text(u8"Lý do: %s", info.ghiChu.c_str());
                        }
                    } else {
                        ImGui::Text(u8"Trạng thái: Còn trống - Sẵn sàng phục vụ");
                        if (info.giaTien > 0) {
                            ImGui::Text(u8"Đơn giá niêm yết: %d VNĐ", info.giaTien);
                        }
                        if (choPhepChon) {
                            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
                            ImGui::TextUnformatted(u8"Bấm chuột để chọn đặt khung giờ này");
                            ImGui::PopStyleColor();
                        }
                    }
                    ImGui::PopStyleColor();
                    ImGui::EndTooltip();
                }
            }
        }

        ImGui::EndTable();
    }

    ImGui::PopID();
    return coThayDoi;
}

}  // namespace Widgets
