#include "GUI/Widgets/TopBar.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <imgui.h>

#include "GUI/Theme/Theme.h"

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

    // Nền trắng và đường viền đáy
    ImVec2 barMin = startPos;
    ImVec2 barMax = ImVec2(startPos.x + totalWidth, startPos.y + chieuCao);

    drawList->AddRectFilled(barMin, barMax, GUI::Theme::Colors::toU32(GUI::Theme::Colors::WhiteCard));
    drawList->AddLine(
        ImVec2(barMin.x, barMax.y),
        ImVec2(barMax.x, barMax.y),
        GUI::Theme::Colors::toU32(GUI::Theme::Colors::BorderLight),
        1.0f
    );

    ImGui::SetCursorScreenPos(ImVec2(startPos.x + 18.0f, startPos.y + 10.0f));
    ImGui::BeginGroup();

    // ==========================================
    // 1. TIÊU ĐỀ & BREADCRUMB BÊN TRÁI
    // ==========================================
    if (!breadcrumbs.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        for (size_t i = 0; i < breadcrumbs.size(); ++i) {
            if (i > 0) {
                ImGui::SameLine();
                ImGui::TextUnformatted(" > ");
                ImGui::SameLine();
            }
            ImGui::TextUnformatted(breadcrumbs[i].c_str());
        }
        ImGui::PopStyleColor();
    }

    if (tieuDeManHinh != nullptr && tieuDeManHinh[0] != '\0') {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::TextUnformatted(tieuDeManHinh);
        ImGui::PopStyleColor();
    }

    ImGui::EndGroup();

    // ==========================================
    // 2. ĐỒNG HỒ HỆ THỐNG & TÀI KHOẢN BÊN PHẢI
    // ==========================================
    std::string timeStr = layGioVaNgayHeThong();
    std::string userStr = std::string(tenNguoiDung) + " (" + vaiTro + ")";

    ImVec2 timeSize = ImGui::CalcTextSize(timeStr.c_str());
    ImVec2 userSize = ImGui::CalcTextSize(userStr.c_str());
    float rightBlockWidth = timeSize.x + userSize.x + 70.0f;

    float rightPosX = barMax.x - rightBlockWidth - 18.0f;
    ImGui::SetCursorScreenPos(ImVec2(rightPosX, startPos.y + 12.0f));
    ImGui::BeginGroup();

    // Đồng hồ hệ thống
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
    ImGui::TextUnformatted(timeStr.c_str());
    ImGui::PopStyleColor();

    ImGui::SameLine(0.0f, 20.0f);

    // Avatar tròn biểu tượng chữ cái đầu
    ImVec2 avatarPos = ImGui::GetCursorScreenPos();
    float avatarR = 14.0f;
    ImVec2 avatarCenter(avatarPos.x + avatarR, avatarPos.y + avatarR - 2.0f);
    drawList->AddCircleFilled(avatarCenter, avatarR, GUI::Theme::Colors::toU32(GUI::Theme::Colors::GrassGreen));

    char initial[2] = {tenNguoiDung && tenNguoiDung[0] ? tenNguoiDung[0] : 'U', '\0'};
    ImVec2 initSize = ImGui::CalcTextSize(initial);
    drawList->AddText(
        ImVec2(avatarCenter.x - initSize.x * 0.5f, avatarCenter.y - initSize.y * 0.5f),
        IM_COL32_WHITE,
        initial
    );

    ImGui::Dummy(ImVec2(avatarR * 2.0f + 6.0f, avatarR * 2.0f));
    ImGui::SameLine();

    // Tên và vai trò người dùng
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
    ImGui::TextUnformatted(userStr.c_str());
    ImGui::PopStyleColor();

    ImGui::EndGroup();

    // Đẩy con trỏ xuống dưới TopBar kèm khoảng cách 14px
    ImGui::SetCursorScreenPos(ImVec2(startPos.x, barMax.y + 14.0f));
}

}  // namespace Widgets
