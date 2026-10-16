#include "GUI/Screens/Renter/DatSanScreen.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <imgui.h>

#include "GUI/App/Session.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/DatePicker.h"
#include "GUI/Widgets/ProgressBar.h"
#include "GUI/Widgets/SlotGrid.h"
#include "GUI/Widgets/Toast.h"

namespace GUI::Screens::Renter {

namespace {

int parseNgaySangSo(const std::string& str) {
    if (str.length() == 10) {
        if (str[2] == '/' && str[5] == '/') {
            try {
                int d = std::stoi(str.substr(0, 2));
                int m = std::stoi(str.substr(3, 2));
                int y = std::stoi(str.substr(6, 4));
                return y * 10000 + m * 100 + d;
            } catch (...) {}
        } else if (str[4] == '-' && str[7] == '-') {
            try {
                int y = std::stoi(str.substr(0, 4));
                int m = std::stoi(str.substr(5, 2));
                int d = std::stoi(str.substr(8, 2));
                return y * 10000 + m * 100 + d;
            } catch (...) {}
        }
    }
    return 0;
}

std::string layNgayHomNayStr() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm localTm{};
#if defined(_WIN32)
    localtime_s(&localTm, &tt);
#else
    localtime_r(&tt, &localTm);
#endif
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << localTm.tm_mday << "/"
        << std::setfill('0') << std::setw(2) << (localTm.tm_mon + 1) << "/"
        << (localTm.tm_year + 1900);
    return oss.str();
}

std::string dinhDangTien(int soTien) {
    std::string s = std::to_string(soTien);
    int n = s.length() - 3;
    while (n > 0) {
        s.insert(n, ".");
        n -= 3;
    }
    return s + u8" VNĐ";
}

}  // namespace

DatSanScreen::DatSanScreen(std::function<void()> onXemLichCuaToi)
    : onXemLichCuaToi_(std::move(onXemLichCuaToi)) {}

void DatSanScreen::onEnter() {
    buocHienTai_ = 1;
    ngayChon_ = layNgayHomNayStr();
    maSanChon_.clear();
    khungGioChon_.clear();
    maDonDaDat_.clear();
    reloadViewModel();
}

void DatSanScreen::onExit() {
}

void DatSanScreen::reloadViewModel() {
    auto& facade = GUI::App::Facade::getInstance();
    User* curUser = GUI::App::Session::getInstance().getUser();
    std::string sdt = curUser ? curUser->get_phone_number() : "0909876543";

    // 1. Kiểm tra trạng thái bị chặn
    biChanDatSan_ = facade.laKhachHangBiChan(sdt);

    // 2. Lọc danh sách sân theo loại sân đã chọn
    danhSachSanTheoLoai_.clear();
    auto allSan = facade.getDanhSachSanBong();
    for (const auto& s : allSan) {
        if (s.loaiSan == loaiSanChon_) {
            danhSachSanTheoLoai_.push_back(s);
        }
    }

    // 3. Nạp danh sách khung giờ và tình trạng ô đặt
    danhSachKhungGio_ = facade.getDanhSachKhungGio();
    slotDataMap_ = facade.getTinhTrangSlotTheoNgay(ngayChon_);

    // 4. Lấy mức ưu đãi phần trăm của khách
    phanTramGiam_ = facade.getMucUuDaiCuaKhach(sdt);

    // 5. Tính giá nếu đã chọn sân và khung giờ
    if (!maSanChon_.empty() && !khungGioChon_.empty()) {
        giaGoc_ = facade.getGiaMaTran(khungGioChon_, loaiSanChon_);
        soTienGiam_ = (giaGoc_ * phanTramGiam_) / 100;
        tongTien_ = giaGoc_ - soTienGiam_;
    }
}

void DatSanScreen::render() {
    // 1. Nếu khách hàng bị chặn -> Hiển thị cảnh báo danh sách đen
    if (biChanDatSan_) {
        renderCanhBaoDanhSachDen();
        return;
    }

    // 2. Thanh tiến trình các bước
    if (buocHienTai_ <= 4) {
        renderThanhTienTrinh();
        ImGui::Spacing();
        ImGui::Spacing();
    }

    // 3. Render nội dung theo bước hiện tại
    switch (buocHienTai_) {
        case 1:
            renderBuoc1ChonLoaiSan();
            break;
        case 2:
            renderBuoc2ChonNgay();
            break;
        case 3:
            renderBuoc3ChonSlot();
            break;
        case 4:
            renderBuoc4XacNhan();
            break;
        case 5:
            renderBuoc5PhieuDatSan();
            break;
        default:
            break;
    }
}

