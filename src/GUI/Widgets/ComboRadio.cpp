#include "GUI/Widgets/ComboRadio.h"

#include <imgui.h>
#include "GUI/Theme/Theme.h"

namespace Widgets {

namespace {

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

bool Combo(
    const char* label,
    const char* id,
    int& selectedIndex,
    const std::vector<std::string>& items,
    const char* loi
) {
    ImGui::PushID(id);
    HienThiNhan(label);

    bool coLoi = (loi != nullptr && loi[0] != '\0');
    if (coLoi) {
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::RedDanger);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
    }

    const char* previewText = (selectedIndex >= 0 && selectedIndex < static_cast<int>(items.size()))
                                  ? items[selectedIndex].c_str()
                                  : "";

    ImGui::SetNextItemWidth(-1.0f);
    bool changed = false;
    if (ImGui::BeginCombo("##combo", previewText)) {
        for (int i = 0; i < static_cast<int>(items.size()); ++i) {
            const bool isSelected = (selectedIndex == i);
            if (ImGui::Selectable(items[i].c_str(), isSelected)) {
                selectedIndex = i;
                changed = true;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (coLoi) {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    HienThiLoi(loi);
    ImGui::PopID();
    return changed;
}

bool Radio(const char* label, int& selectedValue, int value) {
    return ImGui::RadioButton(label, &selectedValue, value);
}

bool RadioGroup(
    const char* label,
    const char* id,
    int& selectedValue,
    const std::vector<std::pair<int, std::string>>& options,
    bool ngang
) {
    ImGui::PushID(id);
    HienThiNhan(label);

    bool changed = false;
    for (size_t i = 0; i < options.size(); ++i) {
        if (i > 0 && ngang) {
            ImGui::SameLine(0.0f, 16.0f);
        }
        if (ImGui::RadioButton(options[i].second.c_str(), selectedValue == options[i].first)) {
            selectedValue = options[i].first;
            changed = true;
        }
    }

    ImGui::PopID();
    return changed;
}

}  // namespace Widgets
