#include "GUI/Widgets/TimePicker.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <imgui.h>

#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/InputFields.h"

namespace Widgets {

namespace {

bool tachGioPhut(const std::string& str, int& h, int& m) {
    if (str.length() == 5 && str[2] == ':') {
        try {
            h = std::stoi(str.substr(0, 2));
            m = std::stoi(str.substr(3, 2));
            return (h >= 0 && h <= 23 && m >= 0 && m <= 59);
        } catch (...) {
            return false;
        }
    }
    return false;
}

std::string dinhDangGioPhut(int h, int m) {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << h << ":"
        << std::setfill('0') << std::setw(2) << m;
    return oss.str();
}

}  // namespace

bool TimePicker(
    const char* label,
    const char* id,
    std::string& timeStr,
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

    float nutGioWidth = 76.0f;
    float availWidth = ImGui::GetContentRegionAvail().x;
    float timeInputWidth = std::max(
        1.0f,
        availWidth - nutGioWidth - ImGui::GetStyle().ItemSpacing.x
    );
    bool changed = InputText(
        nullptr,
        "##time_txt_id",
        timeStr,
        nullptr,
        u8"HH:mm (ví dụ 17:30)",
        timeInputWidth
    );

    if (coLoi) {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    // Nút mở popup chọn giờ
    ImGui::PushStyleColor(ImGuiCol_Button, GUI::Theme::Colors::WhiteCard);
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
    ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::InputBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    if (ImGui::Button(u8"Giờ", ImVec2(nutGioWidth, 0.0f))) {
        ImGui::OpenPopup("##TimePickerPopup");
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    // Lấy giờ và phút hiện tại hoặc từ chuỗi
    int curH = 17, curM = 0;
    tachGioPhut(timeStr, curH, curM);

    ImGuiStorage* storage = ImGui::GetStateStorage();
    ImGuiID hourId = ImGui::GetID("tp_hour");
    ImGuiID minId  = ImGui::GetID("tp_min");

    int selH = storage->GetInt(hourId, curH);
    int selM = storage->GetInt(minId, curM);

    if (ImGui::BeginPopup("##TimePickerPopup")) {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::TextUnformatted(u8"Chọn khung giờ:");
        ImGui::Separator();
        ImGui::Spacing();

        // Spin/Slider chỉnh Giờ và Phút
        ImGui::TextUnformatted(u8"Giờ (0 - 23):");
        ImGui::SameLine(130.0f);
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::InputInt("##spin_h", &selH, 1, 5)) {
            if (selH < 0) selH = 23;
            if (selH > 23) selH = 0;
            storage->SetInt(hourId, selH);
        }

        ImGui::TextUnformatted(u8"Phút (0 - 59):");
        ImGui::SameLine(130.0f);
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::InputInt("##spin_m", &selM, 5, 15)) {
            if (selM < 0) selM = 55;
            if (selM > 59) selM = 0;
            storage->SetInt(minId, selM);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Các khung giờ phổ biến đặt sân bóng
        ImGui::TextUnformatted(u8"Khung giờ phổ biến:");
        const char* khungGioPhoBien[] = {"06:00", "07:30", "16:00", "17:30", "19:00", "20:30"};
        for (int i = 0; i < 6; ++i) {
            if (i % 3 != 0) ImGui::SameLine();
            if (ImGui::Button(khungGioPhoBien[i], ImVec2(76.0f, 26.0f))) {
                timeStr = khungGioPhoBien[i];
                changed = true;
                tachGioPhut(timeStr, selH, selM);
                storage->SetInt(hourId, selH);
                storage->SetInt(minId, selM);
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (NutChinh(u8"Xác nhận", ImVec2(120.0f, 0.0f))) {
            timeStr = dinhDangGioPhut(selH, selM);
            changed = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (NutPhu(u8"Đóng", ImVec2(120.0f, 0.0f))) {
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
