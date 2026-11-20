#include "GUI/Widgets/InputFields.h"

#include <cctype>
#include <algorithm>
#include <vector>
#include <imgui.h>
#include "GUI/Theme/Fonts.h"
#include "GUI/Theme/Theme.h"

namespace Widgets {

namespace {

struct InputTextCallback_UserData {
    std::string* Str;
    ImGuiInputTextCallback ChainCallback;
    void* ChainCallbackUserData;
    int MaxLength;
};

static int InputTextCallback(ImGuiInputTextCallbackData* data) {
    auto* user_data = static_cast<InputTextCallback_UserData*>(data->UserData);
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        std::string* str = user_data->Str;
        str->resize(data->BufTextLen);
        data->Buf = str->data();
    } else if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
        if (user_data->ChainCallback) {
            return user_data->ChainCallback(data);
        }
    }
    return 0;
}

bool InputTextStdString(
    const char* label,
    const char* hint,
    std::string* str,
    ImGuiInputTextFlags flags = 0,
    ImGuiInputTextCallback callback = nullptr,
    void* user_data = nullptr,
    int maxLength = 0
) {
    flags |= ImGuiInputTextFlags_CallbackResize;
    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    cb_user_data.MaxLength = maxLength;

    char emptyBuf[1] = "";
    char* buf = str->empty() ? emptyBuf : str->data();
    size_t buf_size = str->capacity() + 1;
    if (str->empty()) {
        buf_size = 1;
    }

    if (hint != nullptr && hint[0] != '\0') {
        return ImGui::InputTextWithHint(label, hint, buf, buf_size, flags, InputTextCallback, &cb_user_data);
    }
    return ImGui::InputText(label, buf, buf_size, flags, InputTextCallback, &cb_user_data);
}

void HienThiNhan(const char* label) {
    if (label != nullptr && label[0] != '\0') {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
        if (GUI::Theme::getFonts().label) ImGui::PushFont(GUI::Theme::getFonts().label);
        ImGui::TextUnformatted(label);
        if (GUI::Theme::getFonts().label) ImGui::PopFont();
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }
}

void HienThiLoi(const char* loi) {
    if (loi != nullptr && loi[0] != '\0') {
        ImFont* smallFont = GUI::Theme::getFonts().small;
        if (smallFont != nullptr) ImGui::PushFont(smallFont);

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Danger);
        ImGui::TextWrapped("%s", loi);
        ImGui::PopStyleColor();

        if (smallFont != nullptr) ImGui::PopFont();
    }
}

}  // namespace

bool InputText(
    const char* label,
    const char* id,
    std::string& buffer,
    const char* loi,
    const char* placeholder,
    float width
) {
    ImGui::PushID(id);
    HienThiNhan(label);

    bool coLoi = (loi != nullptr && loi[0] != '\0');
    if (coLoi) {
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::Danger);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
    }

    ImGui::SetNextItemWidth(width);
    bool changed = InputTextStdString("##input", placeholder, &buffer);

    if (coLoi) {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    HienThiLoi(loi);
    ImGui::PopID();
    return changed;
}

bool InputTextValidated(
    const char* label,
    const char* id,
    std::string& buffer,
    const char* placeholder,
    bool& daCham,
    bool batBuocKiemTra,
    InputTextValidator validator,
    InputTextSanitizer sanitizer,
    float width
) {
    ImGui::PushID(id);
    HienThiNhan(label);
    ImGui::SetNextItemWidth(width);
    bool changed = InputTextStdString("##input", placeholder, &buffer);
    const bool activated = ImGui::IsItemActivated();
    const bool deactivated = ImGui::IsItemDeactivated();
    daCham = daCham || activated || deactivated;

    if (sanitizer != nullptr) {
        const std::string bufferTruocChuanHoa = buffer;
        sanitizer(buffer);
        changed = changed || buffer != bufferTruocChuanHoa;
    }

    const std::string loi = (daCham || batBuocKiemTra) && validator != nullptr
        ? validator(buffer)
        : "";
    if (!loi.empty()) {
        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 max = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRect(
            min,
            max,
            ImGui::GetColorU32(GUI::Theme::Colors::Danger),
            ImGui::GetStyle().FrameRounding,
            0,
            1.5f
        );
        HienThiLoi(loi.c_str());
    }

    ImGui::PopID();
    return changed;
}

