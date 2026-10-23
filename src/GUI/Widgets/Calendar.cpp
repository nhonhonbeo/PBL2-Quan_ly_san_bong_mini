#include "GUI/Widgets/Calendar.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <imgui.h>

#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Button.h"

namespace Widgets {

namespace {

bool laNamNhuan(int nam) {
    return (nam % 400 == 0) || (nam % 4 == 0 && nam % 100 != 0);
}

int soNgayTrongThang(int thang, int nam) {
    switch (thang) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            return 31;
        case 4: case 6: case 9: case 11:
            return 30;
        case 2:
            return laNamNhuan(nam) ? 29 : 28;
        default:
            return 30;
    }
}

// 0: Chủ Nhật, 1: Thứ 2, ..., 6: Thứ 7
int thuTrongTuan(int y, int m, int d) {
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3) y -= 1;
    return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

void layNgayHienTai(int& d, int& m, int& y) {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm localTm{};
#if defined(_WIN32)
    localtime_s(&localTm, &tt);
#else
    localtime_r(&tt, &localTm);
#endif
    d = localTm.tm_mday;
    m = localTm.tm_mon + 1;
    y = localTm.tm_year + 1900;
}

std::string dinhDangDMY(int d, int m, int y) {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << d << "/"
        << std::setfill('0') << std::setw(2) << m << "/"
        << std::setw(4) << y;
    return oss.str();
}

std::string dinhDangYMD(int d, int m, int y) {
    std::ostringstream oss;
    oss << std::setw(4) << y << "-"
        << std::setfill('0') << std::setw(2) << m << "-"
        << std::setfill('0') << std::setw(2) << d;
    return oss.str();
}

}  // namespace

