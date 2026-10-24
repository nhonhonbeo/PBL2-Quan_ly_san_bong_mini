#include "GUI/Screens/Renter/DatSanScreen.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <utility>

#include <imgui.h>

#include "Core/DateTime.h"
#include "GUI/App/Facade.h"
#include "GUI/App/Session.h"
#include "GUI/Theme/Theme.h"
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

}  // Kết thúc không gian tên ẩn danh

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
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
        ImGui::TextWrapped("%s", loiTaiTaiDuLieu_.c_str());
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
    char label[48];
    std::snprintf(label, sizeof(label), u8"Bước %d / 4", buocHienTai_);
    Widgets::ProgressBar(
        static_cast<float>(buocHienTai_) / 4.0f,
        label,
        ImVec2(-1.0f, 30.0f),
        GUI::Theme::Colors::GrassGreen
    );
    ImGui::Columns(4, nullptr, false);
    const char* steps[] = {
        u8"1. Chọn cơ sở",
        u8"2. Chọn loại và sân",
        u8"3. Chọn ngày, ca",
        u8"4. Xác nhận"
    };
    for (int index = 0; index < 4; ++index) {
        const bool current = buocHienTai_ == index + 1;
        ImGui::PushStyleColor(
            ImGuiCol_Text,
            current ? GUI::Theme::Colors::GrassGreen
                    : GUI::Theme::Colors::MutedText
        );
        ImGui::TextUnformatted(steps[index]);
        ImGui::PopStyleColor();
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
}

void DatSanScreen::renderCanhBaoDanhSachDen() {
    Widgets::Card(
        "renter_blacklist_notice",
        u8"Tài khoản chưa thể đặt sân",
        []() {
            ImGui::TextWrapped(
                u8"Tài khoản của bạn đang nằm trong danh sách hạn chế đặt sân. "
                u8"Vui lòng liên hệ chủ sân để được hỗ trợ."
            );
        }
    );
}

void DatSanScreen::renderBuoc1ChonCoSo() {
    Widgets::Card("choose_facility", u8"Bước 1: Chọn cơ sở", [this]() {
        char search[256]{};
        const std::size_t count =
            tuKhoaCoSo_.size() < sizeof(search) - 1
                ? tuKhoaCoSo_.size()
                : sizeof(search) - 1;
        std::memcpy(search, tuKhoaCoSo_.data(), count);
        if (ImGui::InputText(u8"Tìm theo tên hoặc địa chỉ", search,
                             sizeof(search))) {
            tuKhoaCoSo_ = search;
        }
        ImGui::Spacing();
        bool found = false;
        for (auto* node = danhSachCoSo_.get_head(); node != nullptr;
             node = node->next) {
            const VenueFacilityRecord& facility = node->data;
            if (facility.name.find(tuKhoaCoSo_) == std::string::npos &&
                facility.address.find(tuKhoaCoSo_) == std::string::npos) {
                continue;
            }
            found = true;
            ImGui::PushID(facility.ownerId.c_str());
            const int fieldCount = soSanTheoCoSo_[facility.ownerId];
            const long long lowestPrice = giaThapNhatTheoCoSo_[facility.ownerId];
            ImGui::Text("%s", facility.name.c_str());
            ImGui::TextWrapped(u8"Địa chỉ: %s", facility.address.c_str());
            if (fieldCount == 0) {
                ImGui::TextDisabled(u8"Chưa có sân");
            } else {
                ImGui::Text(
                    u8"%d sân | Giá từ %s",
                    fieldCount,
                    dinhDangTien(lowestPrice).c_str()
                );
                if (Widgets::NutChinh(u8"Chọn cơ sở", ImVec2(160.0f, 34.0f))) {
                    chonCoSo(facility);
                }
            }
            ImGui::Separator();
            ImGui::PopID();
        }
        if (!found) ImGui::TextDisabled(u8"Không tìm thấy cơ sở phù hợp.");
    });
}

