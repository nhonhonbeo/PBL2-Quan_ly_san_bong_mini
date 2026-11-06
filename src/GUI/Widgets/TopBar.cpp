#include "GUI/Widgets/TopBar.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <imgui.h>

#include "GUI/Theme/Fonts.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"

namespace Widgets {

namespace {

std::string layGioVaNgayHeThong() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm localTm{};
#if defined(_WIN32)
    localtime_s(&localTm, &tt);
#else
    localtime_r(&tt, &localTm);
#endif
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << localTm.tm_hour << ":"
        << std::setfill('0') << std::setw(2) << localTm.tm_min << ":"
        << std::setfill('0') << std::setw(2) << localTm.tm_sec << "  "
        << std::setfill('0') << std::setw(2) << localTm.tm_mday << "/"
        << std::setfill('0') << std::setw(2) << (localTm.tm_mon + 1) << "/"
        << (localTm.tm_year + 1900);
    return oss.str();
}

}  // namespace

void TopBar(
    const char* tieuDeManHinh,
    const std::vector<std::string>& breadcrumbs,
    const char* tenNguoiDung,
    const char* vaiTro,
    float chieuCao
) {
    ImVec2 startPos = ImGui::GetCursorScreenPos();
    float totalWidth = ImGui::GetContentRegionAvail().x;
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Nền trắng và đường viền đáy tinh tế
    ImVec2 barMin = startPos;
    ImVec2 barMax = ImVec2(startPos.x + totalWidth, startPos.y + chieuCao);

    drawList->AddRectFilled(barMin, barMax, GUI::Theme::Colors::toU32(GUI::Theme::Colors::Surface));
    drawList->AddLine(
        ImVec2(barMin.x, barMax.y),
        ImVec2(barMax.x, barMax.y),
        GUI::Theme::Colors::toU32(GUI::Theme::Colors::Border),
        1.0f
    );

    ImGui::SetCursorScreenPos(ImVec2(startPos.x + 20.0f, startPos.y + 10.0f));
    ImGui::BeginGroup();

    // ==========================================
    // 1. BREADCRUMB & TIÊU ĐỀ BÊN TRÁI
    // ==========================================
    if (!breadcrumbs.empty()) {
        ImFont* smallFont = GUI::Theme::getFonts().small;
        if (smallFont != nullptr) ImGui::PushFont(smallFont);

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
        for (size_t i = 0; i < breadcrumbs.size(); ++i) {
            if (i > 0) {
                ImGui::SameLine();
                ImGui::TextUnformatted(" › ");
                ImGui::SameLine();
            }
            ImGui::TextUnformatted(breadcrumbs[i].c_str());
        }
        ImGui::PopStyleColor();

        if (smallFont != nullptr) ImGui::PopFont();
    }

    if (tieuDeManHinh != nullptr && tieuDeManHinh[0] != '\0') {
        ImFont* titleFont = GUI::Theme::getFonts().title;
        if (titleFont != nullptr) ImGui::PushFont(titleFont);

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
        ImGui::TextUnformatted(tieuDeManHinh);
        ImGui::PopStyleColor();

        if (titleFont != nullptr) ImGui::PopFont();
    }

    ImGui::EndGroup();

    // ==========================================
    // 2. ĐỒNG HỒ HỆ THỐNG & USER CHIP BÊN PHẢI
    // ==========================================
    std::string timeStr = layGioVaNgayHeThong();
    std::string userStr = tenNguoiDung ? tenNguoiDung : u8"Người dùng";
    std::string roleStr = vaiTro ? vaiTro : "";

    ImVec2 timeSize = ImGui::CalcTextSize(timeStr.c_str());
    ImVec2 userSize = ImGui::CalcTextSize(userStr.c_str());
    ImVec2 roleSize = ImGui::CalcTextSize(roleStr.c_str());
    float rightBlockWidth = timeSize.x + userSize.x + roleSize.x + 110.0f;

    float rightPosX = barMax.x - rightBlockWidth - 20.0f;
    ImGui::SetCursorScreenPos(ImVec2(rightPosX, startPos.y + 14.0f));
    ImGui::BeginGroup();

    // Đồng hồ hệ thống dạng chip nhẹ
    ImFont* smallFont = GUI::Theme::getFonts().small;
    if (smallFont != nullptr) ImGui::PushFont(smallFont);

    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
    ImGui::Text("%s", timeStr.c_str());
    ImGui::PopStyleColor();

    if (smallFont != nullptr) ImGui::PopFont();

    ImGui::SameLine(0.0f, 20.0f);

    // Chấm xanh trạng thái trực tuyến
    ImVec2 curPos = ImGui::GetCursorScreenPos();
    drawList->AddCircleFilled(ImVec2(curPos.x + 3.0f, curPos.y + 10.0f), 4.0f, GUI::Theme::Colors::toU32(GUI::Theme::Colors::Success));
    ImGui::Dummy(ImVec2(10.0f, 20.0f));
    ImGui::SameLine(0.0f, 6.0f);

    // Avatar tròn
    ImVec2 avatarPos = ImGui::GetCursorScreenPos();
    float avatarR = 14.0f;
    ImVec2 avatarCenter(avatarPos.x + avatarR, avatarPos.y + avatarR - 3.0f);
    drawList->AddCircleFilled(avatarCenter, avatarR, GUI::Theme::Colors::toU32(GUI::Theme::Colors::Primary));

    char initial[2] = {tenNguoiDung && tenNguoiDung[0] ? tenNguoiDung[0] : 'U', '\0'};
    ImVec2 initSize = ImGui::CalcTextSize(initial);
    drawList->AddText(
        ImVec2(avatarCenter.x - initSize.x * 0.5f, avatarCenter.y - initSize.y * 0.5f),
        IM_COL32_WHITE,
        initial
    );

    ImGui::Dummy(ImVec2(avatarR * 2.0f + 6.0f, avatarR * 2.0f));
    ImGui::SameLine();

    // Tên người dùng
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
    ImGui::TextUnformatted(userStr.c_str());
    ImGui::PopStyleColor();

    // Pill badge vai trò
    if (!roleStr.empty()) {
        ImGui::SameLine(0.0f, 8.0f);
        if (roleStr == u8"Chủ sân") {
            Badge(roleStr.c_str(), GUI::Theme::Colors::PrimaryLight, GUI::Theme::Colors::Primary);
        } else {
            Badge(roleStr.c_str(), ImVec4(0.910f, 0.949f, 0.980f, 1.0f), ImVec4(0.145f, 0.388f, 0.922f, 1.0f));
        }
    }

    ImGui::EndGroup();

    // Đẩy con trỏ xuống dưới TopBar kèm khoảng cách 16px
    ImGui::SetCursorScreenPos(ImVec2(startPos.x, barMax.y + 16.0f));
}

}  // namespace Widgets
