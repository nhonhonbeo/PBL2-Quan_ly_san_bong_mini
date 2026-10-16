#include "GUI/Widgets/InputFields.h"

#include <cctype>
#include <vector>
#include <imgui.h>
#include "GUI/Theme/Theme.h"

namespace Widgets {

namespace {

// LÝ DO CHỌN: Callback chuẩn của ImGui hỗ trợ std::string tự động cấp phát bộ nhớ khi người dùng gõ
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
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::TextUnformatted(label);
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }
}

void HienThiLoi(const char* loi) {
    if (loi != nullptr && loi[0] != '\0') {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
        ImGui::TextUnformatted(loi);
        ImGui::PopStyleColor();
    }
}

}  // namespace

bool InputText(
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
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::RedDanger);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
    }

    ImGui::SetNextItemWidth(-1.0f);
    bool changed = InputTextStdString("##input", placeholder, &buffer);

    if (coLoi) {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    HienThiLoi(loi);
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
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::RedDanger);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
    }

    float nutMatWidth = 72.0f;
    float availWidth = ImGui::GetContentRegionAvail().x;
    ImGui::SetNextItemWidth(availWidth - nutMatWidth - ImGui::GetStyle().ItemSpacing.x);

    ImGuiInputTextFlags flags = hienMatKhau ? 0 : ImGuiInputTextFlags_Password;
    bool changed = InputTextStdString("##pw_input", placeholder, &buffer, flags);

    if (coLoi) {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    // LÝ DO CHỌN: Nút chuyển đổi hiện/ẩn mật khẩu giúp người dùng kiểm tra lại mật khẩu vừa nhập
    const char* iconText = hienMatKhau ? u8"Ẩn" : u8"Hiện";
    ImGui::PushStyleColor(ImGuiCol_Button, GUI::Theme::Colors::WhiteCard);
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
    ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::BorderLight);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    if (ImGui::Button(iconText, ImVec2(nutMatWidth, 0.0f))) {
        hienMatKhau = !hienMatKhau;
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

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
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::RedDanger);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
    }

    ImGui::SetNextItemWidth(-1.0f);
    bool changed = ImGui::InputInt("##int_input", &giaTri, step, stepFast);

    if (coLoi) {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    HienThiLoi(loi);
    ImGui::PopID();
    return changed;
}

static int FilterPhoneCallback(ImGuiInputTextCallbackData* data) {
    // Chỉ cho phép ký tự số từ 0 đến 9
    if (data->EventChar < '0' || data->EventChar > '9') {
        return 1;
    }
    auto* user_data = static_cast<InputTextCallback_UserData*>(data->UserData);
    if (user_data && user_data->Str && user_data->Str->length() >= 10) {
        return 1; // Đã đủ 10 chữ số, chặn nhập thêm
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
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::RedDanger);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
    }

    ImGui::SetNextItemWidth(-1.0f);
    // LÝ DO CHỌN: Callback lọc ký tự chặn ngay từ bàn phím mọi ký tự không phải số và giới hạn 10 số
    bool changed = InputTextStdString(
        "##phone_input",
        placeholder,
        &buffer,
        ImGuiInputTextFlags_CallbackCharFilter,
        FilterPhoneCallback,
        nullptr,
        10
    );

    // Đảm bảo không vượt quá 10 ký tự nếu người dùng dán (paste)
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
