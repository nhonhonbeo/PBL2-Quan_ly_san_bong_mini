#include "GUI/Screens/Auth/AuthScreen.h"

#include <cmath>
#include <imgui.h>

#include "GUI/App/Facade.h"
#include "GUI/App/ScreenManager.h"
#include "GUI/Screens/DemoScreen.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/FormValidator.h"
#include "GUI/Widgets/InputFields.h"
#include "GUI/Widgets/Toast.h"

namespace GUI::Screens::Auth {

void AuthScreen::onEnter() {
    activeTab_ = 0;
    loginLoiSDT_.clear();
    loginLoiMatKhau_.clear();
    loginLoiChung_.clear();

    regLoiHoTen_.clear();
    regLoiSDT_.clear();
    regLoiMatKhau_.clear();
    regLoiNhapLai_.clear();
    regLoiTenCoSo_.clear();
    regLoiDiaChi_.clear();
}

void AuthScreen::onExit() {
}

void AuthScreen::render() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("##AuthScreenRoot", nullptr, flags)) {
        // Cập nhật bộ đếm ngược khóa nút nếu có
        if (thoiGianKhoaConLai_ > 0.0f) {
            thoiGianKhoaConLai_ -= ImGui::GetIO().DeltaTime;
            if (thoiGianKhoaConLai_ <= 0.0f) {
                thoiGianKhoaConLai_ = 0.0f;
            }
        }

        float totalW = ImGui::GetWindowWidth();
        float totalH = ImGui::GetWindowHeight();

        // Tỷ lệ phân chia: Banner bên trái 40% (tối thiểu 420px), Form bên phải phần còn lại
        float bannerW = std::max(420.0f, totalW * 0.40f);
        float formW = totalW - bannerW;

        // 1. Cột trái: Banner thương hiệu & Sân bóng
        renderBanner(bannerW, totalH);

        ImGui::SameLine(0.0f, 0.0f);

        // 2. Cột phải: Thẻ form Đăng nhập / Đăng ký
        renderFormCard(formW, totalH);
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
}