void DatSanScreen::renderBuoc2ChonSan() {
    Widgets::Card("choose_field", u8"Bước 2: Chọn loại sân và sân cụ thể", [this]() {
        ImGui::TextWrapped(
            u8"Cơ sở: %s — %s",
            tenCoSoChon_.c_str(),
            diaChiCoSoChon_.c_str()
        );
        ImGui::Spacing();
        for (const int size : {5, 7, 11}) {
            if (!loaiSanCoSan_[size]) continue;
            ImGui::SameLine();
            if (ImGui::RadioButton(
                    (u8"Sân " + std::to_string(size) + u8" người").c_str(),
                    loaiSanChon_ == size)) {
                loaiSanChon_ = size;
                maSanChon_.clear();
                tenSanChon_.clear();
            }
        }
        ImGui::Spacing();
        int shown = 0;
        for (auto* node = danhSachSan_.get_head(); node != nullptr;
             node = node->next) {
            const VenueFieldRecord& field = node->data;
            if (field.status != 0 || field.size != loaiSanChon_) continue;
            ++shown;
            ImGui::PushID(field.fieldId.c_str());
            const bool selected = maSanChon_ == field.fieldId;
            if (ImGui::Selectable(
                    (field.name + " (" + field.fieldId + ")").c_str(),
                    selected)) {
                maSanChon_ = field.fieldId;
                tenSanChon_ = field.name;
                loaiSanChon_ = field.size;
            }
            ImGui::PopID();
        }
        if (shown == 0) ImGui::TextDisabled(u8"Loại sân này hiện chưa có sân trống.");
        if (!maSanChon_.empty() &&
            Widgets::NutChinh(u8"Tiếp tục chọn ngày và ca", ImVec2(250.0f, 38.0f))) {
            buocHienTai_ = 3;
            reloadSlots();
        }
        ImGui::SameLine();
        if (Widgets::NutPhu(u8"Quay lại cơ sở", ImVec2(170.0f, 38.0f))) {
            buocHienTai_ = 1;
        }
    });
}

