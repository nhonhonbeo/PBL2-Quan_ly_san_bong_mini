#include "GUI/Screens/Renter/DatSanScreen.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <utility>

#include <imgui.h>

#include "Core/DateTime.h"
#include "GUI/App/Facade.h"
#include "GUI/App/Session.h"
#include "GUI/Theme/Fonts.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/DatePicker.h"
#include "GUI/Widgets/ProgressBar.h"
#include "GUI/Widgets/Toast.h"
#include "features/Account/User.h"
#include "GUI/App/ViewFormat.h"

namespace GUI::Screens::Renter {

using namespace GUI::App::ViewFormat;

DatSanScreen::DatSanScreen(std::function<void()> onXemLichCuaToi)
    : onXemLichCuaToi_(std::move(onXemLichCuaToi)) {}

void DatSanScreen::onEnter() {
    buocHienTai_ = 1;
    ngayChon_ = ngayHienThi(DateTime::today());
    ownerIdChon_.clear();
    maSanChon_.clear();
    khungGioChon_.clear();
    cacCaDangChon_.clear();
    maDonDaDat_.clear();
    reloadViewModel();
}

void DatSanScreen::onExit() {}

void DatSanScreen::reloadViewModel() {
    GUI::App::Facade& facade = GUI::App::Facade::getInstance();
    User* user = GUI::App::Session::getInstance().getUser();
    biChanDatSan_ = user != nullptr &&
                    facade.laKhachHangBiChan(user->get_phone_number());
    danhSachCoSo_.clear();
    soSanTheoCoSo_.clear();
    giaThapNhatTheoCoSo_.clear();
    loaiSanCoSan_.clear();
    loiTaiTaiDuLieu_.clear();
    if (!facade.getDanhSachCoSo(danhSachCoSo_, loiTaiTaiDuLieu_)) return;

    for (auto* facility = danhSachCoSo_.get_head();
         facility != nullptr; facility = facility->next) {
        LinkedList<VenueFieldRecord> fields;
        if (!facade.getDanhSachSanTheoChuSan(
                facility->data.ownerId, 0, false, fields,
                loiTaiTaiDuLieu_)) {
            return;
        }
        int fieldCount = 0;
        for (auto* field = fields.get_head(); field != nullptr;
             field = field->next) {
            ++fieldCount;
        }
        soSanTheoCoSo_[facility->data.ownerId] = fieldCount;
        long long lowestPrice = 0;
        if (!facade.getGiaThapNhatCoSo(
                facility->data.ownerId, lowestPrice, loiTaiTaiDuLieu_)) {
            return;
        }
        giaThapNhatTheoCoSo_[facility->data.ownerId] = lowestPrice;
    }

    if (!ownerIdChon_.empty()) {
        for (auto* facility = danhSachCoSo_.get_head();
             facility != nullptr; facility = facility->next) {
            if (facility->data.ownerId != ownerIdChon_) continue;
            choPhep60Phut_ = facility->data.allows60Minutes;
            choPhep90Phut_ = facility->data.allows90Minutes;
            break;
        }
        if ((thoiLuongChon_ == 60 && !choPhep60Phut_) ||
            (thoiLuongChon_ == 90 && !choPhep90Phut_)) {
            thoiLuongChon_ = choPhep60Phut_ ? 60 : 90;
        }
        if (!facade.getDanhSachSanTheoChuSan(
                ownerIdChon_, 0, false, danhSachSan_, loiTaiTaiDuLieu_)) {
            return;
        }
        loaiSanCoSan_.clear();
        bool selectedFieldStillExists = false;
        for (auto* field = danhSachSan_.get_head(); field != nullptr;
             field = field->next) {
            loaiSanCoSan_[field->data.size] = true;
            if (field->data.fieldId == maSanChon_) {
                selectedFieldStillExists = true;
                tenSanChon_ = field->data.name;
                loaiSanChon_ = field->data.size;
                break;
            }
        }
        if (!selectedFieldStillExists) {
            maSanChon_.clear();
            tenSanChon_.clear();
        }
        reloadSlots();
    }
    if (user != nullptr) {
        phanTramGiam_ = facade.getMucUuDaiCuaKhach(user->get_phone_number());
    }
    soTienGiam_ = giaGoc_ * phanTramGiam_ / 100;
    tongTien_ = giaGoc_ - soTienGiam_;
}

void DatSanScreen::reloadSlots() {
    danhSachKhungGioTrong_.clear();
    loiChonCa_.clear();
    if (!DateTime::isValidDate(DateTime::toIsoDate(ngayChon_))) {
        loiTaiTaiDuLieu_ = u8"Ngày không đúng định dạng dd/MM/yyyy.";
        cacCaDangChon_.clear();
        capNhatTongTien();
        return;
    }
    if (DateTime::isPastDate(DateTime::toIsoDate(ngayChon_))) {
        loiTaiTaiDuLieu_ = u8"Không thể đặt sân cho ngày đã qua.";
        cacCaDangChon_.clear();
        capNhatTongTien();
        return;
    }
    if (maSanChon_.empty() || ngayChon_.empty()) return;
    std::string error;
    if (!GUI::App::Facade::getInstance().getKhungGioSan(
            maSanChon_, ngayChon_, thoiLuongChon_,
            danhSachKhungGioTrong_, error)) {
        loiTaiTaiDuLieu_ = error;
        cacCaDangChon_.clear();
        capNhatTongTien();
        return;
    }
    loiTaiTaiDuLieu_.clear();
    const auto slotIsUnavailable = [this](const VenueSlotRecord& selected) {
        for (auto* node = danhSachKhungGioTrong_.get_head();
             node != nullptr; node = node->next) {
            if (node->data.startTime == selected.startTime &&
                node->data.available && !node->data.past) {
                return false;
            }
        }
        return true;
    };
    while (cacCaDangChon_.removeIf(slotIsUnavailable)) {}
    capNhatTongTien();
}

bool DatSanScreen::slotDangChon(const std::string& startTime) const {
    for (auto* node = cacCaDangChon_.get_head();
         node != nullptr; node = node->next) {
        if (node->data.startTime == startTime) return true;
    }
    return false;
}

void DatSanScreen::capNhatTongTien() {
    giaGoc_ = 0;
    for (auto* node = cacCaDangChon_.get_head();
         node != nullptr; node = node->next) {
        giaGoc_ += node->data.priceVnd;
    }
    soTienGiam_ = giaGoc_ * phanTramGiam_ / 100;
    tongTien_ = giaGoc_ - soTienGiam_;
    khungGioChon_.clear();
    for (auto* slot = danhSachKhungGioTrong_.get_head();
         slot != nullptr; slot = slot->next) {
        if (slotDangChon(slot->data.startTime)) {
            if (!khungGioChon_.empty()) khungGioChon_ += ", ";
            khungGioChon_ += slot->data.startTime;
        }
    }
}

void DatSanScreen::chonCoSo(const VenueFacilityRecord& facility) {
    ownerIdChon_ = facility.ownerId;
    tenCoSoChon_ = facility.name;
    diaChiCoSoChon_ = facility.address;
    choPhep60Phut_ = facility.allows60Minutes;
    choPhep90Phut_ = facility.allows90Minutes;
    thoiLuongChon_ = choPhep60Phut_ ? 60 : 90;
    maSanChon_.clear();
    tenSanChon_.clear();
    khungGioChon_.clear();
    cacCaDangChon_.clear();
    giaGoc_ = 0;
    loaiSanCoSan_.clear();
    if (GUI::App::Facade::getInstance().getDanhSachSanTheoChuSan(
            ownerIdChon_, 0, false, danhSachSan_, loiTaiTaiDuLieu_)) {
        for (auto* field = danhSachSan_.get_head(); field != nullptr;
             field = field->next) {
            loaiSanCoSan_[field->data.size] = true;
        }
        buocHienTai_ = 2;
    }
}

void DatSanScreen::render() {
    if (biChanDatSan_) {
        renderCanhBaoDanhSachDen();
        return;
    }
    if (!loiTaiTaiDuLieu_.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Danger);
        ImGui::TextWrapped(u8"[!] %s", loiTaiTaiDuLieu_.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }
    if (buocHienTai_ <= 4) {
        renderThanhTienTrinh();
        ImGui::Spacing();
    }
    switch (buocHienTai_) {
        case 1: renderBuoc1ChonCoSo(); break;
        case 2: renderBuoc2ChonSan(); break;
        case 3: renderBuoc3ChonNgayVaKhungGio(); break;
        case 4: renderBuoc4XacNhan(); break;
        case 5: renderPhieuDatSan(); break;
        default: break;
    }
}

void DatSanScreen::renderThanhTienTrinh() {
    // L\u00DD DO CH\u1ECCN: Stepper tr\u1EF1c quan 4 b\u01B0\u1EDBc chu\u1EA9n UX, lo\u1EA1i b\u1ECF ho\u00E0n to\u00E0n c\u00E1c chu\u1ED7i l\u1EB7p "B\u01B0\u1EDBc X",
    // hi\u1EC3n th\u1ECB r\u00F5 b\u01B0\u1EDBc \u0111\u00E3 xong (t\u00EDch xanh), b\u01B0\u1EDBc hi\u1EC7n t\u1EA1i (n\u1EC1n n\u1ED5i b\u1EADt) v\u00E0 b\u01B0\u1EDBc ti\u1EBFp theo.
    const char* steps[] = {
        u8"1. Ch\u1ECDn c\u1EE5m s\u00E2n",
        u8"2. Ch\u1ECDn s\u00E2n & lo\u1EA1i s\u00E2n",
        u8"3. Ch\u1ECDn ng\u00E0y & ca \u0111\u00E1",
        u8"4. X\u00E1c nh\u1EADn & \u01AFu \u0111\u00E3i"
    };

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 16.0f);
    ImGui::Columns(4, nullptr, false);

