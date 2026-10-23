#include "GUI/Screens/Renter/ProfileScreen.h"

#include <string>
#include <imgui.h>

#include "Account/Customer.h"
#include "GUI/App/Session.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/FormValidator.h"
#include "GUI/Widgets/InputFields.h"
#include "GUI/Widgets/Toast.h"

namespace GUI::Screens::Renter {

void ProfileScreen::onEnter() {
    formMatKhauCu_.clear();
    formMatKhauMoi_.clear();
    formNhapLaiMatKhau_.clear();

    loiHoTen_.clear();
    loiSDT_.clear();
    loiMatKhauCu_.clear();
    loiMatKhauMoi_.clear();
    loiNhapLai_.clear();

    reloadViewModel();
}

void ProfileScreen::onExit() {
}

void ProfileScreen::reloadViewModel() {
    auto& facade = GUI::App::Facade::getInstance();
    User* curUser = GUI::App::Session::getInstance().getUser();

    formHoTen_ = curUser ? curUser->get_name() : u8"Trần Văn Thuê";
    formSDT_ = curUser ? curUser->get_phone_number() : "0909876543";

    soLanDat_ = facade.getSoLanDatCuaKhach(formSDT_);
    soLanHuy_ = facade.getSoLanHuyCuaKhach(formSDT_);

    int mucUuDai = facade.getMucUuDaiCuaKhach(formSDT_);
    if (mucUuDai >= 10) {
        hangThanhVien_ = u8"Thành viên VIP Bạc / Vàng";
    } else if (mucUuDai > 0) {
        hangThanhVien_ = u8"Khách hàng Thân thiết";
    } else {
        hangThanhVien_ = u8"Khách hàng tiêu chuẩn";
    }
}

void ProfileScreen::render() {
    renderFormThongTin();
    ImGui::Spacing();
    ImGui::Spacing();
    renderFormDoiMatKhau();
}

void ProfileScreen::renderFormThongTin() {
    Widgets::Card("card_renter_profile_info", u8"Thông tin hồ sơ cá nhân", [this]() {
        ImGui::Columns(2, "cols_profile_layout", false);
        ImGui::SetColumnWidth(0, 420.0f);

        // Ô sửa họ và tên
        Widgets::InputText(
            u8"Họ và tên (*)",
            "##profile_fullname",
            formHoTen_,
            u8"Nhập họ và tên của bạn",
            loiHoTen_.empty() ? nullptr : loiHoTen_.c_str()
        );
        ImGui::Spacing();

        // Ô xem/sửa SĐT
        Widgets::InputPhone(
            u8"Số điện thoại tài khoản (*)",
            "##profile_phone",
            formSDT_,
            loiSDT_.empty() ? nullptr : loiSDT_.c_str()
        );
        ImGui::Spacing();

        ImGui::NextColumn();

        // Thống kê tài khoản bên phải
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextUnformatted(u8"Thống kê hoạt động tài khoản:");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::Text(u8"• Số lần đã đặt sân thành công: %d lần", soLanDat_);
        ImGui::Text(u8"• Số lần đã hủy sân: %d lần", soLanHuy_);

        float tyLeHuy = 0.0f;
        int tong = soLanDat_ + soLanHuy_;
        if (tong > 0) {
            tyLeHuy = (static_cast<float>(soLanHuy_) / tong) * 100.0f;
        }
        ImGui::Text(u8"• Tỷ lệ hủy đơn: %.1f%%", tyLeHuy);
        ImGui::Spacing();

        ImGui::TextUnformatted(u8"• Hạng tài khoản: ");
        ImGui::SameLine();
        if (hangThanhVien_.find("VIP") != std::string::npos) {
            Widgets::BadgeVIP(hangThanhVien_.c_str());
        } else {
            Widgets::Badge(hangThanhVien_.c_str(), GUI::Theme::Colors::GrassGreen, GUI::Theme::Colors::WhiteCard);
        }

        ImGui::Columns(1);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (Widgets::NutChinh(u8"Lưu thay đổi thông tin", ImVec2(220.0f, 36.0f))) {
            // Kiểm tra tính hợp lệ bằng FormValidator
            loiHoTen_ = Widgets::FormValidator::kiemTraKhongDeTrong(formHoTen_, u8"Họ và tên");
            loiSDT_ = Widgets::FormValidator::kiemTraSDT(formSDT_);

            if (!loiHoTen_.empty() || !loiSDT_.empty()) {
                return;
            }

            auto kq = GUI::App::Facade::getInstance().capNhatThongTinUser(formSDT_, formHoTen_);
            if (kq.ok) {
                reloadViewModel();
            } else {
            }
        }
    });
}

void ProfileScreen::renderFormDoiMatKhau() {
    Widgets::Card("card_renter_change_password", u8"Đổi mật khẩu tài khoản", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Để bảo vệ an toàn cho tài khoản đặt sân, vui lòng sử dụng mật khẩu mới có tối thiểu 6 ký tự.");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::SetNextItemWidth(360.0f);
        Widgets::InputPassword(
            u8"Mật khẩu hiện tại (*)",
            "##profile_pw_cur",
            formMatKhauCu_,
            hienMatKhauCu_,
            loiMatKhauCu_.empty() ? nullptr : loiMatKhauCu_.c_str()
        );
        ImGui::Spacing();

        ImGui::SetNextItemWidth(360.0f);
        Widgets::InputPassword(
            u8"Mật khẩu mới (Tối thiểu 6 ký tự) (*)",
            "##profile_pw_new",
            formMatKhauMoi_,
            hienMatKhauMoi_,
            loiMatKhauMoi_.empty() ? nullptr : loiMatKhauMoi_.c_str()
        );
        ImGui::Spacing();

        ImGui::SetNextItemWidth(360.0f);
        Widgets::InputPassword(
            u8"Nhập lại mật khẩu mới (*)",
            "##profile_pw_confirm",
            formNhapLaiMatKhau_,
            hienNhapLai_,
            loiNhapLai_.empty() ? nullptr : loiNhapLai_.c_str()
        );

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (Widgets::NutChinh(u8"Xác nhận đổi mật khẩu", ImVec2(220.0f, 36.0f))) {
            // Kiểm tra tính hợp lệ bằng FormValidator
            loiMatKhauCu_ = Widgets::FormValidator::kiemTraKhongDeTrong(formMatKhauCu_, u8"Mật khẩu hiện tại");
            loiMatKhauMoi_ = Widgets::FormValidator::kiemTraMatKhau(formMatKhauMoi_, 6);

            if (formNhapLaiMatKhau_.empty()) {
                loiNhapLai_ = u8"Vui lòng nhập lại mật khẩu mới!";
            } else if (formMatKhauMoi_ != formNhapLaiMatKhau_) {
                loiNhapLai_ = u8"Mật khẩu nhập lại không khớp!";
            } else {
                loiNhapLai_.clear();
            }

            if (!loiMatKhauCu_.empty() || !loiMatKhauMoi_.empty() || !loiNhapLai_.empty()) {
                return;
            }

            User* curUser = GUI::App::Session::getInstance().getUser();
            std::string sdt = curUser ? curUser->get_phone_number() : "0909876543";

            auto kq = GUI::App::Facade::getInstance().doiMatKhauUser(sdt, formMatKhauCu_, formMatKhauMoi_);
            if (kq.ok) {
                formMatKhauCu_.clear();
                formMatKhauMoi_.clear();
                formNhapLaiMatKhau_.clear();
            } else {
            }
        }
    });
}

}  // namespace GUI::Screens::Renter