void AuthScreen::renderBanner(float width, float height) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, GUI::Theme::Colors::GrassGreen);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(28.0f, 28.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

    if (ImGui::BeginChild("##AuthBanner", ImVec2(width, height), false, ImGuiWindowFlags_NoScrollbar)) {
        ImGui::Spacing();

        // Tiêu đề thương hiệu
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::WhiteCard);
        ImGui::TextUnformatted(u8"QUẢN LÝ SÂN BÓNG MINI");
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.88f, 0.98f, 0.92f, 1.0f));
        ImGui::TextUnformatted(u8"Nhanh chóng - Chuyên nghiệp - Hiện đại");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // ==========================================
        // VẼ HÌNH SÂN BÓNG MINI BẰNG IMDRAWLIST
        // ==========================================
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        float availW = ImGui::GetContentRegionAvail().x;
        float pitchW = std::min(availW, 360.0f);
        float pitchH = pitchW * 0.60f; // Tỷ lệ chuẩn sân mini 5-7 người

        float pitchStartX = ImGui::GetCursorScreenPos().x + (availW - pitchW) * 0.5f;
        float pitchStartY = ImGui::GetCursorScreenPos().y + 8.0f;

        ImVec2 p0(pitchStartX, pitchStartY);
        ImVec2 p1(pitchStartX + pitchW, pitchStartY + pitchH);

        // Nền mặt cỏ xanh đậm
        ImU32 grassBg = GUI::Theme::Colors::toU32(GUI::Theme::Colors::DarkGreen);
        drawList->AddRectFilled(p0, p1, grassBg, 6.0f);

        // Vẽ các sọc cỏ xen kẽ
        int numStripes = 8;
        float stripeW = pitchW / numStripes;
        ImU32 stripeCol = IM_COL32(20, 115, 75, 255);
        for (int i = 0; i < numStripes; i += 2) {
            ImVec2 s0(p0.x + i * stripeW, p0.y);
            ImVec2 s1(p0.x + (i + 1) * stripeW, p1.y);
            drawList->AddRectFilled(s0, s1, stripeCol);
        }

        // Đường biên bao quanh sân
        ImU32 lineCol = IM_COL32(255, 255, 255, 220);
        drawList->AddRect(p0, p1, lineCol, 6.0f, 0, 2.0f);

        // Vạch giữa sân
        float midX = p0.x + pitchW * 0.5f;
        drawList->AddLine(ImVec2(midX, p0.y), ImVec2(midX, p1.y), lineCol, 2.0f);

        // Vòng tròn trung tâm & Chấm tâm
        ImVec2 center(midX, p0.y + pitchH * 0.5f);
        float centerR = pitchH * 0.22f;
        drawList->AddCircle(center, centerR, lineCol, 32, 2.0f);
        drawList->AddCircleFilled(center, 3.5f, IM_COL32_WHITE);

        // Khu phạt đền 2 bên
        float boxW = pitchW * 0.16f;
        float boxH = pitchH * 0.52f;
        float boxTop = p0.y + (pitchH - boxH) * 0.5f;

        // Vòng cấm trái
        drawList->AddRect(ImVec2(p0.x, boxTop), ImVec2(p0.x + boxW, boxTop + boxH), lineCol, 0, 0, 2.0f);
        // Cầu môn trái
        float goalH = pitchH * 0.28f;
        float goalTop = p0.y + (pitchH - goalH) * 0.5f;
        drawList->AddRect(ImVec2(p0.x - 6.0f, goalTop), ImVec2(p0.x, goalTop + goalH), lineCol, 0, 0, 2.0f);

        // Vòng cấm phải
        drawList->AddRect(ImVec2(p1.x - boxW, boxTop), ImVec2(p1.x, boxTop + boxH), lineCol, 0, 0, 2.0f);
        // Cầu môn phải
        drawList->AddRect(ImVec2(p1.x, goalTop), ImVec2(p1.x + 6.0f, goalTop + goalH), lineCol, 0, 0, 2.0f);

        ImGui::Dummy(ImVec2(pitchW, pitchH + 16.0f));

        // Thông tin giới thiệu nghiệp vụ
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::WhiteCard);
        ImGui::TextUnformatted(u8"Giải pháp quản lý sân bóng toàn diện:");
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.98f, 0.94f, 1.0f));
        ImGui::TextUnformatted(u8"Đặt sân & giữ chỗ 24/7 trực tuyến");
        ImGui::TextUnformatted(u8"Thiết lập giá & khung giờ linh hoạt");
        ImGui::TextUnformatted(u8"Tích điểm ưu đãi cho khách thân thiết");
        ImGui::TextUnformatted(u8"Quản lý sự kiện giải đấu bóng đá mini");
        ImGui::PopStyleColor();

        // Nút mở nhanh Thư viện Widget Demo ở chân banner
        float bottomPosY = height - 56.0f;
        if (bottomPosY > ImGui::GetCursorPosY() + 20.0f) {
            ImGui::SetCursorPosY(bottomPosY);
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.06f, 0.38f, 0.24f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::WhiteCard);
        float demoButtonWidth = std::max(0.0f, ImGui::GetContentRegionAvail().x);
        if (ImGui::Button(u8"Xem Thư viện Widget Demo", ImVec2(demoButtonWidth, 32.0f))) {
            GUI::App::ScreenManager::getInstance().push(std::make_unique<GUI::Screens::DemoScreen>());
        }
        ImGui::PopStyleColor(2);
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();
}

