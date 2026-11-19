#include "GUI/Screens/Renter/DanhGiaScreen.h"

#include <algorithm>
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
#include "GUI/Widgets/Layout.h"
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

    // 1. Nạp toàn bộ sân và giữ danh sách hoàn tất để áp dụng điều kiện gửi đánh giá.
    danhSachSan_ = facade.getDanhSachSanBong();
    danhSachSanDaThue_ = facade.getDanhSachSanDaThueXong(sdt);

    // 2. Nạp danh sách các đánh giá đã gửi của tôi
    danhSachDanhGiaCuaToi_ = facade.getDanhGiaCuaKhach(sdt);

    // 3. Tên cơ sở theo chủ sân, để dropdown hiển thị tên dễ hiểu thay vì mã nội bộ.
    tenCoSoTheoChuSan_.clear();
    LinkedList<VenueFacilityRecord> coSo;
    std::string loi;
    if (facade.getDanhSachCoSo(coSo, loi)) {
        for (auto* n = coSo.get_head(); n != nullptr; n = n->next) {
            tenCoSoTheoChuSan_[n->data.ownerId] = n->data.name;
        }
    }
}

void DanhGiaScreen::render() {
    const float rongKhaDung = ImGui::GetContentRegionAvail().x;
    constexpr float kNguongHaiCot = 1100.0f;

    if (rongKhaDung >= kNguongHaiCot) {
        // Màn hình rộng: form bên trái, các đánh giá đã gửi bên phải (không để form trải 1600px).
        constexpr float kKhoangCach = 16.0f;
        const float rongTrai = static_cast<float>(static_cast<int>((rongKhaDung - kKhoangCach) * 0.55f));
        Widgets::BeginCot("##danhgia_cot_form", rongTrai);
        renderFormDanhGia();
        Widgets::EndCot();

        ImGui::SameLine(0.0f, kKhoangCach);
        Widgets::BeginCot("##danhgia_cot_lichsu", rongKhaDung - rongTrai - kKhoangCach);
        renderDanhSachDanhGiaCuaToi();
        Widgets::EndCot();
    } else {
        Widgets::BeginNoiDungHep("##danhgia_cot_don", 820.0f);
        renderFormDanhGia();
        renderDanhSachDanhGiaCuaToi();
        Widgets::EndNoiDungHep();
    }
}

