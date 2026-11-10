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

namespace GUI::Screens::Renter {

namespace {

std::string dinhDangTien(long long soTien) {
    std::string text = std::to_string(soTien);
    for (int index = static_cast<int>(text.size()) - 3; index > 0; index -= 3) {
        text.insert(static_cast<std::size_t>(index), ".");
    }
    return text + u8" VNĐ";
}

std::string ngayHienThi(const std::string& isoDate) {
    if (DateTime::isValidDate(isoDate)) {
        return isoDate.substr(8, 2) + "/" + isoDate.substr(5, 2) + "/" +
               isoDate.substr(0, 4);
    }
    return isoDate;
}

}  // namespace

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

void DatSanScreen::renderBuoc1ChonCoSo() {
    Widgets::Card("choose_facility", u8"Ch\u1ECDn c\u1EE5m s\u00E2n th\u1EC3 thao", [this]() {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, GUI::Theme::Colors::Surface);
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::InputBorder);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 7.0f));

        ImGui::AlignTextToFramePadding();
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Primary);
        ImDrawList* searchDl = ImGui::GetWindowDrawList(); ImVec2 searchIconPos = ImGui::GetCursorScreenPos(); ImU32 searchIconCol = GUI::Theme::Colors::toU32(GUI::Theme::Colors::Primary); searchDl->AddCircle(ImVec2(searchIconPos.x + 10.0f, searchIconPos.y + 12.0f), 5.5f, searchIconCol, 16, 1.8f); searchDl->AddLine(ImVec2(searchIconPos.x + 14.0f, searchIconPos.y + 16.0f), ImVec2(searchIconPos.x + 19.0f, searchIconPos.y + 21.0f), searchIconCol, 2.0f); ImGui::Dummy(ImVec2(22.0f, 24.0f));
        ImGui::PopStyleColor();
        ImGui::SameLine(0.0f, 8.0f);

        char search[256]{};
        const std::size_t count = tuKhoaCoSo_.size() < sizeof(search) - 1 ? tuKhoaCoSo_.size() : sizeof(search) - 1;
        std::memcpy(search, tuKhoaCoSo_.data(), count);
        ImGui::SetNextItemWidth(380.0f);
        if (ImGui::InputTextWithHint("##search_facility", u8"T\u00ECm ki\u1EBFm c\u1EE5m s\u00E2n theo t\u00EAn ho\u1EB7c \u0111\u1ECBa ch\u1EC9...", search, sizeof(search))) {
            tuKhoaCoSo_ = search;
        }

        if (!tuKhoaCoSo_.empty()) {
            ImGui::SameLine(0.0f, 8.0f);
            if (ImGui::Button(u8"\u2715 X\u00F3a l\u1ECDc##clear_facility_search")) {
                tuKhoaCoSo_.clear();
            }
        }

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);

        int matchingCount = 0;
        for (auto* node = danhSachCoSo_.get_head(); node != nullptr; node = node->next) {
            const VenueFacilityRecord& facility = node->data;
            if (!tuKhoaCoSo_.empty() &&
                facility.name.find(tuKhoaCoSo_) == std::string::npos &&
                facility.address.find(tuKhoaCoSo_) == std::string::npos) {
                continue;
            }
            ++matchingCount;
        }

        ImGui::SameLine(0.0f, 18.0f);
        ImGui::AlignTextToFramePadding();
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
        ImGui::Text(u8"Hi\u1EC3n th\u1ECB %d c\u1EE5m s\u00E2n s\u1EB5n c\u00F3", matchingCount);
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        if (matchingCount == 0) {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
            ImGui::TextUnformatted(u8"Kh\u00F4ng t\u00ECm th\u1EA5y c\u1EE5m s\u00E2n n\u00E0o ph\u00F9 h\u1EE3p v\u1EDBi t\u1EEB kh\u00F3a t\u00ECm ki\u1EBFm. Vui l\u00F2ng th\u1EED l\u1EA1i!");
            ImGui::PopStyleColor();
            ImGui::Spacing();
            return;
        }

        float availableWidth = ImGui::GetContentRegionAvail().x;
        int numCols = static_cast<int>(availableWidth / 330.0f);
        if (numCols < 1) numCols = 1;
        if (numCols > 4) numCols = 4;

        if (ImGui::BeginTable("##cum_san_grid_table", numCols, ImGuiTableFlags_SizingStretchSame)) {
            for (auto* node = danhSachCoSo_.get_head(); node != nullptr; node = node->next) {
                const VenueFacilityRecord& facility = node->data;
                if (!tuKhoaCoSo_.empty() &&
                    facility.name.find(tuKhoaCoSo_) == std::string::npos &&
                    facility.address.find(tuKhoaCoSo_) == std::string::npos) {
                    continue;
                }

                ImGui::TableNextColumn();
                renderTheCumSan(facility, soSanTheoCoSo_[facility.ownerId], giaThapNhatTheoCoSo_[facility.ownerId]);
            }
            ImGui::EndTable();
        }
    });
}

