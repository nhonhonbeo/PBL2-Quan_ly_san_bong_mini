#include "GUI/Screens/Owner/FieldManagementScreen.h"

#include <algorithm>
#include <cstdio>
#include <imgui.h>

#include "Core/DateTime.h"
#include "GUI/Theme/Theme.h"
#include "GUI/App/Session.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/ComboRadio.h"
#include "GUI/Widgets/InputFields.h"
#include "GUI/Widgets/Modal.h"
#include "GUI/Widgets/TimePicker.h"
#include "GUI/Widgets/Toast.h"

namespace GUI::Screens::Owner {

void FieldManagementScreen::renderModalThemNhanh() {
    Widgets::KiemTraVaMoModal(u8"Thêm nhanh sân bóng");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(440.0f, 0.0f), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal(u8"Thêm nhanh sân bóng", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        const std::vector<std::string> loaiSanLabels = {
            u8"Sân 5 người",
            u8"Sân 7 người",
            u8"Sân 11 người"
        };
        int selectedType = loaiSanThemNhanh_ == 5 ? 0 : (loaiSanThemNhanh_ == 7 ? 1 : 2);
        Widgets::Combo(u8"Loại sân", "##quick_field_type", selectedType, loaiSanLabels);
        loaiSanThemNhanh_ = selectedType == 0 ? 5 : (selectedType == 1 ? 7 : 11);

        Widgets::InputInt(u8"Số lượng sân", "##quick_field_count", soLuongSanThemNhanh_);
        soLuongSanThemNhanh_ = std::max(1, std::min(10, soLuongSanThemNhanh_));

        if (!loiThemNhanh_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
            ImGui::TextWrapped("%s", loiThemNhanh_.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Separator();
        if (Widgets::NutChinh(u8"Tạo sân", ImVec2(120.0f, 34.0f))) {
            std::vector<std::string> maSanMoi;
            auto result = GUI::App::Facade::getInstance().taoNhieuSan(
                loaiSanThemNhanh_,
                soLuongSanThemNhanh_,
                "",
                maSanMoi
            );
            if (result.ok) {
                loiThemNhanh_.clear();
                reloadViewModel();
                ImGui::CloseCurrentPopup();
            } else {
                loiThemNhanh_ = result.loi;
            }
        }
        ImGui::SameLine();
        if (Widgets::NutPhu(u8"Đóng", ImVec2(100.0f, 34.0f))) {
            loiThemNhanh_.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void FieldManagementScreen::renderModalXoaSan() {
    Widgets::ModalXacNhan(
        u8"Xác nhận xóa sân bóng",
        (u8"Xác nhận xóa vĩnh viễn sân bóng '" + maSanCanXoa_ + u8"' khỏi cơ sở.\nMọi dữ liệu liên quan đến sân này sẽ bị xóa bỏ.").c_str(),
        [this]() {
            auto kq = GUI::App::Facade::getInstance().xoaSanBong(maSanCanXoa_);
            if (kq.ok) {
                reloadViewModel();
            } else if (!kq.loi.empty()) {
                Widgets::ToastLoi(kq.loi);
            }
        },
        u8"Xóa vĩnh viễn",
        u8"Hủy bỏ",
        true // Nút đỏ nguy hiểm
    );
}

void FieldManagementScreen::renderModalSuaSan() {
    Widgets::KiemTraVaMoModal(u8"Chỉnh sửa thông tin sân bóng");

    // Modal tùy biến chỉnh sửa thông tin sân bóng
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(440.0f, 0.0f), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal(u8"Chỉnh sửa thông tin sân bóng", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Spacing();
        ImGui::Text(u8"Mã sân định danh: %s", maSanDangSua_.c_str());
        ImGui::Spacing();

        Widgets::InputText(u8"Tên sân bóng:", "edit_field_name", tenSanDangSua_);
        ImGui::Spacing();

        ImGui::TextWrapped(
            u8"Giá được quản lý theo bảng giá của cơ sở tại tab Bảng giá thuê."
        );
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (Widgets::NutChinh(u8"Lưu thay đổi", ImVec2(130.0f, 34.0f))) {
            auto kq = GUI::App::Facade::getInstance().suaSanBong(
                maSanDangSua_, tenSanDangSua_, loaiSanDangSua_, 0
            );
            if (kq.ok) {
                reloadViewModel();
                ImGui::CloseCurrentPopup();
            } else if (!kq.loi.empty()) {
                Widgets::ToastLoi(kq.loi);
            }
        }

        ImGui::SameLine();
        if (Widgets::NutPhu(u8"Hủy bỏ", ImVec2(100.0f, 34.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void FieldManagementScreen::renderModalUploadAnhSan() {
    Widgets::KiemTraVaMoModal(u8"Cập nhật ảnh sân bóng");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(520.0f, 0.0f), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal(u8"Cập nhật ảnh sân bóng", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Primary);
        ImGui::Text(u8"Sân bóng: %s - %s", maSanUploadAnh_.c_str(), tenSanUploadAnh_.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::TextWrapped(u8"Tải lên hoặc chọn ảnh sân bóng để khách hàng thấy được mặt cỏ, không gian và cơ sở vật chất khi đặt sân trực tuyến.");
        ImGui::Spacing();

        Widgets::InputText(u8"Đường dẫn file ảnh (hoặc URL):", "input_field_img_path", duongDanAnhMoi_);
        ImGui::Spacing();

        ImGui::TextUnformatted(u8"Hoặc chọn nhanh từ thư viện ảnh sân bóng tiêu chuẩn:");
        ImGui::Spacing();

        auto nutPreset = [this](const char* label, const char* path) {
            if (ImGui::Button(label, ImVec2(240.0f, 30.0f))) {
                duongDanAnhMoi_ = path;
            }
        };

        nutPreset(u8"Sân cỏ chuẩn FIFA Pro", "assets/images/san_co_fifa_vip.jpg");
        ImGui::SameLine(0.0f, 10.0f);
        nutPreset(u8"Sân đêm dàn đèn LED", "assets/images/san_den_led_night.jpg");

        nutPreset(u8"Sân ngoài trời thoáng mát", "assets/images/san_ngoai_troi_view.jpg");
        ImGui::SameLine(0.0f, 10.0f);
        nutPreset(u8"Sân mái che cao cấp", "assets/images/san_mai_che_premium.jpg");

        ImGui::Spacing();

        if (!duongDanAnhMoi_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Success);
            ImGui::Text(u8"Ảnh đã chọn: %s", duongDanAnhMoi_.c_str());
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
            ImGui::TextUnformatted(u8"Chưa chọn ảnh (sân sẽ dùng ảnh sa bàn mặc định).");
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (Widgets::NutChinh(u8"Lưu ảnh sân", ImVec2(130.0f, 34.0f))) {
            auto kq = GUI::App::Facade::getInstance().uploadAnhSan(maSanUploadAnh_, duongDanAnhMoi_);
            if (kq.ok) {
                reloadViewModel();
                Widgets::ToastThanhCong(u8"Đã cập nhật ảnh sân bóng thành công!");
                ImGui::CloseCurrentPopup();
            } else {
                Widgets::ToastLoi(kq.loi);
            }
        }

        ImGui::SameLine(0.0f, 10.0f);
        if (Widgets::NutPhu(u8"Xóa ảnh", ImVec2(90.0f, 34.0f))) {
            duongDanAnhMoi_.clear();
            GUI::App::Facade::getInstance().uploadAnhSan(maSanUploadAnh_, "");
            reloadViewModel();
            Widgets::ToastThanhCong(u8"Đã xóa ảnh sân bóng!");
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(0.0f, 10.0f);
        if (Widgets::NutPhu(u8"Đóng", ImVec2(90.0f, 34.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

}  // namespace GUI::Screens::Owner
