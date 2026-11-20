#include "GUI/Screens/Owner/CreateFieldWizardScreen.h"

#include <algorithm>
#include <cstring>
#include <imgui.h>

#include "Account/Owner.h"
#include "GUI/App/Facade.h"
#include "GUI/App/Session.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Theme/Fonts.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/InputFields.h"
#include "GUI/Widgets/Modal.h"
#include "GUI/Widgets/Layout.h"
#include "GUI/Widgets/StepIndicator.h"
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
    // Tiêu đề trang đã có ở thanh trên nên thẻ không lặp lại; form giới hạn bề rộng để dễ đọc.
    Widgets::BeginNoiDungHep("##wizard_wrap", 1440.0f);
    Widgets::Card("card_create_field_wizard", "", [this]() {
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
    Widgets::EndNoiDungHep();

    renderResultModal();
}

void CreateFieldWizardScreen::renderProgressBar() {
    Widgets::ThanhCacBuoc(
        {u8"Loại sân", u8"Số lượng", u8"Quy định", u8"Xác nhận"},
        currentStep_);
}

void CreateFieldWizardScreen::renderStep1LoaiSan() {
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
    if (GUI::Theme::getFonts().heading) ImGui::PushFont(GUI::Theme::getFonts().heading);
    ImGui::TextUnformatted(u8"Chọn loại sân bóng");
    if (GUI::Theme::getFonts().heading) ImGui::PopFont();
    ImGui::TextUnformatted(u8"Lựa chọn loại sân phù hợp với cơ sở của bạn.");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    float availW = ImGui::GetContentRegionAvail().x - 18.0f;
    float cardSpacing = 16.0f;
    const int cols = availW >= 960.0f ? 3 : (availW >= 620.0f ? 2 : 1);
    float cardW = (availW - cardSpacing * (cols - 1)) / cols;
    const float cardH = cols == 3 ? 320.0f : 250.0f;
    const float artH = cols == 3 ? 136.0f : 90.0f;
    float startX = ImGui::GetCursorPosX();
    float startY = ImGui::GetCursorPosY();

    auto veTheLoai = [this, cardW, cardH, artH](int loai, const char* tieuDe, const char* kichThuoc, const char* co, const char* giaDeXuat) {
        bool isSelected = (loaiSan_ == loai);

        ImVec2 p0 = ImGui::GetCursorScreenPos();
        ImVec2 p1 = ImVec2(p0.x + cardW, p0.y + cardH);
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // Nền thẻ & Viền chuẩn Design System
        ImU32 bgCol = isSelected
            ? GUI::Theme::Colors::toU32(GUI::Theme::Colors::PrimaryLight)
            : GUI::Theme::Colors::toU32(GUI::Theme::Colors::Surface);
        ImU32 borderCol = isSelected
            ? GUI::Theme::Colors::toU32(GUI::Theme::Colors::Primary)
            : GUI::Theme::Colors::toU32(GUI::Theme::Colors::Border);

        dl->AddRectFilled(p0, p1, bgCol, 8.0f);
        dl->AddRect(p0, p1, borderCol, 8.0f, 0, isSelected ? 2.0f : 1.0f);

        // Nút radio tròn ở góc trên phải
        ImVec2 radioCenter(p1.x - 20.0f, p0.y + artH + 45.0f);
        dl->AddCircle(radioCenter, 8.0f, borderCol, 0, 1.5f);
        if (isSelected) {
            dl->AddCircleFilled(radioCenter, 4.5f, GUI::Theme::Colors::toU32(GUI::Theme::Colors::Primary));
        }

        std::string btnId = "##btn_loai_" + std::to_string(loai);
        ImGui::SetCursorScreenPos(p0);
        if (ImGui::InvisibleButton(btnId.c_str(), ImVec2(cardW, cardH))) {
            loaiSan_ = loai;
        }

        // Pitch illustration: vector only, no assets or dependencies.
        const ImVec2 a(p0.x + 16, p0.y + 16), b(p1.x - 16, p0.y + 16 + artH);
        dl->AddRectFilled(a,b,IM_COL32(21,105,70,255),6);
        const ImVec2 l(a.x+10,a.y+9), r(b.x-10,b.y-9);
        const ImU32 white = IM_COL32(210,240,220,255);
        dl->AddRect(l,r,white,2);
        const float mid=(l.x+r.x)*.5f, cy=(l.y+r.y)*.5f;
        dl->AddLine(ImVec2(mid,l.y),ImVec2(mid,r.y),white,1.5f);
        dl->AddCircle(ImVec2(mid,cy),17,white,24,1.5f);
        dl->AddRect(ImVec2(l.x,cy-19),ImVec2(l.x+30,cy+19),white);
        dl->AddRect(ImVec2(r.x-30,cy-19),ImVec2(r.x,cy+19),white);
        // Nội dung trong thẻ
        ImGui::SetCursorScreenPos(ImVec2(p0.x + 16.0f, p0.y + artH + 32.0f));
        ImGui::BeginGroup();

        if (GUI::Theme::getFonts().title) ImGui::PushFont(GUI::Theme::getFonts().title);
        ImGui::PushStyleColor(ImGuiCol_Text, isSelected ? GUI::Theme::Colors::Primary : GUI::Theme::Colors::TextPrimary);
        ImGui::TextUnformatted(tieuDe);
        ImGui::PopStyleColor();
        if (GUI::Theme::getFonts().title) ImGui::PopFont();

        ImGui::Spacing();
        if (GUI::Theme::getFonts().small) ImGui::PushFont(GUI::Theme::getFonts().small);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
        ImGui::Text(u8"Kích thước: %s", kichThuoc);
        ImGui::Text(u8"Mặt sân: %s", co);
        ImGui::Text(u8"Giá tham khảo: %s", giaDeXuat);
        ImGui::PopStyleColor();
        if (GUI::Theme::getFonts().small) ImGui::PopFont();

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
        ImGui::SetCursorPos(ImVec2(startX + (i % cols) * (cardW + cardSpacing), startY + (i / cols) * (cardH + cardSpacing)));
        veTheLoai(loaiSan[i], tieuDe[i], kichThuoc[i], matSan[i], giaDeXuat[i]);
    }

    ImGui::SetCursorPos(ImVec2(startX, startY + ((3 + cols - 1) / cols) * (cardH + cardSpacing)));
    ImGui::Dummy(ImVec2(1.0f, 1.0f));
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

    if (ImGui::InputTextMultiline("##quy_dinh_multiline", quyDinhBuf, sizeof(quyDinhBuf), ImVec2(std::max(100.0f, ImGui::GetContentRegionAvail().x - 18.0f), 260.0f))) {
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
    constexpr float kChieuCaoNut = 46.0f;
    const float btnW = 120.0f;
    const float confirmW = currentStep_ == 4 ? 180.0f : btnW;
    const float spacing = ImGui::GetStyle().ItemSpacing.x;

    // Tính trước mép phải của hàng để cả ba nút nằm trên cùng một dòng:
    // "Hủy bỏ" bên trái, "Quay lại" + "Tiếp tục" bên phải.
    const float rowStartX = ImGui::GetCursorPosX();
    const float rightEdgeX = rowStartX + ImGui::GetContentRegionAvail().x - 18.0f;
    const float rightGroupWidth = btnW + confirmW + spacing;

    if (Widgets::NutPhu(u8"Hủy bỏ", ImVec2(btnW, kChieuCaoNut))) {
        if (onHoanTatCallback_) {
            onHoanTatCallback_();
        }
    }

    ImGui::SameLine();
    ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), rightEdgeX - rightGroupWidth));

    // Nút Quay lại
    ImGui::BeginDisabled(currentStep_ <= 1);
    if (Widgets::NutPhu(u8"Quay lại", ImVec2(btnW, kChieuCaoNut))) {
        if (currentStep_ > 1) {
            currentStep_--;
        }
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    // Nút Tiếp tục / Xác nhận tạo sân
    if (currentStep_ < 4) {
        if (Widgets::NutChinh(u8"Tiếp tục", ImVec2(btnW, kChieuCaoNut))) {
            currentStep_++;
        }
    } else {
        if (Widgets::NutChinh(u8"Xác nhận tạo sân", ImVec2(confirmW, kChieuCaoNut))) {
            auto kq = GUI::App::Facade::getInstance().taoNhieuSan(loaiSan_, soLuong_, quyDinh_, danhSachMaSanVuaTao_);
            if (kq.ok) {
                moPopupKetQua_ = true;
                Widgets::MoModalXacNhan(u8"Kết quả khởi tạo sân bóng");
            } else {
                // Trước đây lỗi bị bỏ qua im lặng, người dùng bấm xong không thấy gì.
                Widgets::ToastLoi(kq.loi.empty() ? u8"Không thể tạo sân bóng." : kq.loi);
            }
        }
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