void DatSanScreen::renderThanhTienTrinh() {
    float fraction = static_cast<float>(buocHienTai_) / 4.0f;
    char buf[64];
    std::snprintf(buf, sizeof(buf), u8"Bước %d / 4", buocHienTai_);

    Widgets::ProgressBar(fraction, buf, ImVec2(-1.0f, 14.0f), GUI::Theme::Colors::GrassGreen);
    ImGui::Spacing();

    const char* buocNames[] = {
        u8"1. Chọn loại sân",
        u8"2. Chọn ngày thi đấu",
        u8"3. Chọn khung giờ trống",
        u8"4. Xác nhận đặt sân"
    };

    ImGui::Columns(4, nullptr, false);
    for (int i = 0; i < 4; ++i) {
        bool isCurrent = (buocHienTai_ == i + 1);
        bool isDone = (buocHienTai_ > i + 1);

        ImVec4 textCol = isCurrent ? GUI::Theme::Colors::GrassGreen : (isDone ? GUI::Theme::Colors::DarkText : GUI::Theme::Colors::MutedText);
        ImGui::PushStyleColor(ImGuiCol_Text, textCol);
        ImGui::TextUnformatted(buocNames[i]);
        ImGui::PopStyleColor();

        ImGui::NextColumn();
    }
    ImGui::Columns(1);
}

void DatSanScreen::renderCanhBaoDanhSachDen() {
    Widgets::Card("card_renter_blacklist_alert", u8"CẢNH BÁO TÀI KHOẢN BỊ KHÓA ĐẶT SÂN", []() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
        ImGui::TextUnformatted(u8"⚠️ TÀI KHOẢN CỦA BẠN ĐANG NẰM TRONG DANH SÁCH ĐEN (BLACKLIST)");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::TextWrapped(u8"Do bạn đã hủy lịch đặt sân quá số lần quy định của cơ sở, quyền đặt giữ chỗ trực tuyến đã tạm thời bị khóa nhằm đảm bảo quyền lợi cho các đội bóng khác.");
        ImGui::Spacing();
        ImGui::TextWrapped(u8"👉 Vui lòng liên hệ trực tiếp với Chủ sân qua số điện thoại HOTLINE để được hỗ trợ kiểm tra và gỡ bỏ trạng thái khóa.");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Spacing();

        Widgets::BadgeDanhSachDen(u8"Trạng thái: Bị khóa đặt sân");
    });
}

