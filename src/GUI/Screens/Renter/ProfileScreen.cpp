#include "GUI/Screens/Renter/ProfileScreen.h"

#include <string>
#include <imgui.h>

#include "Account/Customer.h"
#include "GUI/App/Session.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Theme/Fonts.h"
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
    sdtBanDau_ = formSDT_;

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
            loiHoTen_.empty() ? nullptr : loiHoTen_.c_str(),
            u8"Nhập họ và tên của bạn"
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

        // Thống kê tài khoản bên phải với các thẻ tóm tắt trực quan
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextUnformatted(u8"Thống kê hoạt động tài khoản:");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        float rightColW = ImGui::GetContentRegionAvail().x;
        ImDrawList* dl = ImGui::GetWindowDrawList();

        auto veStatRow = [dl, rightColW](const char* label, const char* val, ImVec4 accent) {
            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImVec2 p1(p0.x + rightColW, p0.y + 42.0f);
            dl->AddRectFilled(p0, p1, GUI::Theme::Colors::toU32(GUI::Theme::Colors::GrayHover), 6.0f);
            dl->AddRect(p0, p1, GUI::Theme::Colors::toU32(GUI::Theme::Colors::Border), 6.0f);
            dl->AddRectFilled(p0, ImVec2(p0.x + 3.5f, p1.y), GUI::Theme::Colors::toU32(accent), 6.0f);

            dl->AddText(ImVec2(p0.x + 12.0f, p0.y + 12.0f), GUI::Theme::Colors::toU32(GUI::Theme::Colors::TextSecondary), label);
            ImVec2 valSize = ImGui::CalcTextSize(val);
            dl->AddText(ImVec2(p1.x - valSize.x - 12.0f, p0.y + 12.0f), GUI::Theme::Colors::toU32(GUI::Theme::Colors::TextPrimary), val);
            ImGui::Dummy(ImVec2(rightColW, 44.0f));
        };

        std::string sDat = std::to_string(soLanDat_) + u8" lần";
        veStatRow(u8"Đặt sân thành công", sDat.c_str(), GUI::Theme::Colors::Primary);

        float tyLeHuy = 0.0f;
        int tong = soLanDat_ + soLanHuy_;
        if (tong > 0) {
            tyLeHuy = (static_cast<float>(soLanHuy_) / tong) * 100.0f;
        }
        char sHuy[64];
        std::snprintf(sHuy, sizeof(sHuy), u8"%d lần (%.0f%%)", soLanHuy_, tyLeHuy);
        veStatRow(u8"Đã hủy đặt sân", sHuy, (tyLeHuy > 20.0f ? GUI::Theme::Colors::Danger : GUI::Theme::Colors::Warning));

        ImVec2 p0 = ImGui::GetCursorScreenPos();
        ImVec2 p1(p0.x + rightColW, p0.y + 42.0f);
        dl->AddRectFilled(p0, p1, GUI::Theme::Colors::toU32(GUI::Theme::Colors::GrayHover), 6.0f);
        dl->AddRect(p0, p1, GUI::Theme::Colors::toU32(GUI::Theme::Colors::Border), 6.0f);
        dl->AddRectFilled(p0, ImVec2(p0.x + 3.5f, p1.y), GUI::Theme::Colors::toU32(GUI::Theme::Colors::Info), 6.0f);
        dl->AddText(ImVec2(p0.x + 12.0f, p0.y + 12.0f), GUI::Theme::Colors::toU32(GUI::Theme::Colors::TextSecondary), u8"Hạng thành viên");

        ImGui::SetCursorScreenPos(ImVec2(p1.x - 150.0f, p0.y + 8.0f));
        if (hangThanhVien_.find("VIP") != std::string::npos) {
            Widgets::BadgeVIP(hangThanhVien_.c_str());
        } else {
            Widgets::Badge(hangThanhVien_.c_str(), GUI::Theme::Colors::PrimaryLight, GUI::Theme::Colors::Primary);
        }
        ImGui::SetCursorScreenPos(ImVec2(p0.x, p1.y + 4.0f));

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

            auto kq = GUI::App::Facade::getInstance().capNhatThongTinUser(sdtBanDau_, formHoTen_, formSDT_);
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
