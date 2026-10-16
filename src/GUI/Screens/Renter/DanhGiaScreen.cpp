#include "GUI/Screens/Renter/DanhGiaScreen.h"

#include <cstdio>
#include <string>
#include <vector>
#include <imgui.h>

#include "GUI/App/Session.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/ComboRadio.h"
#include "GUI/Widgets/StarRating.h"
#include "GUI/Widgets/Toast.h"

namespace GUI::Screens::Renter {

void DanhGiaScreen::onEnter() {
    formIndexSan_ = 0;
    formSoSao_ = 5;
    formNoiDung_.clear();
    formDanhGiaIdDangSua_.clear();
    reloadViewModel();
}

void DanhGiaScreen::onExit() {
}

void DanhGiaScreen::reloadViewModel() {
    auto& facade = GUI::App::Facade::getInstance();
    User* curUser = GUI::App::Session::getInstance().getUser();
    std::string sdt = curUser ? curUser->get_phone_number() : "0909876543";

    // 1. Chỉ liệt kê các sân bóng mà người thuê này đã từng đặt xong
    danhSachSanDaThue_ = facade.getDanhSachSanDaThueXong(sdt);

    // 2. Nạp danh sách các đánh giá đã gửi của tôi
    danhSachDanhGiaCuaToi_ = facade.getDanhGiaCuaKhach(sdt);
}

void DanhGiaScreen::render() {
    renderFormDanhGia();
    ImGui::Spacing();
    ImGui::Spacing();
    renderDanhSachDanhGiaCuaToi();
}

void DanhGiaScreen::renderFormDanhGia() {
    const char* cardTitle = formDanhGiaIdDangSua_.empty()
        ? u8"Đánh giá & Nhận xét chất lượng sân bóng"
        : u8"Chỉnh sửa nội dung đánh giá của bạn";

    Widgets::Card("card_renter_write_review", cardTitle, [this]() {
        if (danhSachSanDaThue_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::TextWrapped(u8"Bạn chỉ có thể gửi đánh giá cho những sân bóng mà bạn đã hoàn tất đặt và thi đấu.");
            ImGui::PopStyleColor();
            return;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Chia sẻ cảm nhận thực tế của bạn về chất lượng sân bãi, dàn đèn chiếu sáng, mặt cỏ và dịch vụ:");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // 1. Chọn sân bóng đã hoàn thành thi đấu
        std::vector<std::string> dsTenSan;
        for (const auto& s : danhSachSanDaThue_) {
            dsTenSan.push_back(s.maSan + " - " + s.tenSan + " (" + std::to_string(s.loaiSan) + u8" người)");
        }
        if (formIndexSan_ >= static_cast<int>(dsTenSan.size())) formIndexSan_ = 0;
        Widgets::Combo(u8"Sân bóng bạn đã trải nghiệm (*)", "##r_review_pitch", formIndexSan_, dsTenSan);
        ImGui::Spacing();

        // 2. Chấm số sao bằng StarRating (chỉnh được)
        Widgets::StarRating(
            u8"Mức độ hài lòng của bạn (*):",
            "star_renter_interactive",
            formSoSao_,
            false, // false: chỉnh được
            28.0f
        );
        ImGui::Spacing();

        // 3. Ô nhận xét nhiều dòng
        ImGui::TextUnformatted(u8"Nhận xét chi tiết (*):");
        char buf[1024];
        std::snprintf(buf, sizeof(buf), "%s", formNoiDung_.c_str());
        if (ImGui::InputTextMultiline("##r_review_text", buf, sizeof(buf), ImVec2(-1.0f, 90.0f))) {
            formNoiDung_ = buf;
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Nút Gửi / Cập nhật
        if (formDanhGiaIdDangSua_.empty()) {
            if (Widgets::NutChinh(u8"Gửi đánh giá & Nhận xét", ImVec2(240.0f, 36.0f))) {
                if (formNoiDung_.empty()) {
                    Widgets::ToastLoi(u8"Vui lòng nhập nội dung nhận xét!");
                    return;
                }

                User* curUser = GUI::App::Session::getInstance().getUser();
                std::string sdt = curUser ? curUser->get_phone_number() : "0909876543";
                std::string ten = curUser ? curUser->get_name() : u8"Người thuê";
                std::string maSan = danhSachSanDaThue_[formIndexSan_].maSan;

                auto kq = GUI::App::Facade::getInstance().guiHoacSuaDanhGia(
                    sdt, ten, maSan, formSoSao_, formNoiDung_, ""
                );

                if (kq.ok) {
                    Widgets::ToastThanhCong(u8"Cảm ơn bạn đã gửi đánh giá cho sân " + maSan + "!");
                    formNoiDung_.clear();
                    formSoSao_ = 5;
                    reloadViewModel();
                } else {
                    Widgets::ToastLoi(kq.loi);
                }
            }
        } else {
            if (Widgets::NutChinh(u8"Lưu cập nhật đánh giá", ImVec2(200.0f, 36.0f))) {
                if (formNoiDung_.empty()) {
                    Widgets::ToastLoi(u8"Nội dung nhận xét không được để trống!");
                    return;
                }

                User* curUser = GUI::App::Session::getInstance().getUser();
                std::string sdt = curUser ? curUser->get_phone_number() : "0909876543";
                std::string ten = curUser ? curUser->get_name() : u8"Người thuê";
                std::string maSan = danhSachSanDaThue_[formIndexSan_].maSan;

                auto kq = GUI::App::Facade::getInstance().guiHoacSuaDanhGia(
                    sdt, ten, maSan, formSoSao_, formNoiDung_, formDanhGiaIdDangSua_
                );

                if (kq.ok) {
                    Widgets::ToastThanhCong(u8"Đã cập nhật đánh giá thành công!");
                    formDanhGiaIdDangSua_.clear();
                    formNoiDung_.clear();
                    formSoSao_ = 5;
                    reloadViewModel();
                } else {
                    Widgets::ToastLoi(kq.loi);
                }
            }

            ImGui::SameLine(0.0f, 12.0f);

            if (Widgets::NutPhu(u8"Hủy bỏ sửa", ImVec2(120.0f, 36.0f))) {
                formDanhGiaIdDangSua_.clear();
                formNoiDung_.clear();
                formSoSao_ = 5;
            }
        }
    });
}

void DanhGiaScreen::renderDanhSachDanhGiaCuaToi() {
    Widgets::Card("card_renter_my_reviews", u8"Các đánh giá bạn đã gửi", [this]() {
        if (danhSachDanhGiaCuaToi_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::TextUnformatted(u8"Bạn chưa gửi đánh giá nào gần đây.");
            ImGui::PopStyleColor();
            return;
        }

        for (const auto& dg : danhSachDanhGiaCuaToi_) {
            ImGui::BeginGroup();

            Widgets::Badge((u8"Sân " + dg.maSan).c_str(), GUI::Theme::Colors::GrassGreen, GUI::Theme::Colors::WhiteCard);
            ImGui::SameLine(0.0f, 8.0f);

            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::Text(u8"Ngày gửi: %s", dg.ngayGui.c_str());
            ImGui::PopStyleColor();

            ImGui::SameLine(0.0f, 16.0f);

            // Nút Chỉnh sửa đánh giá cũ của mình
            std::string btnEditId = u8"Chỉnh sửa##dg_" + dg.id;
            if (Widgets::NutPhu(btnEditId.c_str(), ImVec2(80.0f, 22.0f))) {
                formDanhGiaIdDangSua_ = dg.id;
                formSoSao_ = dg.soSao;
                formNoiDung_ = dg.noiDung;

                for (size_t k = 0; k < danhSachSanDaThue_.size(); ++k) {
                    if (danhSachSanDaThue_[k].maSan == dg.maSan) {
                        formIndexSan_ = static_cast<int>(k);
                        break;
                    }
                }
                Widgets::ToastThongTin(u8"Đã nạp đánh giá để chỉnh sửa!");
            }

            ImGui::Spacing();

            // Hiển thị số sao (chỉ xem)
            Widgets::StarRating(
                nullptr,
                ("my_star_" + dg.id).c_str(),
                const_cast<int&>(dg.soSao),
                true, // readonly
                18.0f
            );

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
            ImGui::TextWrapped(u8"\"%s\"", dg.noiDung.c_str());
            ImGui::PopStyleColor();

            ImGui::EndGroup();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
    });
}

}  // namespace GUI::Screens::Renter
