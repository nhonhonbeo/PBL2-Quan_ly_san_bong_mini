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