bool Calendar(
    const char* id,
    CalendarState& state,
    const std::vector<SuKienLich>& danhSachSuKien,
    std::function<void(const std::string& ngay)> onNgayClick,
    ImVec2 kichThuoc
) {
    int curD = 1, curM = 1, curY = 2026;
    layNgayHienTai(curD, curM, curY);

    if (!state.daKhoiTao) {
        state.thang = curM;
        state.nam = curY;
        if (state.ngayDangChon.empty()) {
            state.ngayDangChon = dinhDangDMY(curD, curM, curY);
        }
        state.daKhoiTao = true;
    }

    bool coThayDoi = false;
    ImGui::PushID(id);

    ImVec2 size = kichThuoc;
    if (size.x <= 0.0f) size.x = ImGui::GetContentRegionAvail().x;
    if (size.y <= 0.0f) size.y = 360.0f;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, GUI::Theme::Colors::WhiteCard);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::BorderLight);

    if (ImGui::BeginChild("##CalendarContainer", size, true, ImGuiWindowFlags_NoScrollbar)) {
        // ==========================================
        // 1. THANH ĐIỀU HƯỚNG THÁNG / NĂM
        // ==========================================
        float availW = ImGui::GetContentRegionAvail().x;

        // Nút chuyển tháng trước
        if (NutPhu("<##prev_m", ImVec2(32.0f, 28.0f))) {
            state.thang--;
            if (state.thang < 1) {
                state.thang = 12;
                state.nam--;
            }
        }

        ImGui::SameLine();

        // Tiêu đề Tháng và Năm
        char monthTitle[64];
        std::snprintf(monthTitle, sizeof(monthTitle), u8"Tháng %02d / %d", state.thang, state.nam);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::TextUnformatted(monthTitle);
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4.0f);

        // Nút chuyển tháng sau
        if (NutPhu(">##next_m", ImVec2(32.0f, 28.0f))) {
            state.thang++;
            if (state.thang > 12) {
                state.thang = 1;
                state.nam++;
            }
        }

        ImGui::SameLine(availW - 84.0f);

        // Nút quay về Hôm nay
        if (NutPhu(u8"Hôm nay", ImVec2(80.0f, 28.0f))) {
            state.thang = curM;
            state.nam = curY;
            state.ngayDangChon = dinhDangDMY(curD, curM, curY);
            if (onNgayClick) {
                onNgayClick(state.ngayDangChon);
            }
            coThayDoi = true;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // ==========================================
        // 2. TIÊU ĐỀ 7 CỘT (THỨ 2 -> CHỦ NHẬT)
        // ==========================================
        const char* thuLabels[7] = {
            u8"Thứ 2", u8"Thứ 3", u8"Thứ 4", u8"Thứ 5", u8"Thứ 6", u8"Thứ 7", u8"Chủ Nhật"
        };

        float cellSpacing = 4.0f;
        float totalCellsW = ImGui::GetContentRegionAvail().x;
        float cellW = (totalCellsW - (6 * cellSpacing)) / 7.0f;
        float cellH = 38.0f;

        for (int col = 0; col < 7; ++col) {
            if (col > 0) ImGui::SameLine(0.0f, cellSpacing);

            // Cột Chủ Nhật tô màu đỏ nhấn mạnh ngày cuối tuần
            ImVec4 textColor = (col == 6) ? GUI::Theme::Colors::RedDanger : GUI::Theme::Colors::MutedText;

            ImVec2 curPos = ImGui::GetCursorPos();
            ImVec2 textSize = ImGui::CalcTextSize(thuLabels[col]);
            float textOffset = (cellW - textSize.x) * 0.5f;
            if (textOffset < 0.0f) textOffset = 0.0f;

            ImGui::SetCursorPosX(curPos.x + textOffset);
            ImGui::PushStyleColor(ImGuiCol_Text, textColor);
            ImGui::TextUnformatted(thuLabels[col]);
            ImGui::PopStyleColor();

            ImGui::SetCursorPos(ImVec2(curPos.x + cellW + cellSpacing, curPos.y));
        }

        ImGui::NewLine();
        ImGui::Spacing();

        // ==========================================
        // 3. LƯỚI CÁC Ô NGÀY TRONG THÁNG
        // ==========================================
        // Tính cột bắt đầu của ngày 1 (Thứ 2: col 0, ..., Chủ Nhật: col 6)
        int startCol = (thuTrongTuan(state.nam, state.thang, 1) + 6) % 7;
        int totalDays = soNgayTrongThang(state.thang, state.nam);

        // Tính ngày của tháng trước để đệm đầu lịch
        int prevM = (state.thang == 1) ? 12 : state.thang - 1;
        int prevY = (state.thang == 1) ? state.nam - 1 : state.nam;
        int prevTotalDays = soNgayTrongThang(prevM, prevY);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        int curCol = 0;

        // Vẽ các ngày tháng trước (màu xám nhạt)
        for (int i = 0; i < startCol; ++i) {
            if (curCol > 0) ImGui::SameLine(0.0f, cellSpacing);

            int dPrev = prevTotalDays - startCol + 1 + i;
            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImVec2 p1(p0.x + cellW, p0.y + cellH);

            drawList->AddRectFilled(p0, p1, IM_COL32(248, 249, 250, 160), 4.0f);

            char buf[8];
            std::snprintf(buf, sizeof(buf), "%d", dPrev);
            ImVec2 tSize = ImGui::CalcTextSize(buf);
            ImVec2 tPos(p0.x + (cellW - tSize.x) * 0.5f, p0.y + (cellH - tSize.y) * 0.5f);
            drawList->AddText(tPos, IM_COL32(180, 185, 190, 200), buf);

            ImGui::Dummy(ImVec2(cellW, cellH));
            curCol++;
        }

        // Vẽ các ngày trong tháng hiện tại
        for (int d = 1; d <= totalDays; ++d) {
            if (curCol > 0) ImGui::SameLine(0.0f, cellSpacing);

            std::string dmy = dinhDangDMY(d, state.thang, state.nam);
            std::string ymd = dinhDangYMD(d, state.thang, state.nam);

            bool isToday = (d == curD && state.thang == curM && state.nam == curY);
            bool isSelected = (dmy == state.ngayDangChon || ymd == state.ngayDangChon);

            // Tìm các sự kiện trong ngày
            std::vector<const SuKienLich*> eventsToday;
            for (const auto& sk : danhSachSuKien) {
                if (sk.ngay == dmy || sk.ngay == ymd) {
                    eventsToday.push_back(&sk);
                }
            }

            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImVec2 p1(p0.x + cellW, p0.y + cellH);

            std::string btnId = "##cal_day_" + std::to_string(d);
            bool isHovered = false;
            bool clicked = false;

            if (ImGui::InvisibleButton(btnId.c_str(), ImVec2(cellW, cellH))) {
                clicked = true;
            }
            isHovered = ImGui::IsItemHovered();

            // 1. Nền ô ngày
            ImU32 bgCol = IM_COL32(255, 255, 255, 255);
            if (isSelected) {
                bgCol = IM_COL32(27, 138, 90, 50); // Xanh cỏ nhạt nổi bật
            } else if (isHovered) {
                bgCol = IM_COL32(240, 243, 241, 255);
            }
            drawList->AddRectFilled(p0, p1, bgCol, 4.0f);

            // 2. Viền ô ngày (Hôm nay: viền xanh cỏ 2px)
            if (isToday) {
                drawList->AddRect(p0, p1, IM_COL32(27, 138, 90, 255), 4.0f, 0, 2.0f);
            } else if (isSelected) {
                drawList->AddRect(p0, p1, IM_COL32(27, 138, 90, 200), 4.0f, 0, 1.5f);
            } else {
                drawList->AddRect(p0, p1, IM_COL32(230, 235, 232, 200), 4.0f, 0, 1.0f);
            }

            // 3. Số ngày
            char buf[8];
            std::snprintf(buf, sizeof(buf), "%d", d);
            ImVec2 tSize = ImGui::CalcTextSize(buf);
            float tPosY = eventsToday.empty() ? (p0.y + (cellH - tSize.y) * 0.5f) : (p0.y + 6.0f);
            ImVec2 tPos(p0.x + (cellW - tSize.x) * 0.5f, tPosY);

            ImU32 textCol = isSelected ? IM_COL32(27, 138, 90, 255) : IM_COL32(31, 41, 55, 255);
            drawList->AddText(tPos, textCol, buf);

            // 4. Chấm màu sự kiện (vẽ tối đa 3 chấm nhỏ bên dưới số ngày)
            if (!eventsToday.empty()) {
                size_t dotCount = eventsToday.size();
                if (dotCount > 3) dotCount = 3;

                float dotRadius = 3.0f;
                float dotSpacing = 8.0f;
                float startDotX = p0.x + (cellW - (static_cast<float>(dotCount - 1) * dotSpacing)) * 0.5f;
                float dotY = p1.y - 7.0f;

                for (size_t k = 0; k < dotCount; ++k) {
                    ImU32 dotColor = eventsToday[k]->mau != 0 ? eventsToday[k]->mau : IM_COL32(245, 179, 1, 255); // Vàng/Xanh
                    drawList->AddCircleFilled(ImVec2(startDotX + static_cast<float>(k) * dotSpacing, dotY), dotRadius, dotColor);
                }

                // Tooltip hiển thị danh sách sự kiện khi rê chuột
                if (isHovered) {
                    ImGui::BeginTooltip();
                    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
                    ImGui::Text(u8"Sự kiện ngày %02d/%02d/%d:", d, state.thang, state.nam);
                    ImGui::PopStyleColor();
                    ImGui::Separator();
                    for (const auto* ev : eventsToday) {
                        ImGui::BulletText(u8"%s", ev->nhan.c_str());
                    }
                    ImGui::EndTooltip();
                }
            }

            // Xử lý khi click vào ô ngày
            if (clicked) {
                state.ngayDangChon = dmy;
                if (onNgayClick) {
                    onNgayClick(dmy);
                }
                coThayDoi = true;
            }

            curCol++;
            if (curCol == 7) {
                curCol = 0;
            }
        }

        // Vẽ các ngày đầu tháng sau để lấp đầy hàng cuối nếu cần
        if (curCol > 0) {
            int nextDays = 7 - curCol;
            for (int k = 1; k <= nextDays; ++k) {
                ImGui::SameLine(0.0f, cellSpacing);

                ImVec2 p0 = ImGui::GetCursorScreenPos();
                ImVec2 p1(p0.x + cellW, p0.y + cellH);

                drawList->AddRectFilled(p0, p1, IM_COL32(248, 249, 250, 160), 4.0f);

                char buf[8];
                std::snprintf(buf, sizeof(buf), "%d", k);
                ImVec2 tSize = ImGui::CalcTextSize(buf);
                ImVec2 tPos(p0.x + (cellW - tSize.x) * 0.5f, p0.y + (cellH - tSize.y) * 0.5f);
                drawList->AddText(tPos, IM_COL32(180, 185, 190, 200), buf);

                ImGui::Dummy(ImVec2(cellW, cellH));
            }
        }
    }
    ImGui::EndChild();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
    ImGui::PopID();

    return coThayDoi;
}

}  // namespace Widgets
