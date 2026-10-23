#include "GUI/Widgets/DatePicker.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <string>
#include <imgui.h>

#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/InputFields.h"

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

bool tachNgayThangNam(const std::string& str, int& d, int& m, int& y) {
    if (str.length() == 10 && str[2] == '/' && str[5] == '/') {
        try {
            d = std::stoi(str.substr(0, 2));
            m = std::stoi(str.substr(3, 2));
            y = std::stoi(str.substr(6, 4));
            return (m >= 1 && m <= 12 && d >= 1 && d <= soNgayTrongThang(m, y) && y >= 1900 && y <= 2100);
        } catch (...) {
            return false;
        }
    }
    return false;
}

std::string dinhDangNgay(int d, int m, int y) {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << d << "/"
        << std::setfill('0') << std::setw(2) << m << "/"
        << std::setw(4) << y;
    return oss.str();
}

}  // namespace

bool DatePicker(
    const char* label,
    const char* id,
    std::string& dateStr,
    const char* loi
) {
    ImGui::PushID(id);

    if (label != nullptr && label[0] != '\0') {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::TextUnformatted(label);
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    bool coLoi = (loi != nullptr && loi[0] != '\0');
    if (coLoi) {
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::RedDanger);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
    }

    float nutLichWidth = 76.0f;
    float availWidth = ImGui::GetContentRegionAvail().x;
    float dateInputWidth = std::max(
        1.0f,
        availWidth - nutLichWidth - ImGui::GetStyle().ItemSpacing.x
    );
    bool changed = InputText(
        nullptr,
        "##date_txt_id",
        dateStr,
        nullptr,
        u8"dd/MM/yyyy",
        dateInputWidth
    );

    if (coLoi) {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    // Nút mở lịch popup
    ImGui::PushStyleColor(ImGuiCol_Button, GUI::Theme::Colors::WhiteCard);
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
    ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::InputBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    if (ImGui::Button(u8"Lịch", ImVec2(nutLichWidth, 0.0f))) {
        ImGui::OpenPopup("##DatePickerPopup");
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    // Lấy ngày hiện tại
    int todayD = 0, todayM = 0, todayY = 0;
    layNgayHienTai(todayD, todayM, todayY);

    // Lấy trạng thái hiển thị của popup lịch từ ImGui Storage
    ImGuiStorage* storage = ImGui::GetStateStorage();
    ImGuiID viewMonthId = ImGui::GetID("view_month");
    ImGuiID viewYearId  = ImGui::GetID("view_year");

    int viewMonth = storage->GetInt(viewMonthId, 0);
    int viewYear  = storage->GetInt(viewYearId, 0);

    if (viewMonth == 0 || viewYear == 0) {
        int curD = 0, curM = 0, curY = 0;
        if (tachNgayThangNam(dateStr, curD, curM, curY)) {
            viewMonth = curM;
            viewYear  = curY;
        } else {
            viewMonth = todayM;
            viewYear  = todayY;
        }
        storage->SetInt(viewMonthId, viewMonth);
        storage->SetInt(viewYearId, viewYear);
    }

    // Popup tự vẽ lịch tháng
    if (ImGui::BeginPopup("##DatePickerPopup")) {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);

        // Thanh điều hướng: << < [Tháng MM / YYYY] > >>
        if (ImGui::Button("<<")) {
            viewYear--;
            storage->SetInt(viewYearId, viewYear);
        }
        ImGui::SameLine();
        if (ImGui::Button("<")) {
            viewMonth--;
            if (viewMonth < 1) {
                viewMonth = 12;
                viewYear--;
            }
            storage->SetInt(viewMonthId, viewMonth);
            storage->SetInt(viewYearId, viewYear);
        }
        ImGui::SameLine();

        std::string titleHeader = std::string(u8"Tháng ") + (viewMonth < 10 ? "0" : "") + std::to_string(viewMonth) +
                                  " / " + std::to_string(viewYear);
        float titleWidth = 130.0f;
        float currentX = ImGui::GetCursorPosX();
        ImGui::SetCursorPosX(currentX + (titleWidth - ImGui::CalcTextSize(titleHeader.c_str()).x) * 0.5f);
        ImGui::TextUnformatted(titleHeader.c_str());
        ImGui::SameLine();
        ImGui::SetCursorPosX(currentX + titleWidth);

        if (ImGui::Button(">")) {
            viewMonth++;
            if (viewMonth > 12) {
                viewMonth = 1;
                viewYear++;
            }
            storage->SetInt(viewMonthId, viewMonth);
            storage->SetInt(viewYearId, viewYear);
        }
        ImGui::SameLine();
        if (ImGui::Button(">>")) {
            viewYear++;
            storage->SetInt(viewYearId, viewYear);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Tiêu đề các thứ trong tuần (T2 đến CN)
        const char* thuHeaders[] = {"T2", "T3", "T4", "T5", "T6", "T7", "CN"};
        ImVec2 cellSize(32.0f, 26.0f);

        for (int i = 0; i < 7; ++i) {
            if (i > 0) ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            float offset = (cellSize.x - ImGui::CalcTextSize(thuHeaders[i]).x) * 0.5f;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
            ImGui::TextUnformatted(thuHeaders[i]);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (cellSize.x - ImGui::CalcTextSize(thuHeaders[i]).x - offset));
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();

        // Lưới ngày trong tháng
        int firstDow = thuTrongTuan(viewYear, viewMonth, 1);
        int startCol = (firstDow + 6) % 7; // Chuyển để T2 là 0, CN là 6
        int totalDays = soNgayTrongThang(viewMonth, viewYear);

        int selectedD = 0, selectedM = 0, selectedY = 0;
        tachNgayThangNam(dateStr, selectedD, selectedM, selectedY);

        int currentDay = 1;
        int col = 0;

        // Vẽ các ô trống trước ngày mùng 1
        for (; col < startCol; ++col) {
            if (col > 0) ImGui::SameLine();
            ImGui::InvisibleButton("##empty", cellSize);
        }

        // Vẽ các ngày
        while (currentDay <= totalDays) {
            if (col > 0) ImGui::SameLine();

            bool isSelected = (currentDay == selectedD && viewMonth == selectedM && viewYear == selectedY);
            bool isToday = (currentDay == todayD && viewMonth == todayM && viewYear == todayY);

            std::string dayLabel = std::to_string(currentDay);
            std::string btnId = "##day_" + dayLabel;

            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, GUI::Theme::Colors::GrassGreen);
                ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::WhiteCard);
            } else if (isToday) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.90f, 0.96f, 0.92f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, GUI::Theme::Colors::WhiteCard);
                ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
            }

            if (ImGui::Button((dayLabel + btnId).c_str(), cellSize)) {
                dateStr = dinhDangNgay(currentDay, viewMonth, viewYear);
                changed = true;
                ImGui::CloseCurrentPopup();
            }

            ImGui::PopStyleColor(2);

            currentDay++;
            col++;
            if (col == 7 && currentDay <= totalDays) {
                col = 0;
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Nút chọn nhanh "Hôm nay" và "Đóng"
        if (ImGui::Button(u8"Hôm nay", ImVec2(110.0f, 0.0f))) {
            dateStr = dinhDangNgay(todayD, todayM, todayY);
            viewMonth = todayM;
            viewYear  = todayY;
            storage->SetInt(viewMonthId, viewMonth);
            storage->SetInt(viewYearId, viewYear);
            changed = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(u8"Đóng", ImVec2(110.0f, 0.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleColor();
        ImGui::EndPopup();
    }

    if (coLoi) {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
        ImGui::TextUnformatted(loi);
        ImGui::PopStyleColor();
    }

    ImGui::PopID();
    return changed;
}

}  // namespace Widgets
