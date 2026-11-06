#include "GUI/Widgets/Toast.h"

#include <algorithm>
#include <vector>
#include <imgui.h>

#include "GUI/Theme/Fonts.h"
#include "GUI/Theme/Theme.h"

namespace Widgets {

namespace {

struct ToastItem {
    std::string noiDung;
    LoaiToast loai;
    float thoiGianConLai;
    float tongThoiGian;
};

std::vector<ToastItem> s_Toasts;

}  // namespace

void ThemToast(const std::string& noiDung, LoaiToast loai, float thoiGian) {
    s_Toasts.push_back({noiDung, loai, thoiGian, thoiGian});
}

void ToastThanhCong(const std::string& noiDung) {
    ThemToast(noiDung, LoaiToast::ThanhCong, 3.0f);
}

void ToastLoi(const std::string& noiDung) {
    ThemToast(noiDung, LoaiToast::Loi, 3.0f);
}

void ToastCanhBao(const std::string& noiDung) {
    ThemToast(noiDung, LoaiToast::CanhBao, 3.0f);
}

void ToastThongTin(const std::string& noiDung) {
    ThemToast(noiDung, LoaiToast::ThongTin, 3.0f);
}

void RenderToasts() {
    if (s_Toasts.empty()) {
        return;
    }

    float dt = ImGui::GetIO().DeltaTime;
    for (auto& t : s_Toasts) {
        t.thoiGianConLai -= dt;
    }

    s_Toasts.erase(
        std::remove_if(s_Toasts.begin(), s_Toasts.end(), [](const ToastItem& t) {
            return t.thoiGianConLai <= 0.0f;
        }),
        s_Toasts.end()
    );

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    float toastWidth = 340.0f;
    float posX = viewport->WorkPos.x + viewport->WorkSize.x - toastWidth - 24.0f;
    float posY = viewport->WorkPos.y + 24.0f;

    for (size_t i = 0; i < s_Toasts.size(); ++i) {
        const auto& t = s_Toasts[i];

        float alpha = 1.0f;
        if (t.thoiGianConLai < 0.6f) {
            alpha = t.thoiGianConLai / 0.6f;
        }
        if (alpha < 0.05f) alpha = 0.05f;

        ImGui::SetNextWindowPos(ImVec2(posX, posY), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(toastWidth, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.98f * alpha);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoSavedSettings |
                                 ImGuiWindowFlags_NoFocusOnAppearing |
                                 ImGuiWindowFlags_NoNav |
                                 ImGuiWindowFlags_NoMove;

        std::string windowId = "##Toast_" + std::to_string(i);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 12.0f));

        if (ImGui::Begin(windowId.c_str(), nullptr, flags)) {
            ImVec4 themeColor;
            const char* headerText = "";

            switch (t.loai) {
                case LoaiToast::ThanhCong:
                    themeColor = GUI::Theme::Colors::Success;
                    headerText = u8"✓ Thành công";
                    break;
                case LoaiToast::Loi:
                    themeColor = GUI::Theme::Colors::Danger;
                    headerText = u8"✕ Lỗi xử lý";
                    break;
                case LoaiToast::CanhBao:
                    themeColor = GUI::Theme::Colors::Warning;
                    headerText = u8"⚠ Cảnh báo";
                    break;
                case LoaiToast::ThongTin:
                default:
                    themeColor = GUI::Theme::Colors::Info;
                    headerText = u8"ℹ Thông báo";
                    break;
            }

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 winMin = ImGui::GetWindowPos();
            ImVec2 winMax = ImVec2(winMin.x + ImGui::GetWindowWidth(), winMin.y + ImGui::GetWindowHeight());
            ImU32 barCol = GUI::Theme::Colors::toU32(themeColor, alpha);
            drawList->AddRectFilled(winMin, ImVec2(winMin.x + 5.0f, winMax.y), barCol, 8.0f, ImDrawFlags_RoundCornersLeft);

            ImGui::PushStyleColor(ImGuiCol_Text, themeColor);
            ImGui::TextUnformatted(headerText);
            ImGui::PopStyleColor();

            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
            ImGui::PushTextWrapPos(toastWidth - 14.0f);
            ImGui::TextUnformatted(t.noiDung.c_str());
            ImGui::PopTextWrapPos();
            ImGui::PopStyleColor();

            posY += ImGui::GetWindowHeight() + 10.0f;
            ImGui::End();
        }

        ImGui::PopStyleVar(3);
    }
}

}  // namespace Widgets