void DatSanScreen::renderTheCumSan(const VenueFacilityRecord& facility, int fieldCount, long long lowestPrice) {
    ImGui::PushID(facility.ownerId.c_str());
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 12.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, GUI::Theme::Colors::Surface);
    ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::Border);

    std::string childId = "##card_" + facility.ownerId;
    if (ImGui::BeginChild(childId.c_str(), ImVec2(0.0f, 355.0f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        ImVec2 bannerP0 = ImGui::GetCursorScreenPos();
        float bannerWidth = ImGui::GetContentRegionAvail().x;
        float bannerHeight = 44.0f;
        ImVec2 bannerP1(bannerP0.x + bannerWidth, bannerP0.y + bannerHeight);

        drawList->AddRectFilledMultiColor(bannerP0, bannerP1, IM_COL32(22, 134, 91, 255), IM_COL32(16, 96, 64, 255), IM_COL32(16, 96, 64, 255), IM_COL32(22, 134, 91, 255));

        float midX = bannerP0.x + bannerWidth * 0.5f;
        float midY = bannerP0.y + bannerHeight * 0.5f;
        drawList->AddLine(ImVec2(midX, bannerP0.y), ImVec2(midX, bannerP1.y), IM_COL32(255, 255, 255, 80), 1.2f);
        drawList->AddCircle(ImVec2(midX, midY), 13.0f, IM_COL32(255, 255, 255, 80), 24, 1.2f);
        drawList->AddRect(bannerP0, bannerP1, IM_COL32(255, 255, 255, 60), 4.0f, 0, 1.0f);

        ImGui::SetCursorScreenPos(ImVec2(bannerP0.x + 8.0f, bannerP0.y + 12.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_WHITE);
        ImGui::TextUnformatted(u8"\u26BD C\u1EE4M S\u00C2N MINI CHU\u1EA8N FIFA");
        ImGui::PopStyleColor();

        ImGui::SetCursorScreenPos(ImVec2(bannerP0.x, bannerP1.y + 8.0f));
        ImFont* titleFont = GUI::Theme::getFonts().title;
        if (titleFont != nullptr) ImGui::PushFont(titleFont);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
        ImGui::TextUnformatted(facility.name.c_str());
        ImGui::PopStyleColor();
        if (titleFont != nullptr) ImGui::PopFont();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.55f, 0.05f, 1.0f));
        ImGui::TextUnformatted(u8"\u2605 4.9");
        ImGui::PopStyleColor();
        ImGui::SameLine(0.0f, 4.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
        ImGui::TextUnformatted(u8"(48 l\u01B0\u1EE3t \u0111\u00E1nh gi\u00E1)");
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
        ImGui::TextWrapped(u8"• Địa chỉ: %s", facility.address.c_str());
        ImGui::TextUnformatted(u8"• Khoảng cách: ~1.5 km (gần bạn)");

        std::string gioMo = facility.openTime.empty() ? "06:00" : facility.openTime;
        std::string gioDong = facility.closeTime.empty() ? "23:00" : facility.closeTime;
        ImGui::Text(u8"• Giờ mở cửa: %s - %s", gioMo.c_str(), gioDong.c_str());
        ImGui::TextUnformatted(u8"• Tiện ích: Đèn LED • Bãi xe • Căn tin");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        if (fieldCount == 0) {
            Widgets::BadgeBaoTri(u8"Ch\u01B0a c\u00F3 s\u00E2n s\u1EB5n s\u00E0ng");
        } else {
            Widgets::BadgeTrong((std::to_string(fieldCount) + u8" s\u00E2n s\u1EB5n s\u00E0ng").c_str());
            ImGui::SameLine(0.0f, 6.0f);
            Widgets::BadgeVIP((u8"T\u1EEB " + dinhDangTien(lowestPrice)).c_str());
        }

        ImGui::Spacing();

        if (fieldCount == 0) {
            ImGui::BeginDisabled(true);
            Widgets::NutPhu(u8"T\u1EA1m ng\u01B0ng \u0111\u00F3n kh\u00E1ch", ImVec2(ImGui::GetContentRegionAvail().x, 34.0f));
            ImGui::EndDisabled();
        } else {
            if (Widgets::NutChinh(u8"Ch\u1ECDn c\u1EE5m s\u00E2n n\u00E0y \u2192", ImVec2(ImGui::GetContentRegionAvail().x, 34.0f))) {
                chonCoSo(facility);
            }
        }
    }
    ImGui::EndChild();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
    ImGui::PopID();
}

void DatSanScreen::renderBuoc2ChonSan() {
    Widgets::Card("choose_field", u8"Bước 2: Chọn loại sân và sân bóng cụ thể", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
        ImGui::Text(u8"Cơ sở đang chọn: %s", tenCoSoChon_.c_str());
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
        ImGui::Text(u8"Địa chỉ: %s", diaChiCoSoChon_.c_str());
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::TextUnformatted(u8"Chọn loại sân bóng:");
        ImGui::Spacing();

        for (const int size : {5, 7, 11}) {
            if (!loaiSanCoSan_[size]) continue;

            bool isSelectedSize = (loaiSanChon_ == size);
            if (isSelectedSize) {
                ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::PrimaryLight);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::PrimaryLight);
                ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::PrimaryDark);
                ImGui::PushStyleColor(ImGuiCol_Border,        GUI::Theme::Colors::Primary);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::Surface);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::GrayHover);
                ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::TextPrimary);
                ImGui::PushStyleColor(ImGuiCol_Border,        GUI::Theme::Colors::InputBorder);
            }
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, isSelectedSize ? 2.0f : 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

            std::string btnSizeLabel = u8"Sân " + std::to_string(size) + u8" người";
            if (ImGui::Button(btnSizeLabel.c_str(), ImVec2(160.0f, 38.0f))) {
                loaiSanChon_ = size;
                maSanChon_.clear();
                tenSanChon_.clear();
            }
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(4);

            ImGui::SameLine(0.0f, 10.0f);
        }

        ImGui::NewLine();
        ImGui::Spacing();
        ImGui::TextUnformatted(u8"Danh sách sân bóng thuộc loại đã chọn:");
        ImGui::Spacing();

        int shown = 0;
        for (auto* node = danhSachSan_.get_head(); node != nullptr;
             node = node->next) {
            const VenueFieldRecord& field = node->data;
            if (field.status != 0 || field.size != loaiSanChon_) continue;
            ++shown;
            ImGui::PushID(field.fieldId.c_str());
            const bool selected = (maSanChon_ == field.fieldId);

            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::Primary);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::PrimaryHover);
                ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::Surface);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::Surface);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::PrimaryLight);
                ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::TextPrimary);
            }
            ImGui::PushStyleColor(ImGuiCol_Border, selected ? GUI::Theme::Colors::Primary : GUI::Theme::Colors::InputBorder);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

            std::string fieldBtnLabel = field.name + " (" + field.fieldId + ")";
            if (ImGui::Button(fieldBtnLabel.c_str(), ImVec2(240.0f, 40.0f))) {
                maSanChon_ = field.fieldId;
                tenSanChon_ = field.name;
                loaiSanChon_ = field.size;
            }

            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(4);
            ImGui::PopID();
            ImGui::SameLine(0.0f, 10.0f);
        }

        ImGui::NewLine();
        if (shown == 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
            ImGui::TextUnformatted(u8"Loại sân này hiện chưa có sân nào trống sẵn sàng phục vụ.");
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        if (!maSanChon_.empty()) {
            ImGui::Spacing();
            std::string imgPath = GUI::App::Facade::getInstance().getAnhSan(maSanChon_);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.96f, 0.98f, 0.96f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
            if (ImGui::BeginChild("##field_photo_preview_box", ImVec2(0.0f, 66.0f), true)) {
                ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::PrimaryDark);
                ImGui::Text(u8"Thông tin sân đã chọn: %s (%s)", tenSanChon_.c_str(), maSanChon_.c_str());
                ImGui::PopStyleColor();

                if (!imgPath.empty()) {
                    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Success);
                    ImGui::Text(u8"Ảnh thực tế: %s (Mặt sân cỏ đạt chuẩn chất lượng)", imgPath.c_str());
                    ImGui::PopStyleColor();
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
                    ImGui::TextUnformatted(u8"Chưa có ảnh thực tế riêng (áp dụng mặt cỏ tiêu chuẩn của cụm sân).");
                    ImGui::PopStyleColor();
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();

        if (Widgets::NutPhu(u8"Quay lại chọn cơ sở", ImVec2(180.0f, 38.0f))) {
            buocHienTai_ = 1;
        }

        if (!maSanChon_.empty()) {
            ImGui::SameLine(0.0f, 12.0f);
            if (Widgets::NutChinh(u8"Tiếp tục chọn ngày và ca đá", ImVec2(260.0f, 38.0f))) {
                buocHienTai_ = 3;
                reloadSlots();
            }
        }
    });
}