bool InputPassword(
    const char* label,
    const char* id,
    std::string& buffer,
    bool& hienMatKhau,
    const char* loi,
    const char* placeholder
) {
    ImGui::PushID(id);
    HienThiNhan(label);

    bool coLoi = (loi != nullptr && loi[0] != '\0');
    if (coLoi) {
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::Danger);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
    }

    float nutMatWidth = 42.0f;
    float availWidth = std::min(480.0f, ImGui::GetContentRegionAvail().x);
    ImGui::SetNextItemWidth(std::max(40.0f, availWidth - nutMatWidth - ImGui::GetStyle().ItemSpacing.x));

    ImGuiInputTextFlags flags = hienMatKhau ? 0 : ImGuiInputTextFlags_Password;
    bool changed = InputTextStdString("##pw_input", placeholder, &buffer, flags);

    if (coLoi) {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    // LÝ DO CHỌN: Nút hiện/ẩn mật khẩu được tinh chỉnh giao diện nhỏ gọn, bo góc mềm mại
    const char* iconText = "##toggle_password";
    ImGui::PushStyleColor(ImGuiCol_Button, GUI::Theme::Colors::Surface);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::GrayHover);
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Primary);
    ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::InputBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    if (ImGui::Button(iconText, ImVec2(nutMatWidth, 0.0f))) {
        hienMatKhau = !hienMatKhau;
    }
    const ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
    const ImVec2 c((a.x+b.x)*.5f,(a.y+b.y)*.5f);
    auto* dl = ImGui::GetWindowDrawList();
    const ImU32 ink = GUI::Theme::Colors::toU32(GUI::Theme::Colors::TextSecondary);
    dl->AddEllipse(c, ImVec2(9,5), ink, 0, 20, 1.5f);
    dl->AddCircle(c, 2.5f, ink, 12, 1.4f);
    if (!hienMatKhau) dl->AddLine(ImVec2(c.x-8,c.y+7),ImVec2(c.x+8,c.y-7),ink,1.5f);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", hienMatKhau?u8"Ẩn mật khẩu":u8"Hiện mật khẩu");
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);

    HienThiLoi(loi);
    ImGui::PopID();
    return changed;
}

bool InputInt(
    const char* label,
    const char* id,
    int& giaTri,
    const char* loi,
    int step,
    int stepFast
) {
    ImGui::PushID(id);
    HienThiNhan(label);

    bool coLoi = (loi != nullptr && loi[0] != '\0');
    if (coLoi) {
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::Danger);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
    }

    const ImGuiStyle& style = ImGui::GetStyle();
    const std::string currentValue = std::to_string(giaTri);
    const float stepButtonsWidth =
        2.0f * (ImGui::GetFrameHeight() + style.ItemInnerSpacing.x);
    const float inputTextWidth = std::max(
        1.0f,
        ImGui::GetContentRegionAvail().x - stepButtonsWidth
    );
    const float centeredPadding = std::max(
        style.FramePadding.x,
        (inputTextWidth - ImGui::CalcTextSize(currentValue.c_str()).x) * 0.5f
    );
    ImGui::PushStyleVar(
        ImGuiStyleVar_FramePadding,
        ImVec2(centeredPadding, style.FramePadding.y)
    );
    ImGui::SetNextItemWidth(-1.0f);
    bool changed = ImGui::InputInt("##int_input", &giaTri, step, stepFast);
    ImGui::PopStyleVar();

    if (coLoi) {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    HienThiLoi(loi);
    ImGui::PopID();
    return changed;
}

static int FilterPhoneCallback(ImGuiInputTextCallbackData* data) {
    if (data->EventChar < '0' || data->EventChar > '9') {
        return 1;
    }
    auto* user_data = static_cast<InputTextCallback_UserData*>(data->UserData);
    if (user_data && user_data->Str && user_data->Str->length() >= 10) {
        return 1;
    }
    return 0;
}

bool InputPhone(
    const char* label,
    const char* id,
    std::string& buffer,
    const char* loi,
    const char* placeholder
) {
    ImGui::PushID(id);
    HienThiNhan(label);

    bool coLoi = (loi != nullptr && loi[0] != '\0');
    if (coLoi) {
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::Danger);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
    }

    ImGui::SetNextItemWidth(-1.0f);
    bool changed = InputTextStdString(
        "##phone_input",
        placeholder,
        &buffer,
        ImGuiInputTextFlags_CallbackCharFilter,
        FilterPhoneCallback,
        nullptr,
        10
    );

    if (buffer.length() > 10) {
        buffer = buffer.substr(0, 10);
        changed = true;
    }

    if (coLoi) {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    HienThiLoi(loi);
    ImGui::PopID();
    return changed;
}

}  // namespace Widgets