void AuthScreen::renderFormCard(float width, float height) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, GUI::Theme::Colors::LightBg);

    if (ImGui::BeginChild("##AuthFormArea", ImVec2(width, height), false)) {
        float cardWidth = std::min(480.0f, width - 40.0f);
        float startX = (width - cardWidth) * 0.5f;

        // Đẩy xuống một chút từ mép trên
        ImGui::SetCursorPos(ImVec2(startX, 36.0f));

        ImGui::PushStyleColor(ImGuiCol_ChildBg, GUI::Theme::Colors::WhiteCard);
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::BorderLight);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f, 24.0f));

        if (ImGui::BeginChild("##AuthCardBox", ImVec2(cardWidth, height - 72.0f), true)) {
            // Thanh Tab Đăng nhập / Đăng ký
            float tabWidth = (ImGui::GetContentRegionAvail().x - 10.0f) * 0.5f;

            if (activeTab_ == 0) {
                Widgets::NutChinh(u8"Đăng nhập", ImVec2(tabWidth, 38.0f));
            } else {
                if (Widgets::NutPhu(u8"Đăng nhập", ImVec2(tabWidth, 38.0f))) {
                    activeTab_ = 0;
                }
            }

            ImGui::SameLine();

            if (activeTab_ == 1) {
                Widgets::NutChinh(u8"Đăng ký tài khoản", ImVec2(tabWidth, 38.0f));
            } else {
                if (Widgets::NutPhu(u8"Đăng ký tài khoản", ImVec2(tabWidth, 38.0f))) {
                    activeTab_ = 1;
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (activeTab_ == 0) {
                renderTabDangNhap();
            } else {
                renderTabDangKy();
            }
        }
        ImGui::EndChild();

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
    }
    ImGui::EndChild();

    ImGui::PopStyleColor();
}