void DatSanScreen::renderBuoc3ChonNgayVaKhungGio() {
    Widgets::Card("choose_date_slot", u8"Bước 3: Chọn ngày thi đấu và ca đá", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
        ImGui::Text(u8"Cơ sở: %s  |  Sân: %s (Sân %d người)", tenCoSoChon_.c_str(), tenSanChon_.c_str(), loaiSanChon_);
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // Chọn thời lượng ca đá
        ImGui::TextUnformatted(u8"Thời lượng mỗi ca đá:");
        ImGui::SameLine(0.0f, 12.0f);

        int duration = thoiLuongChon_;
        if (choPhep60Phut_) {
            ImGui::RadioButton(u8"60 phút", &duration, 60);
        }
        if (choPhep90Phut_) {
            if (choPhep60Phut_) ImGui::SameLine(0.0f, 16.0f);
            ImGui::RadioButton(u8"90 phút", &duration, 90);
        }
        if (duration != thoiLuongChon_) {
            thoiLuongChon_ = duration;
            cacCaDangChon_.clear();
            reloadSlots();
        }

        ImGui::Spacing();

        // Chọn ngày đá
        ImGui::SetNextItemWidth(260.0f);
        if (Widgets::DatePicker(u8"Ngày đặt sân đá bóng:", "##renter_booking_date", ngayChon_)) {
            cacCaDangChon_.clear();
            reloadSlots();
        }

        ImGui::Spacing();
        const bool validDate = DateTime::isValidDate(DateTime::toIsoDate(ngayChon_));
        const bool pastDate = validDate && DateTime::isPastDate(DateTime::toIsoDate(ngayChon_));

        if (pastDate) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Danger);
            ImGui::TextUnformatted(u8"[!] Không thể đặt sân cho ngày đã qua trong quá khứ.");
            ImGui::PopStyleColor();
        } else if (!validDate) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Danger);
            ImGui::TextUnformatted(u8"[!] Vui lòng nhập ngày hợp lệ theo định dạng dd/MM/yyyy.");
            ImGui::PopStyleColor();
        } else if (danhSachKhungGioTrong_.isEmpty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
            ImGui::TextUnformatted(u8"Không có khung giờ nào còn trống cho lựa chọn ngày này.");
            ImGui::PopStyleColor();
        } else {
            ImGui::TextUnformatted(u8"Bấm chọn các ca đá (khung giờ liên tiếp):");
            ImGui::Spacing();

            for (auto* node = danhSachKhungGioTrong_.get_head(); node != nullptr; node = node->next) {
                const VenueSlotRecord& slot = node->data;
                ImGui::PushID(slot.startTime.c_str());

                const bool selected = slotDangChon(slot.startTime);

                std::string label = slot.startTime + " - " + slot.endTime;
                if (!slot.available) {
                    label += u8" (Đã đặt)";
                } else if (slot.past) {
                    label += u8" (Đã qua)";
                } else {
                    label += " | " + dinhDangTien(slot.priceVnd);
                }

                if (selected) {
                    ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::Primary);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::PrimaryHover);
                    ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::Surface);
                } else if (slot.available && !slot.past) {
                    ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::Surface);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::PrimaryLight);
                    ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::TextPrimary);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::Gray);
                }
                ImGui::PushStyleColor(ImGuiCol_Border, selected ? GUI::Theme::Colors::Primary : (slot.available && !slot.past ? GUI::Theme::Colors::Primary : GUI::Theme::Colors::Border));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

                ImGui::BeginDisabled(!slot.available || slot.past);
                if (ImGui::Button(label.c_str(), ImVec2(270.0f, 40.0f))) {
                    if (selected) {
                        cacCaDangChon_.removeIf([&slot](const VenueSlotRecord& current) {
                            return current.startTime == slot.startTime;
                        });
                        loiChonCa_.clear();
                    } else {
                        bool adjacent = cacCaDangChon_.isEmpty();
                        std::string earliestStart;
                        std::string latestEnd;
                        for (auto* chosen = cacCaDangChon_.get_head(); chosen != nullptr; chosen = chosen->next) {
                            if (earliestStart.empty() || chosen->data.startTime < earliestStart) {
                                earliestStart = chosen->data.startTime;
                            }
                            if (latestEnd.empty() || chosen->data.endTime > latestEnd) {
                                latestEnd = chosen->data.endTime;
                            }
                        }
                        adjacent = adjacent || slot.endTime == earliestStart || slot.startTime == latestEnd;
                        if (adjacent) {
                            cacCaDangChon_.push_back(slot);
                            loiChonCa_.clear();
                        } else {
                            loiChonCa_ = u8"Chỉ có thể chọn thêm các ca liền kề với các ca đã chọn.";
                        }
                    }
                    capNhatTongTien();
                }
                ImGui::EndDisabled();

                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(4);
                ImGui::PopID();
                ImGui::SameLine(0.0f, 10.0f);
            }
        }

        ImGui::NewLine();
        if (!loiChonCa_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Danger);
            ImGui::Text(u8"[!] %s", loiChonCa_.c_str());
            ImGui::PopStyleColor();
        }

        if (!cacCaDangChon_.isEmpty()) {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
            ImGui::Text(u8"[Đã chọn] %s", khungGioChon_.c_str());
            ImGui::Text(u8"Giá gốc: %s  |  Ưu đãi VIP: %d%% (-%s)  |  Tổng thanh toán: %s",
                        dinhDangTien(giaGoc_).c_str(), phanTramGiam_,
                        dinhDangTien(soTienGiam_).c_str(), dinhDangTien(tongTien_).c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        if (Widgets::NutPhu(u8"Quay lại chọn sân", ImVec2(190.0f, 38.0f))) {
            buocHienTai_ = 2;
        }

        ImGui::SameLine(0.0f, 12.0f);
        ImGui::BeginDisabled(cacCaDangChon_.isEmpty());
        if (Widgets::NutChinh(u8"Tiếp tục xác nhận đơn", ImVec2(220.0f, 38.0f))) {
            buocHienTai_ = 4;
        }
        ImGui::EndDisabled();
    });
}