void DanhGiaScreen::renderFormDanhGia() {
    const char* cardTitle = formDanhGiaIdDangSua_.empty()
        ? ""  // tiêu đề trang đã có ở thanh trên
        : u8"Chỉnh sửa nội dung đánh giá của bạn";

    Widgets::Card("card_renter_write_review", cardTitle, [this]() {
        if (danhSachSan_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::TextWrapped(u8"Hiện chưa có sân bóng nào trong hệ thống.");
            ImGui::PopStyleColor();
            return;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Chia sẻ cảm nhận thực tế của bạn về chất lượng sân bãi, dàn đèn chiếu sáng, mặt cỏ và dịch vụ:");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Spacing();

        // 1. Chọn từ danh sách sân đầy đủ; quyền gửi vẫn giới hạn ở sân đã hoàn tất thi đấu.
        std::vector<std::string> dsTenSan;
        for (const auto& s : danhSachSan_) {
            // Hiển thị "Sân 5 người - Tên cơ sở" thay vì mã nội bộ; mã chỉ dùng để định danh.
            const auto it = tenCoSoTheoChuSan_.find(s.ownerUserId);
            std::string nhan = s.tenSan;
            if (it != tenCoSoTheoChuSan_.end() && !it->second.empty()) {
                nhan += " - " + it->second;
            }
            dsTenSan.push_back(nhan);
        }
        if (formIndexSan_ >= static_cast<int>(dsTenSan.size())) {
            formIndexSan_ = 0;
        }
        Widgets::Combo(u8"Sân bóng bạn đã trải nghiệm (*)", "##r_review_pitch", formIndexSan_, dsTenSan);
        const std::string& maSanChon = danhSachSan_[static_cast<size_t>(formIndexSan_)].maSan;
        bool duDieuKienDanhGia = std::any_of(
            danhSachSanDaThue_.begin(),
            danhSachSanDaThue_.end(),
            [&maSanChon](const GUI::App::SanBongInfo& san) {
                return san.maSan == maSanChon;
            }
        );
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
        ImGui::PushStyleColor(ImGuiCol_FrameBg, GUI::Theme::Colors::WhiteCard);
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::InputBorder);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
        if (ImGui::InputTextMultiline("##r_review_text", buf, sizeof(buf), ImVec2(-1.0f, 90.0f))) {
            formNoiDung_ = buf;
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);

        // Giải thích lý do nút Gửi đang bị mờ để người dùng biết cần làm gì.
        if (!duDieuKienDanhGia) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::TextUnformatted(u8"Chỉ có thể đánh giá sân đã hoàn tất thi đấu.");
            ImGui::PopStyleColor();
        } else if (formNoiDung_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::TextUnformatted(u8"Hãy viết nhận xét để có thể gửi đánh giá.");
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Nút Gửi / Cập nhật
        if (formDanhGiaIdDangSua_.empty()) {
            ImGui::BeginDisabled(!duDieuKienDanhGia || formNoiDung_.empty());
            if (Widgets::NutChinh(u8"Gửi đánh giá & Nhận xét", ImVec2(240.0f, 36.0f))) {
                User* curUser = GUI::App::Session::getInstance().getUser();
                std::string sdt = curUser ? curUser->get_phone_number() : "0909876543";
                std::string ten = curUser ? curUser->get_name() : u8"Người thuê";
                std::string maSan = maSanChon;

                auto kq = GUI::App::Facade::getInstance().guiHoacSuaDanhGia(
                    sdt, ten, maSan, formSoSao_, formNoiDung_, ""
                );

                if (kq.ok) {
                    formNoiDung_.clear();
                    formSoSao_ = 5;
                    reloadViewModel();
                    Widgets::ToastThanhCong(u8"Đã gửi đánh giá. Cảm ơn bạn!");
                } else {
                    Widgets::ToastLoi(kq.loi.empty() ? u8"Không thể gửi đánh giá." : kq.loi);
                }
            }
            ImGui::EndDisabled();
        } else {
            ImGui::BeginDisabled(!duDieuKienDanhGia || formNoiDung_.empty());
            if (Widgets::NutChinh(u8"Lưu cập nhật đánh giá", ImVec2(200.0f, 36.0f))) {
                User* curUser = GUI::App::Session::getInstance().getUser();
                std::string sdt = curUser ? curUser->get_phone_number() : "0909876543";
                std::string ten = curUser ? curUser->get_name() : u8"Người thuê";
                std::string maSan = maSanChon;

                auto kq = GUI::App::Facade::getInstance().guiHoacSuaDanhGia(
                    sdt, ten, maSan, formSoSao_, formNoiDung_, formDanhGiaIdDangSua_
                );

                if (kq.ok) {
                    formDanhGiaIdDangSua_.clear();
                    formNoiDung_.clear();
                    formSoSao_ = 5;
                    reloadViewModel();
                    Widgets::ToastThanhCong(u8"Đã cập nhật đánh giá.");
                } else {
                    Widgets::ToastLoi(kq.loi.empty() ? u8"Không thể cập nhật đánh giá." : kq.loi);
                }
            }
            ImGui::EndDisabled();

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

            Widgets::BadgeTrong((u8"Sân " + dg.maSan).c_str());
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

                for (size_t k = 0; k < danhSachSan_.size(); ++k) {
                    if (danhSachSan_[k].maSan == dg.maSan) {
                        formIndexSan_ = static_cast<int>(k);
                        break;
                    }
                }
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