void AuthScreen::renderTabDangNhap() {
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
    ImGui::TextUnformatted(u8"Chào mừng bạn trở lại!");
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
    ImGui::TextUnformatted(u8"Vui lòng đăng nhập bằng Số điện thoại đã đăng ký.");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // Ô SĐT
    Widgets::InputPhone(
        u8"Số điện thoại:",
        "login_sdt",
        loginSDT_,
        loginLoiSDT_.empty() ? nullptr : loginLoiSDT_.c_str(),
        u8"0xxxxxxxxx"
    );
    ImGui::Spacing();

    // Ô Mật khẩu
    Widgets::InputPassword(
        u8"Mật khẩu:",
        "login_pw",
        loginMatKhau_,
        loginHienMatKhau_,
        loginLoiMatKhau_.empty() ? nullptr : loginLoiMatKhau_.c_str(),
        u8"Nhập mật khẩu..."
    );

    // LÝ DO CHỌN: Phím Enter khi đang ở ô nhập mật khẩu sẽ tự động kích hoạt gửi form đăng nhập
    if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter)) {
        xuLyDangNhap();
    }

    ImGui::Spacing();

    // Thông báo lỗi chung nếu đăng nhập thất bại
    if (!loginLoiChung_.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
        ImGui::TextWrapped(u8"%s", loginLoiChung_.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    // Nút Đăng nhập (khóa 10s nếu sai >= 3 lần)
    bool biKhoa = (thoiGianKhoaConLai_ > 0.0f);
    if (biKhoa) {
        ImGui::BeginDisabled();
        char lockText[64];
        std::snprintf(lockText, sizeof(lockText), u8"Tạm khóa (thử lại sau %.0fs)", std::ceil(thoiGianKhoaConLai_));
        Widgets::NutPhu(lockText, ImVec2(-1.0f, 42.0f));
        ImGui::EndDisabled();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GoldAccent);
        ImGui::TextWrapped(u8"Bạn đã nhập sai mật khẩu 3 lần. Hệ thống tạm khóa 10 giây để bảo vệ an toàn.");
        ImGui::PopStyleColor();
    } else {
        if (Widgets::NutChinh(u8"Đăng nhập ngay", ImVec2(-1.0f, 42.0f))) {
            xuLyDangNhap();
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Hướng dẫn tài khoản mẫu có sẵn
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
    ImGui::TextUnformatted(u8"Tài khoản mẫu thử nghiệm:");
    ImGui::TextUnformatted(u8"• Chủ sân: 0901234567 | MK: 123456");
    ImGui::TextUnformatted(u8"• Người thuê: 0909876543 | MK: 123456");
    ImGui::PopStyleColor();
}

void AuthScreen::renderTabDangKy() {
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
    ImGui::TextUnformatted(u8"Tạo tài khoản mới:");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // LÝ DO CHỌN: Hai thẻ lớn cho phép người dùng nhận biết rõ rệt vai trò trước khi đăng ký
    float cardW = (ImGui::GetContentRegionAvail().x - 10.0f) * 0.5f;

    // Thẻ Người thuê
    bool isRenter = (regVaiTro_ == 1);
    if (isRenter) {
        ImGui::PushStyleColor(ImGuiCol_Button, GUI::Theme::Colors::GrassGreen);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::WhiteCard);
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::GrassGreen);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, GUI::Theme::Colors::WhiteCard);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::BorderLight);
    }
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, isRenter ? 2.0f : 1.0f);

    if (ImGui::Button(u8"Người thuê sân", ImVec2(cardW, 46.0f))) {
        regVaiTro_ = 1;
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();

    // Thẻ Chủ sân
    bool isOwner = (regVaiTro_ == 0);
    if (isOwner) {
        ImGui::PushStyleColor(ImGuiCol_Button, GUI::Theme::Colors::GrassGreen);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::WhiteCard);
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::GrassGreen);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, GUI::Theme::Colors::WhiteCard);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::BorderLight);
    }
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, isOwner ? 2.0f : 1.0f);

    if (ImGui::Button(u8"Chủ sân bóng", ImVec2(cardW, 46.0f))) {
        regVaiTro_ = 0;
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Họ và tên
    Widgets::InputText(
        regVaiTro_ == 0 ? u8"Họ tên chủ sân (*):" : u8"Họ và tên (*):",
        "reg_name",
        regHoTen_,
        regLoiHoTen_.empty() ? nullptr : regLoiHoTen_.c_str(),
        u8"Nguyễn Văn A"
    );
    ImGui::Spacing();

    // Số điện thoại (kiểm tra trùng qua Facade)
    Widgets::InputPhone(
        u8"Số điện thoại đăng ký (*):",
        "reg_phone",
        regSDT_,
        regLoiSDT_.empty() ? nullptr : regLoiSDT_.c_str(),
        u8"0xxxxxxxxx"
    );
    ImGui::Spacing();

    // Nếu là Chủ sân: nhập thêm thông tin cơ sở
    if (regVaiTro_ == 0) {
        Widgets::InputText(
            u8"Tên cơ sở / Cụm sân bóng (*):",
            "reg_coso",
            regTenCoSo_,
            regLoiTenCoSo_.empty() ? nullptr : regLoiTenCoSo_.c_str(),
            u8"Ví dụ: Cụm sân cỏ nhân tạo Sao Vàng"
        );
        ImGui::Spacing();

        Widgets::InputText(
            u8"Địa chỉ sân bóng (*):",
            "reg_diachi",
            regDiaChi_,
            regLoiDiaChi_.empty() ? nullptr : regLoiDiaChi_.c_str(),
            u8"Số nhà, đường, quận/huyện, thành phố..."
        );
        ImGui::Spacing();
    }

    // Mật khẩu
    Widgets::InputPassword(
        u8"Mật khẩu (*):",
        "reg_pw",
        regMatKhau_,
        regHienMatKhau_,
        regLoiMatKhau_.empty() ? nullptr : regLoiMatKhau_.c_str(),
        u8"Tối thiểu 6 ký tự..."
    );
    ImGui::Spacing();

    // Nhập lại mật khẩu
    bool dummy = false;
    Widgets::InputPassword(
        u8"Xác nhận lại mật khẩu (*):",
        "reg_re_pw",
        regNhapLaiMatKhau_,
        dummy,
        regLoiNhapLai_.empty() ? nullptr : regLoiNhapLai_.c_str(),
        u8"Nhập lại mật khẩu vừa đặt..."
    );
    ImGui::Spacing();

    if (Widgets::NutChinh(u8"Hoàn tất đăng ký", ImVec2(-1.0f, 42.0f))) {
        xuLyDangKy();
    }
}