void DatSanScreen::renderBuoc4XacNhan() {
    User* user = GUI::App::Session::getInstance().getUser();
    Widgets::Card("confirm_booking", u8"Bước 4: Xác nhận phiếu đặt sân & Chiết khấu VIP", [this, user]() {
        if (user == nullptr) {
            ImGui::TextWrapped(u8"Phiên đăng nhập đã hết hạn. Vui lòng đăng nhập lại.");
            return;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
        ImGui::Text(u8"• Cơ sở sân bóng: %s", tenCoSoChon_.c_str());
        ImGui::Text(u8"• Địa chỉ cơ sở: %s", diaChiCoSoChon_.c_str());
        ImGui::Text(u8"• Sân bóng thi đấu: %s (Sân %d người)", tenSanChon_.c_str(), loaiSanChon_);
        ImGui::Text(u8"• Ngày thi đấu: %s  |  Thời lượng: %d phút/ca", ngayChon_.c_str(), thoiLuongChon_);
        ImGui::Text(u8"• Khung giờ đã chọn: %s", khungGioChon_.c_str());
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::Text(u8"Tiền sân niêm yết: %s", dinhDangTien(giaGoc_).c_str());
        if (phanTramGiam_ > 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Primary);
            ImGui::Text(u8"Chiết khấu thành viên VIP: -%d%% (-%s)", phanTramGiam_, dinhDangTien(soTienGiam_).c_str());
            ImGui::PopStyleColor();
        }

        ImFont* titleFont = GUI::Theme::getFonts().title;
        if (titleFont != nullptr) ImGui::PushFont(titleFont);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::PrimaryDark);
        ImGui::Text(u8"Tổng thanh toán thực tế: %s", dinhDangTien(tongTien_).c_str());
        ImGui::PopStyleColor();
        if (titleFont != nullptr) ImGui::PopFont();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        if (Widgets::NutPhu(u8"Quay lại chọn ca", ImVec2(190.0f, 38.0f))) {
            buocHienTai_ = 3;
        }

        ImGui::SameLine(0.0f, 12.0f);
        ImGui::BeginDisabled(cacCaDangChon_.isEmpty());
        if (Widgets::NutChinh(u8"Xác nhận đặt giữ chỗ ngay", ImVec2(240.0f, 38.0f))) {
            LinkedList<VenueBookingRequest> requests;
            LinkedList<std::string> bookingIds;
            for (auto* slot = danhSachKhungGioTrong_.get_head();
                 slot != nullptr; slot = slot->next) {
                if (!slotDangChon(slot->data.startTime)) continue;
                VenueBookingRequest request;
                request.customerId = user->get_user_id();
                request.customerName = user->get_name();
                request.customerPhone = user->get_phone_number();
                request.fieldId = maSanChon_;
                request.date = ngayChon_;
                request.startTime = slot->data.startTime;
                request.durationMinutes = thoiLuongChon_;
                requests.push_back(request);
            }
            std::string error;
            if (GUI::App::Facade::getInstance().datNhieuKhungGio(
                    requests, bookingIds, error)) {
                maDonDaDat_.clear();
                for (auto* id = bookingIds.get_head();
                     id != nullptr; id = id->next) {
                    if (!maDonDaDat_.empty()) maDonDaDat_ += ", ";
                    maDonDaDat_ += id->data;
                }
                buocHienTai_ = 5;
                Widgets::ToastThanhCong(u8"Đã gửi yêu cầu đặt sân thành công!");
            } else {
                loiTaiTaiDuLieu_ = error;
            }
        }
        ImGui::EndDisabled();
    });
}

