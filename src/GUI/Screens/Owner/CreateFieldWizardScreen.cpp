#include "GUI/Screens/Owner/CreateFieldWizardScreen.h"

#include <cstring>
#include <imgui.h>

#include "Account/Owner.h"
#include "GUI/App/Facade.h"
#include "GUI/App/Session.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/InputFields.h"
#include "GUI/Widgets/Modal.h"
#include "GUI/Widgets/ProgressBar.h"
#include "GUI/Widgets/Toast.h"

namespace GUI::Screens::Owner {

CreateFieldWizardScreen::CreateFieldWizardScreen(std::function<void()> onHoanTat)
    : onHoanTatCallback_(std::move(onHoanTat)) {}

void CreateFieldWizardScreen::onEnter() {
    currentStep_ = 1;
    loaiSan_ = 5;
    soLuong_ = 2;
    moPopupKetQua_ = false;
    danhSachMaSanVuaTao_.clear();

    quyDinh_ = u8"1. Vui lòng mang giày thể thao chuyên dụng (đế cao su TF hoặc IC).\n"
               u8"2. Nghiêm cấm hút thuốc lá và mang kẹo cao su vào mặt sân cỏ.\n"
               u8"3. Có mặt trước giờ thi đấu 10 phút để nhận sân đúng lịch.\n"
               u8"4. Giữ gìn vệ sinh chung và bảo quản trang thiết bị bóng, lưới.";

    User* user = GUI::App::Session::getInstance().getUser();
    auto owner = dynamic_cast<::Owner*>(user);

    tenChuSan_ = user ? user->get_name() : u8"Nguyễn Văn Chủ";
    sdtChuSan_ = user ? user->get_phone_number() : "0901234567";
    tenCoSo_   = owner ? owner->get_field_name() : u8"Cụm sân Sao Vàng";
}

void CreateFieldWizardScreen::onExit() {
}

void CreateFieldWizardScreen::render() {
    Widgets::Card("card_create_field_wizard", u8"Quy trình tạo sân bóng mới (Wizard 4 bước)", [this]() {
        renderProgressBar();
        ImGui::Spacing();
        ImGui::Spacing();

        switch (currentStep_) {
            case 1:
                renderStep1LoaiSan();
                break;
            case 2:
                renderStep2SoLuong();
                break;
            case 3:
                renderStep3QuyDinh();
                break;
            case 4:
                renderStep4XacNhan();
                break;
            default:
                break;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        renderNavigationButtons();
    });

    renderResultModal();
}

void CreateFieldWizardScreen::renderProgressBar() {
    float ratio = currentStep_ * 0.25f;
    const char* stepLabels[] = {
        "",
        u8"Bước 1 / 4: Chọn loại sân bóng",
        u8"Bước 2 / 4: Thiết lập số lượng sân",
        u8"Bước 3 / 4: Ban hành quy định sử dụng",
        u8"Bước 4 / 4: Xác nhận thông tin & Hoàn tất"
    };

    Widgets::ProgressBar(ratio, stepLabels[currentStep_], ImVec2(-1.0f, 30.0f));
}

void CreateFieldWizardScreen::renderStep1LoaiSan() {
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
    ImGui::TextUnformatted(u8"Vui lòng chọn loại sân bóng bạn muốn khởi tạo:");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    float availW = ImGui::GetContentRegionAvail().x;
    float cardSpacing = 16.0f;
    float cardW = (availW - cardSpacing * 2.0f) / 3.0f;
    float startX = ImGui::GetCursorPosX();
    float startY = ImGui::GetCursorPosY();

    auto veTheLoai = [this, cardW](int loai, const char* tieuDe, const char* kichThuoc, const char* co, const char* giaDeXuat) {
        bool isSelected = (loaiSan_ == loai);

        ImVec2 p0 = ImGui::GetCursorScreenPos();
        ImVec2 p1 = ImVec2(p0.x + cardW, p0.y + 140.0f);
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // Nền thẻ
        ImU32 bgCol = isSelected
            ? IM_COL32(236, 253, 245, 255) // Xanh lá rất nhạt
            : GUI::Theme::Colors::toU32(GUI::Theme::Colors::WhiteCard);
        ImU32 borderCol = isSelected
            ? GUI::Theme::Colors::toU32(GUI::Theme::Colors::GrassGreen)
            : GUI::Theme::Colors::toU32(GUI::Theme::Colors::BorderLight);

        dl->AddRectFilled(p0, p1, bgCol, 8.0f);
        dl->AddRect(p0, p1, borderCol, 8.0f, 0, isSelected ? 2.5f : 1.0f);

        std::string btnId = "##btn_loai_" + std::to_string(loai);
        ImGui::SetCursorScreenPos(p0);
        if (ImGui::InvisibleButton(btnId.c_str(), ImVec2(cardW, 140.0f))) {
            loaiSan_ = loai;
        }

        // Nội dung trong thẻ
        ImGui::SetCursorScreenPos(ImVec2(p0.x + 16.0f, p0.y + 14.0f));
        ImGui::BeginGroup();

        ImGui::PushStyleColor(ImGuiCol_Text, isSelected ? GUI::Theme::Colors::GrassGreen : GUI::Theme::Colors::DarkText);
        ImGui::TextUnformatted(tieuDe);
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::Text(u8"• Kích thước: %s", kichThuoc);
        ImGui::Text(u8"• Mặt sân: %s", co);
        ImGui::Text(u8"• Đơn giá đề xuất: %s", giaDeXuat);
        ImGui::PopStyleColor();

        ImGui::EndGroup();

        ImGui::SetCursorScreenPos(ImVec2(p1.x, p0.y));
    };

    // LÝ DO CHỌN: Cố định cùng tọa độ Y để các thẻ không bị đẩy lệch bởi nội dung từng thẻ.
    const int loaiSan[] = {5, 7, 11};
    const char* tieuDe[] = {u8"Sân 5 người (Mini)", u8"Sân 7 người (Trung)", u8"Sân 11 người (Lớn)"};
    const char* kichThuoc[] = {u8"20m x 40m", u8"30m x 50m", u8"60m x 100m"};
    const char* matSan[] = {u8"Cỏ nhân tạo 50mm", u8"Khuôn viên ngoài trời", u8"Mặt cỏ thi đấu"};
    const char* giaDeXuat[] = {u8"200k - 350k/h", u8"350k - 550k/h", u8"700k - 1.2tr/h"};
    for (int i = 0; i < 3; ++i) {
        ImGui::SetCursorPos(ImVec2(startX + i * (cardW + cardSpacing), startY));
        veTheLoai(loaiSan[i], tieuDe[i], kichThuoc[i], matSan[i], giaDeXuat[i]);
    }

    ImGui::Dummy(ImVec2(availW, 145.0f));
}

void CreateFieldWizardScreen::renderStep2SoLuong() {
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
    ImGui::Text(u8"Số lượng sân bóng loại %d người bạn muốn tạo thêm:", loaiSan_);
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // Hàng nút điều chỉnh số lượng [-] [ X ] [+]
    float startX = ImGui::GetCursorPosX() + 40.0f;
    ImGui::SetCursorPosX(startX);

    if (Widgets::NutPhu("-##giam_sl", ImVec2(48.0f, 44.0f))) {
        if (soLuong_ > 1) {
            soLuong_--;
        }
    }

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
    std::string slStr = "  " + std::to_string(soLuong_) + u8" Sân  ";
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::TextUnformatted(slStr.c_str());
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 8.0f);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (Widgets::NutPhu("+##tang_sl", ImVec2(48.0f, 44.0f))) {
        if (soLuong_ < 10) {
            soLuong_++;
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
    ImGui::TextWrapped(u8"Hệ thống sẽ tự động sinh mã số định danh liên tiếp bắt đầu bằng tiền tố 'S%d-xx' (ví dụ: S%d-01, S%d-02...) để đảm bảo tính duy nhất.", loaiSan_, loaiSan_, loaiSan_);
    ImGui::PopStyleColor();
}

void CreateFieldWizardScreen::renderStep3QuyDinh() {
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
    ImGui::TextUnformatted(u8"Quy định sử dụng cụm sân bóng (cho phép chỉnh sửa tự do):");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    char quyDinhBuf[1024];
    std::memset(quyDinhBuf, 0, sizeof(quyDinhBuf));
    std::strncpy(quyDinhBuf, quyDinh_.c_str(), sizeof(quyDinhBuf) - 1);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, GUI::Theme::Colors::WhiteCard);
    ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::InputBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

    if (ImGui::InputTextMultiline("##quy_dinh_multiline", quyDinhBuf, sizeof(quyDinhBuf), ImVec2(-1.0f, 150.0f))) {
        quyDinh_ = quyDinhBuf;
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

void CreateFieldWizardScreen::renderStep4XacNhan() {
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
    ImGui::TextUnformatted(u8"Xác nhận tóm tắt thông tin trước khi khởi tạo:");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // 1. Tóm tắt thông số sân
    ImGui::Text(u8"• Loại sân bóng: Sân %d người", loaiSan_);
    ImGui::Text(u8"• Số lượng sân mới tạo: %d sân", soLuong_);
    ImGui::Text(u8"• Tiền tố mã sân dự kiến: S%d-xx", loaiSan_);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 2. Thông tin chủ sân (cho phép kiểm tra & sửa nhanh)
    ImGui::TextUnformatted(u8"Thông tin chủ sân & cơ sở quản lý (có thể điều chỉnh):");
    ImGui::Spacing();

    Widgets::InputText(u8"Họ tên chủ sân:", "wz_owner_name", tenChuSan_);
    ImGui::Spacing();
    Widgets::InputPhone(u8"Số điện thoại liên hệ:", "wz_owner_phone", sdtChuSan_);
    ImGui::Spacing();
    Widgets::InputText(u8"Tên cơ sở sân bóng:", "wz_facility_name", tenCoSo_);
}

void CreateFieldWizardScreen::renderNavigationButtons() {
    float btnW = 120.0f;
    float confirmW = currentStep_ == 4 ? 180.0f : btnW;

    // Nút Hủy
    if (Widgets::NutPhu(u8"Hủy bỏ", ImVec2(btnW, 36.0f))) {
        if (onHoanTatCallback_) {
            onHoanTatCallback_();
        }
    }

    float rowStartX = ImGui::GetCursorPosX();
    float rightEdgeX = rowStartX + ImGui::GetContentRegionAvail().x;
    float rightGroupWidth = btnW + confirmW + ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetCursorPosX(std::max(rowStartX, rightEdgeX - rightGroupWidth));

    // Nút Quay lại
    if (currentStep_ <= 1) {
        ImGui::BeginDisabled();
    }
    if (Widgets::NutPhu(u8"Quay lại", ImVec2(btnW, 36.0f))) {
        if (currentStep_ > 1) {
            currentStep_--;
        }
    }
    if (currentStep_ <= 1) {
        ImGui::EndDisabled();
    }

    ImGui::SameLine();

    // Nút Tiếp tục / Xác nhận tạo sân
    if (currentStep_ < 4) {
        if (Widgets::NutChinh(u8"Tiếp tục >", ImVec2(btnW, 36.0f))) {
            currentStep_++;
        }
    } else {
        if (Widgets::NutChinh(u8"Xác nhận tạo sân", ImVec2(confirmW, 36.0f))) {
            // Thực hiện tạo sân qua Facade
            auto kq = GUI::App::Facade::getInstance().taoNhieuSan(loaiSan_, soLuong_, quyDinh_, danhSachMaSanVuaTao_);
            if (kq.ok) {
                moPopupKetQua_ = true;
                Widgets::MoModalXacNhan(u8"Kết quả khởi tạo sân bóng");
            } else {
            }
        }
        ImGui::Dummy(ImVec2(0.0f, 4.0f));
    }
}

void CreateFieldWizardScreen::renderResultModal() {
    if (!moPopupKetQua_) {
        return;
    }

    Widgets::ModalXacNhan(
        u8"Kết quả khởi tạo sân bóng",
        u8"Chúc mừng! Các sân bóng mới đã được tự động sinh mã định danh và lưu vào hệ thống thành công.",
        [this]() {
            moPopupKetQua_ = false;
            if (onHoanTatCallback_) {
                onHoanTatCallback_();
            }
        },
        u8"Xem danh sách sân",
        u8"Đóng",
        false
    );
}

}  // namespace GUI::Screens::Owner