void DatSanScreen::renderBuoc3ChonNgayVaKhungGio() {
    Widgets::Card("choose_date_slot", u8"Bước 3: Chọn thời lượng, ngày và ca", [this]() {
        ImGui::Text(
            u8"%s | %s | Sân %d người",
            tenCoSoChon_.c_str(), tenSanChon_.c_str(), loaiSanChon_
        );
        int duration = thoiLuongChon_;
        if (choPhep60Phut_) {
            ImGui::RadioButton(u8"60 phút", &duration, 60);
        }
        if (choPhep90Phut_) {
            if (choPhep60Phut_) ImGui::SameLine();
            ImGui::RadioButton(u8"90 phút", &duration, 90);
        }
        if (duration != thoiLuongChon_) {
            thoiLuongChon_ = duration;
            cacCaDangChon_.clear();
            reloadSlots();
        }

        ImGui::SetNextItemWidth(260.0f);
        if (Widgets::DatePicker(
                u8"Ngày đặt sân", "##renter_booking_date", ngayChon_)) {
            cacCaDangChon_.clear();
            reloadSlots();
        }
        ImGui::Spacing();
        const bool validDate =
            DateTime::isValidDate(DateTime::toIsoDate(ngayChon_));
        const bool pastDate =
            validDate && DateTime::isPastDate(DateTime::toIsoDate(ngayChon_));
        if (pastDate) {
            ImGui::TextColored(
                GUI::Theme::Colors::RedDanger,
                u8"Không thể đặt sân cho ngày đã qua."
            );
        } else if (!validDate) {
            ImGui::TextColored(
                GUI::Theme::Colors::RedDanger,
                u8"Vui lòng nhập ngày hợp lệ theo định dạng dd/MM/yyyy."
            );
        } else if (danhSachKhungGioTrong_.isEmpty()) {
            ImGui::TextDisabled(u8"Không có khung giờ trống cho lựa chọn này.");
        } else {
            for (auto* node = danhSachKhungGioTrong_.get_head();
                 node != nullptr; node = node->next) {
                const VenueSlotRecord& slot = node->data;
                ImGui::PushID(slot.startTime.c_str());
                const bool selected = slotDangChon(slot.startTime);
                std::string label =
                    slot.startTime + " - " + slot.endTime + " | ";
                if (!slot.bookingId.empty()) {
                    label += u8"ĐÃ ĐẶT";
                } else if (slot.past) {
                    label += u8"ĐÃ QUA";
                } else if (!slot.available) {
                    label += u8"KHÔNG KHẢ DỤNG";
                } else {
                    label += selected ? u8"ĐANG CHỌN | " : "";
                    label += dinhDangTien(slot.priceVnd);
                }
                if (selected) {
                    ImGui::PushStyleColor(
                        ImGuiCol_Button, GUI::Theme::Colors::GoldAccent
                    );
                }
                ImGui::BeginDisabled(!slot.available || slot.past);
                if (ImGui::Button(label.c_str(), ImVec2(270.0f, 38.0f))) {
                    if (selected) {
                        cacCaDangChon_.removeIf(
                            [&slot](const VenueSlotRecord& current) {
                                return current.startTime == slot.startTime;
                            }
                        );
                        loiChonCa_.clear();
                    } else {
                        bool adjacent = cacCaDangChon_.isEmpty();
                        std::string earliestStart;
                        std::string latestEnd;
                        for (auto* chosen = cacCaDangChon_.get_head();
                             chosen != nullptr; chosen = chosen->next) {
                            if (earliestStart.empty() ||
                                chosen->data.startTime < earliestStart) {
                                earliestStart = chosen->data.startTime;
                            }
                            if (latestEnd.empty() ||
                                chosen->data.endTime > latestEnd) {
                                latestEnd = chosen->data.endTime;
                            }
                        }
                        adjacent = adjacent || slot.endTime == earliestStart ||
                                   slot.startTime == latestEnd;
                        if (adjacent) {
                            cacCaDangChon_.push_back(slot);
                            loiChonCa_.clear();
                        } else {
                            loiChonCa_ =
                                u8"Chỉ có thể chọn thêm ca liền kề với các ca đã chọn.";
                        }
                    }
                    capNhatTongTien();
                }
                ImGui::EndDisabled();
                if (selected) ImGui::PopStyleColor();
                ImGui::PopID();
                ImGui::SameLine();
            }
        }
        ImGui::NewLine();
        if (!loiChonCa_.empty()) {
            ImGui::TextColored(
                GUI::Theme::Colors::RedDanger, "%s", loiChonCa_.c_str()
            );
        }
        if (!cacCaDangChon_.isEmpty()) {
            ImGui::Text(
                u8"Đang chọn: %s | Tổng: %s | Ưu đãi: %d%% (-%s) | Thanh toán: %s",
                khungGioChon_.c_str(), dinhDangTien(giaGoc_).c_str(),
                phanTramGiam_, dinhDangTien(soTienGiam_).c_str(),
                dinhDangTien(tongTien_).c_str()
            );
        }
        ImGui::Spacing();
        if (Widgets::NutPhu(u8"Quay lại chọn sân", ImVec2(190.0f, 36.0f))) {
            buocHienTai_ = 2;
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(cacCaDangChon_.isEmpty());
        if (Widgets::NutChinh(u8"Tiếp tục xác nhận", ImVec2(210.0f, 36.0f))) {
            buocHienTai_ = 4;
        }
        ImGui::EndDisabled();
    });
}

void DatSanScreen::renderBuoc4XacNhan() {
    User* user = GUI::App::Session::getInstance().getUser();
    Widgets::Card("confirm_booking", u8"Bước 4: Xác nhận đặt sân", [this, user]() {
        if (user == nullptr) {
            ImGui::TextWrapped(u8"Phiên đăng nhập đã hết hạn. Vui lòng đăng nhập lại.");
            return;
        }
        ImGui::Text(u8"Cơ sở: %s", tenCoSoChon_.c_str());
        ImGui::Text(u8"Địa chỉ: %s", diaChiCoSoChon_.c_str());
        ImGui::Text(u8"Sân: %s (sân %d người)", tenSanChon_.c_str(), loaiSanChon_);
        ImGui::Text(u8"Ngày: %s | Thời lượng mỗi ca: %d phút",
                    ngayChon_.c_str(), thoiLuongChon_);
        ImGui::Text(u8"Các ca đã chọn: %s", khungGioChon_.c_str());
        ImGui::Separator();
        ImGui::Text(u8"Giá: %s", dinhDangTien(giaGoc_).c_str());
        ImGui::Text(u8"Ưu đãi: %d%% (-%s)", phanTramGiam_,
                    dinhDangTien(soTienGiam_).c_str());
        ImGui::Text(u8"Tổng thanh toán: %s", dinhDangTien(tongTien_).c_str());
        ImGui::Spacing();
        if (Widgets::NutPhu(u8"Quay lại chọn ca", ImVec2(190.0f, 38.0f))) {
            buocHienTai_ = 3;
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(cacCaDangChon_.isEmpty());
        if (Widgets::NutChinh(u8"Xác nhận đặt sân", ImVec2(230.0f, 38.0f))) {
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
                Widgets::ToastThanhCong(u8"Đã gửi yêu cầu đặt sân.");
            } else {
                loiTaiTaiDuLieu_ = error;
            }
        }
        ImGui::EndDisabled();
    });
}

void DatSanScreen::renderPhieuDatSan() {
    Widgets::Card("booking_receipt", u8"Yêu cầu đặt sân đã được ghi nhận", [this]() {
        ImGui::Text(u8"Mã đơn: %s", maDonDaDat_.c_str());
        ImGui::Text(u8"Cơ sở: %s", tenCoSoChon_.c_str());
        ImGui::Text(u8"Sân: %s | %s", tenSanChon_.c_str(), maSanChon_.c_str());
        ImGui::Text(u8"Ngày: %s | Ca: %s | %d phút",
                    ngayChon_.c_str(), khungGioChon_.c_str(), thoiLuongChon_);
        ImGui::Text(u8"Tổng tiền: %s", dinhDangTien(tongTien_).c_str());
        ImGui::Spacing();
        if (Widgets::NutChinh(u8"Xem lịch đặt của tôi", ImVec2(220.0f, 38.0f)) &&
            onXemLichCuaToi_) {
            onXemLichCuaToi_();
        }
        ImGui::SameLine();
        if (Widgets::NutPhu(u8"Đặt sân khác", ImVec2(160.0f, 38.0f))) {
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