void DatSanScreen::renderBuoc1ChonLoaiSan() {
    Widgets::Card("card_step1_pitch_type", u8"Bước 1: Lựa chọn quy mô loại sân bóng", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Chọn loại sân bóng mini phù hợp với số lượng cầu thủ của đội bạn:");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Spacing();

        float cardW = (ImGui::GetContentRegionAvail().x - 32.0f) / 3.0f;
        if (cardW < 220.0f) cardW = 220.0f;

        struct LoaiSanItem {
            int loai;
            const char* title;
            const char* desc;
            const char* price;
        } dsLoai[] = {
            {5,  u8"Sân 5 người",  u8"Mini 5v5 tiêu chuẩn\nMặt cỏ êm ái, thoát nước tốt", u8"Từ 200.000đ / h"},
            {7,  u8"Sân 7 người",  u8"Bán chuyên 7v7 rộng rãi\nKhung thành và đèn cao cấp", u8"Từ 350.000đ / h"},
            {11, u8"Sân 11 người", u8"Thi đấu 11v11 quy chuẩn\nPhù hợp giải đấu nội bộ",   u8"Từ 800.000đ / h"}
        };

        for (int i = 0; i < 3; ++i) {
            if (i > 0) ImGui::SameLine(0.0f, 16.0f);

            bool isSelected = (loaiSanChon_ == dsLoai[i].loai);
            std::string subCardId = "card_type_" + std::to_string(dsLoai[i].loai);

            ImGui::BeginGroup();
            Widgets::Card(subCardId.c_str(), dsLoai[i].title, [this, i, isSelected, &dsLoai]() {
                ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
                ImGui::TextWrapped(u8"%s", dsLoai[i].desc);
                ImGui::PopStyleColor();

                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
                ImGui::Text(u8"💰 %s", dsLoai[i].price);
                ImGui::PopStyleColor();

                ImGui::Spacing();
                ImGui::Spacing();

                if (isSelected) {
                    Widgets::Badge(u8"✓ Đang chọn", GUI::Theme::Colors::GrassGreen, GUI::Theme::Colors::WhiteCard);
                } else {
                    std::string btnSelectId = u8"Chọn loại này##" + std::to_string(dsLoai[i].loai);
                    if (Widgets::NutPhu(btnSelectId.c_str(), ImVec2(-1.0f, 32.0f))) {
                        loaiSanChon_ = dsLoai[i].loai;
                        maSanChon_.clear();
                        khungGioChon_.clear();
                        reloadViewModel();
                    }
                }
            });
            ImGui::EndGroup();
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (Widgets::NutChinh(u8"Tiếp tục sang Chọn ngày thi đấu →", ImVec2(280.0f, 38.0f))) {
            buocHienTai_ = 2;
            reloadViewModel();
        }
    });
}

void DatSanScreen::renderBuoc2ChonNgay() {
    Widgets::Card("card_step2_select_date", u8"Bước 2: Lựa chọn ngày thi đấu", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Chọn ngày thi đấu bạn mong muốn. Lưu ý hệ thống không chấp nhận đặt lịch cho những ngày trong quá khứ.");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::SetNextItemWidth(260.0f);
        Widgets::DatePicker(u8"Ngày thi đấu (dd/MM/yyyy) (*)", "##dp_renter_date", ngayChon_);

        ImGui::Spacing();

        // Kiểm tra không cho chọn ngày quá khứ
        std::string todayStr = layNgayHomNayStr();
        bool laNgayQuaKhu = (parseNgaySangSo(ngayChon_) < parseNgaySangSo(todayStr));

        if (laNgayQuaKhu) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
            ImGui::TextUnformatted(u8"✕ Ngày được chọn đã ở trong quá khứ! Vui lòng chọn từ ngày hôm nay trở đi.");
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
            ImGui::Text(u8"✓ Ngày hợp lệ: %s", ngayChon_.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Nút Quay lại
        if (Widgets::NutPhu(u8"← Quay lại", ImVec2(120.0f, 36.0f))) {
            buocHienTai_ = 1;
        }

        ImGui::SameLine(0.0f, 16.0f);

        // Nút Tiếp tục (vô hiệu hóa nếu là ngày quá khứ)
        if (!laNgayQuaKhu) {
            if (Widgets::NutChinh(u8"Tiếp tục sang Chọn khung giờ trống →", ImVec2(300.0f, 36.0f))) {
                buocHienTai_ = 3;
                reloadViewModel();
            }
        }
    });
}