    for (int i = 0; i < 4; ++i) {
        const bool isDone = (buocHienTai_ > i + 1);
        const bool isCurrent = (buocHienTai_ == i + 1);

        if (isCurrent) {
            ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::PrimaryLight);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::PrimaryLight);
            ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::PrimaryDark);
            ImGui::PushStyleColor(ImGuiCol_Border,        GUI::Theme::Colors::Primary);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
        } else if (isDone) {
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.95f, 0.98f, 0.96f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.98f, 0.96f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::Success);
            ImGui::PushStyleColor(ImGuiCol_Border,        GUI::Theme::Colors::Success);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.96f, 0.96f, 0.96f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.96f, 0.96f, 0.96f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::TextSecondary);
            ImGui::PushStyleColor(ImGuiCol_Border,        GUI::Theme::Colors::Border);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        }

        std::string label = isDone ? (std::string(u8"\u2713 ") + steps[i]) : steps[i];
        ImGui::Button(label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 32.0f));

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::PopStyleVar();

    Widgets::ProgressBar(
        static_cast<float>(buocHienTai_) / 4.0f,
        "",
        ImVec2(-1.0f, 6.0f),
        GUI::Theme::Colors::Primary
    );
}

void DatSanScreen::renderCanhBaoDanhSachDen() {
    Widgets::Card(
        "renter_blacklist_notice",
        u8"T\u00E0i kho\u1EA3n b\u1ECB gi\u1EDBi h\u1EA1n \u0111\u1EB7t s\u00E2n",
        []() {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Danger);
            ImGui::TextWrapped(
                u8"T\u00E0i kho\u1EA3n c\u1EE7a b\u1EA1n hi\u1EC7n \u0111ang n\u1EB1m trong danh s\u00E1ch h\u1EA1n ch\u1EBF \u0111\u1EB7t s\u00E2n do h\u1EE7y \u0111\u01A1n nhi\u1EC1u l\u1EA7n ho\u1EB7c vi ph\u1EA1m quy ch\u1EBF.\n"
                u8"Vui l\u00F2ng li\u00EAn h\u1EC7 tr\u1EF1c ti\u1EBFp ban qu\u1EA3n tr\u1ECB c\u1EE5m s\u00E2n b\u00F3ng \u0111\u1EC3 \u0111\u01B0\u1EE3c h\u1ED7 tr\u1EE3 m\u1EDF kh\u00F3a."
            );
            ImGui::PopStyleColor();
        }
    );
}

}  // namespace GUI::Screens::Renter
