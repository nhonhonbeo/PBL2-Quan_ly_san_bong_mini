#include "GUI/Screens/Renter/DatSanScreen.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <utility>

#include <imgui.h>

#include "Core/DateTime.h"
#include "GUI/App/Facade.h"
#include "GUI/Theme/Fonts.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/DatePicker.h"
#include "GUI/Widgets/ProgressBar.h"
#include "GUI/Widgets/Toast.h"
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
            }
        }
        if (!selectedFieldStillExists) {
            maSanChon_.clear();
            tenSanChon_.clear();
            cacCaDangChon_.clear();
            capNhatTongTien();
        }
        reloadSlots();
    }
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
    const char* steps[] = {u8"Chọn cơ sở", u8"Chọn sân", u8"Chọn giờ", u8"Xác nhận"};
    using C = GUI::Theme::Colors;
    const float width = ImGui::GetContentRegionAvail().x;
    const float cell = width / 4.f;
    const ImVec2 start = ImGui::GetCursorScreenPos();
    auto* dl = ImGui::GetWindowDrawList();
    const ImVec2 labelSize = ImGui::CalcTextSize("1");
    for (int i = 0; i < 4; ++i) {
        const ImVec2 center(start.x + cell * (i + .5f), start.y + 15);
        if (i < 3) dl->AddLine(ImVec2(center.x+16,center.y), ImVec2(center.x+cell-16,center.y), C::toU32(i+1<buocHienTai_?C::Primary:C::Border), 2);
        const bool active = i+1 <= buocHienTai_;
        dl->AddCircleFilled(center, 14, C::toU32(active?C::Primary:C::Border));
        const std::string number = std::to_string(i+1);
        dl->AddText(ImVec2(center.x-labelSize.x*.5f,center.y-labelSize.y*.5f), C::toU32(active?C::Surface:C::TextSecondary), number.c_str());
        const ImVec2 size = ImGui::CalcTextSize(steps[i]);
        dl->AddText(ImVec2(center.x-size.x*.5f,start.y+36), C::toU32(i+1==buocHienTai_?C::PrimaryDark:C::TextSecondary), steps[i]);
    }
    ImGui::Dummy(ImVec2(width, 58));
}

} // namespace GUI::Screens::Renter