void DatSanScreen::renderBuoc3ChonSlot() {
    std::string cardTitle = u8"Bước 3: Chọn khung giờ trống trên lưới sân " + std::to_string(loaiSanChon_) + u8" người (Ngày: " + ngayChon_ + ")";

    Widgets::Card("card_step3_slotgrid", cardTitle.c_str(), [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Bấm trực tiếp vào ô khung giờ còn trống (màu trắng) để chọn giữ chỗ. Các ô màu xám đã có người đặt trước sẽ bị khóa.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // Chú thích trạng thái
        Widgets::BadgeTrong(u8"Còn trống (Click để chọn)");
        ImGui::SameLine();
        Widgets::Badge(u8"Đang chọn", GUI::Theme::Colors::GoldAccent, GUI::Theme::Colors::WhiteCard);
        ImGui::SameLine();
        Widgets::BadgeDaDat(u8"Đã có người đặt (Khóa)");
        ImGui::Spacing();
        ImGui::Spacing();

        // Chuẩn bị danh sách mã sân hiển thị (chỉ các sân thuộc loại sân đã chọn)
        std::vector<std::string> dsMaSan;
        for (const auto& s : danhSachSanTheoLoai_) {
            dsMaSan.push_back(s.maSan);
        }

        std::vector<std::string> dsKhungGio;
        for (const auto& kg : danhSachKhungGio_) {
            dsKhungGio.push_back(kg.batDau + " - " + kg.ketThuc);
        }

        // Tái sử dụng SlotGrid ở chế độ chọn ô
        bool coDoiNgay = Widgets::SlotGrid::Render(
            "grid_renter_booking",
            ngayChon_,
            dsMaSan,
            dsKhungGio,
            slotDataMap_,
            true, // choPhepChon = true
            maSanChon_,
            khungGioChon_,
            [this](const std::string& maSan, const std::string& khungGio) {
                auto it = slotDataMap_.find({maSan, khungGio});
                if (it != slotDataMap_.end() && it->second.status == Widgets::SlotStatus::DaDat) {
                    Widgets::ToastLoi(u8"Ô này đã có người đặt trước!");
                    return;
                }
                if (it != slotDataMap_.end() && it->second.status == Widgets::SlotStatus::BaoTri) {
                    Widgets::ToastLoi(u8"Sân đang bảo trì trong khung giờ này!");
                    return;
                }

                maSanChon_ = maSan;
                khungGioChon_ = khungGio;
                reloadViewModel();
                Widgets::ToastThongTin(u8"Đã chọn: Sân " + maSan + " (" + khungGio + ")");
            }
        );

        if (coDoiNgay) {
            reloadViewModel();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Hiển thị tóm tắt ô đang chọn
        if (!maSanChon_.empty() && !khungGioChon_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
            ImGui::Text(u8"📌 Đang chọn: Sân %s | Khung giờ: %s | Giá dự kiến: %s",
                        maSanChon_.c_str(), khungGioChon_.c_str(), dinhDangTien(giaGoc_).c_str());
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
            ImGui::TextUnformatted(u8"Vui lòng nhấp chuột chọn 1 ô khung giờ trống ở bảng trên để tiếp tục!");
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Spacing();

        // Nút điều hướng
        if (Widgets::NutPhu(u8"← Quay lại Chọn ngày", ImVec2(180.0f, 36.0f))) {
            buocHienTai_ = 2;
        }

        ImGui::SameLine(0.0f, 16.0f);

        if (!maSanChon_.empty() && !khungGioChon_.empty()) {
            if (Widgets::NutChinh(u8"Tiếp tục sang Xác nhận đơn đặt →", ImVec2(280.0f, 36.0f))) {
                buocHienTai_ = 4;
                reloadViewModel();
            }
        }
    });
}

void DatSanScreen::renderBuoc4XacNhan() {
    Widgets::Card("card_step4_confirm", u8"Bước 4: Xác nhận thông tin và thanh toán giữ chỗ", [this]() {
        User* curUser = GUI::App::Session::getInstance().getUser();
        std::string tenKhach = curUser ? curUser->get_name() : u8"Trần Văn Thuê";
        std::string sdtKhach = curUser ? curUser->get_phone_number() : "0909876543";

        ImGui::Columns(2, "cols_confirm_bill", false);
        ImGui::SetColumnWidth(0, 420.0f);

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::TextUnformatted(u8"TÓM TẮT ĐƠN ĐẶT SÂN:");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::Text(u8"• Khách hàng: %s", tenKhach.c_str());
        ImGui::Text(u8"• Số điện thoại: %s", sdtKhach.c_str());
        ImGui::Text(u8"• Sân thi đấu: %s (Sân %d người)", maSanChon_.c_str(), loaiSanChon_);
        ImGui::Text(u8"• Ngày thi đấu: %s", ngayChon_.c_str());
        ImGui::Text(u8"• Khung giờ: %s", khungGioChon_.c_str());

        ImGui::NextColumn();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::TextUnformatted(u8"CHI TIẾT THANH TOÁN:");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::Text(u8"• Giá thuê niêm yết: %s", dinhDangTien(giaGoc_).c_str());

        if (phanTramGiam_ > 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
            ImGui::Text(u8"• Ưu đãi thành viên (Giảm %d%%): -%s", phanTramGiam_, dinhDangTien(soTienGiam_).c_str());
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::TextUnformatted(u8"• Ưu đãi áp dụng: 0 VNĐ (Khách hàng tiêu chuẩn)");
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
        ImGui::Text(u8"TỔNG CỘNG THANH TOÁN: %s", dinhDangTien(tongTien_).c_str());
        ImGui::PopStyleColor();

        ImGui::Columns(1);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Nút điều hướng
        if (Widgets::NutPhu(u8"← Quay lại Chọn giờ", ImVec2(180.0f, 38.0f))) {
            buocHienTai_ = 3;
        }

        ImGui::SameLine(0.0f, 16.0f);

        if (Widgets::NutChinh(u8"✓ Xác nhận & Hoàn tất đặt sân", ImVec2(260.0f, 38.0f))) {
            auto& facade = GUI::App::Facade::getInstance();
            auto kq = facade.datSanMoi(
                maSanChon_,
                sdtKhach,
                tenKhach,
                ngayChon_,
                khungGioChon_,
                giaGoc_,
                soTienGiam_,
                maDonDaDat_
            );

            if (kq.ok) {
                Widgets::ToastThanhCong(u8"Đặt sân thành công! Mã đơn: " + maDonDaDat_);
                buocHienTai_ = 5; // Chuyển sang phiếu đặt sân
            } else {
                Widgets::ToastLoi(kq.loi);
            }
        }
    });
}

void DatSanScreen::renderBuoc5PhieuDatSan() {
    Widgets::Card("card_step5_ticket", u8"PHIẾU XÁC NHẬN ĐẶT SÂN BÓNG ĐIỆN TỬ", [this]() {
        User* curUser = GUI::App::Session::getInstance().getUser();
        std::string tenKhach = curUser ? curUser->get_name() : u8"Trần Văn Thuê";
        std::string sdtKhach = curUser ? curUser->get_phone_number() : "0909876543";

        // Khung phiếu đặt sân mô phỏng vé điện tử
        ImGui::PushStyleColor(ImGuiCol_ChildBg, GUI::Theme::Colors::LightBg);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));

        if (ImGui::BeginChild("##ticket_box", ImVec2(0.0f, 240.0f), true)) {
            Widgets::Badge(u8"✓ ĐẶT SÂN THÀNH CÔNG", GUI::Theme::Colors::GrassGreen, GUI::Theme::Colors::WhiteCard);
            ImGui::SameLine(0.0f, 12.0f);
            Widgets::BadgeVIP(u8"Thanh toán tại sân");

            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
            ImGui::Text(u8"MÃ ĐƠN ĐẶT SÂN:  %s", maDonDaDat_.c_str());
            ImGui::PopStyleColor();

            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Columns(2, nullptr, false);
            ImGui::Text(u8"• Người đặt: %s", tenKhach.c_str());
            ImGui::Text(u8"• Số điện thoại: %s", sdtKhach.c_str());
            ImGui::Text(u8"• Loại sân: Sân %d người", loaiSanChon_);

            ImGui::NextColumn();
            ImGui::Text(u8"• Sân thi đấu: %s", maSanChon_.c_str());
            ImGui::Text(u8"• Ngày đá: %s", ngayChon_.c_str());
            ImGui::Text(u8"• Khung giờ: %s", khungGioChon_.c_str());
            ImGui::Columns(1);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
            ImGui::Text(u8"TỔNG TIỀN THANH TOÁN: %s", dinhDangTien(tongTien_).c_str());
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Spacing();

        // Các nút hành động sau khi đặt
        if (Widgets::NutChinh(u8"📅 Xem lịch của tôi", ImVec2(200.0f, 38.0f))) {
            if (onXemLichCuaToi_) {
                onXemLichCuaToi_();
            }
        }

        ImGui::SameLine(0.0f, 16.0f);

        if (Widgets::NutPhu(u8"+ Đặt thêm sân khác", ImVec2(180.0f, 38.0f))) {
            buocHienTai_ = 1;
            maSanChon_.clear();
            khungGioChon_.clear();
            reloadViewModel();
        }
    });
}

}  // namespace GUI::Screens::Renter