void DatSanScreen::renderPhieuDatSan() {
    Widgets::Card("booking_receipt", u8"Phiếu xác nhận đặt sân thành công", [this]() {
        Widgets::BadgeTrong(u8"ĐÃ GHI NHẬN YÊU CẦU ĐẶT SÂN");
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
        ImGui::Text(u8"• Mã đơn đặt sân: %s", maDonDaDat_.c_str());
        ImGui::Text(u8"• Cơ sở sân: %s", tenCoSoChon_.c_str());
        ImGui::Text(u8"• Sân bóng: %s (%s)", tenSanChon_.c_str(), maSanChon_.c_str());
        ImGui::Text(u8"• Ngày đá: %s  |  Ca: %s (%d phút)", ngayChon_.c_str(), khungGioChon_.c_str(), thoiLuongChon_);
        ImGui::Text(u8"• Tổng tiền thanh toán: %s", dinhDangTien(tongTien_).c_str());
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        if (Widgets::NutChinh(u8"Xem lịch đặt của tôi", ImVec2(220.0f, 38.0f)) && onXemLichCuaToi_) {
            onXemLichCuaToi_();
        }

        ImGui::SameLine(0.0f, 12.0f);
        if (Widgets::NutPhu(u8"Đặt thêm sân khác", ImVec2(180.0f, 38.0f))) {
            buocHienTai_ = 1;
            ownerIdChon_.clear();
            maSanChon_.clear();
            khungGioChon_.clear();
            cacCaDangChon_.clear();
            maDonDaDat_.clear();
            reloadViewModel();
        }
    });
}

}  // namespace GUI::Screens::Renter