void AuthScreen::xuLyDangNhap() {
    if (thoiGianKhoaConLai_ > 0.0f) {
        return;
    }

    loginLoiSDT_ = Widgets::FormValidator::kiemTraSDT(loginSDT_);
    loginLoiMatKhau_ = Widgets::FormValidator::kiemTraKhongDeTrong(loginMatKhau_, u8"Mật khẩu");
    loginLoiChung_.clear();

    if (!loginLoiSDT_.empty() || !loginLoiMatKhau_.empty()) {
        return;
    }

    // Gọi Facade để xác thực nghiệp vụ
    auto ketQua = GUI::App::Facade::getInstance().dangNhap(loginSDT_, loginMatKhau_);

    if (ketQua.ok && ketQua.duLieu) {
        // Đăng nhập thành công -> reset biến đếm sai
        soLanDangNhapSai_ = 0;
        thoiGianKhoaConLai_ = 0.0f;

        Widgets::ToastThanhCong(u8"Đăng nhập thành công! Chào mừng " + ketQua.duLieu->get_name());

        // LÝ DO CHỌN: đa hình thay cho if/else theo vai trò để mở Shell tương ứng
        GUI::App::Facade::getInstance().moManHinhChinhChoUser(
            ketQua.duLieu.get(),
            GUI::App::ScreenManager::getInstance()
        );
    } else {
        // Đăng nhập thất bại -> tăng biến đếm sai
        soLanDangNhapSai_++;
        loginLoiChung_ = ketQua.loi;

        if (soLanDangNhapSai_ >= 3) {
            thoiGianKhoaConLai_ = 10.0f; // Khóa 10 giây
            Widgets::ToastLoi(u8"Bạn đã nhập sai 3 lần! Nút đăng nhập tạm khóa trong 10 giây.");
        } else {
            Widgets::ToastLoi(ketQua.loi);
        }
    }
}

void AuthScreen::xuLyDangKy() {
    regLoiHoTen_ = Widgets::FormValidator::kiemTraKhongDeTrong(regHoTen_, u8"Họ và tên");
    regLoiSDT_ = Widgets::FormValidator::kiemTraSDT(regSDT_);
    regLoiMatKhau_ = Widgets::FormValidator::kiemTraMatKhau(regMatKhau_, 6);

    if (regNhapLaiMatKhau_.empty()) {
        regLoiNhapLai_ = u8"Vui lòng nhập lại mật khẩu để xác nhận.";
    } else if (regMatKhau_ != regNhapLaiMatKhau_) {
        regLoiNhapLai_ = u8"Mật khẩu xác nhận không trùng khớp.";
    } else {
        regLoiNhapLai_.clear();
    }

    if (regVaiTro_ == 0) {
        regLoiTenCoSo_ = Widgets::FormValidator::kiemTraKhongDeTrong(regTenCoSo_, u8"Tên cơ sở");
        regLoiDiaChi_ = Widgets::FormValidator::kiemTraKhongDeTrong(regDiaChi_, u8"Địa chỉ");
    } else {
        regLoiTenCoSo_.clear();
        regLoiDiaChi_.clear();
    }

    if (!regLoiHoTen_.empty() || !regLoiSDT_.empty() || !regLoiMatKhau_.empty() ||
        !regLoiNhapLai_.empty() || !regLoiTenCoSo_.empty() || !regLoiDiaChi_.empty()) {
        return;
    }

    // Kiểm tra SĐT trùng qua Facade
    if (GUI::App::Facade::getInstance().kiemTraSDTDaTonTai(regSDT_)) {
        regLoiSDT_ = u8"Số điện thoại này đã được đăng ký trong hệ thống.";
        return;
    }

    GUI::App::KetQua kq;
    if (regVaiTro_ == 0) {
        kq = GUI::App::Facade::getInstance().dangKyChuSan(regHoTen_, regSDT_, regMatKhau_, regTenCoSo_, regDiaChi_);
    } else {
        kq = GUI::App::Facade::getInstance().dangKyNguoiThue(regHoTen_, regSDT_, regMatKhau_);
    }

    if (kq.ok) {
        loginSDT_ = regSDT_;
        loginMatKhau_ = regMatKhau_;
        activeTab_ = 0; // Chuyển về Tab Đăng nhập điền sẵn SĐT & Mật khẩu
    }
}

}  // namespace GUI::Screens::Auth
