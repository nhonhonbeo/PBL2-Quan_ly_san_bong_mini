#include "GUI/Screens/Auth/AuthScreen.h"

#include <cmath>
#include <algorithm>
#include <imgui.h>

#include "GUI/App/Facade.h"
#include "GUI/App/ScreenManager.h"
#include "GUI/Screens/DemoScreen.h"
#include "GUI/Theme/Fonts.h"
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
        if (thoiGianKhoaConLai_ > 0.0f) {
            thoiGianKhoaConLai_ -= ImGui::GetIO().DeltaTime;
            if (thoiGianKhoaConLai_ <= 0.0f) {
                thoiGianKhoaConLai_ = 0.0f;
            }
        }

        float totalW = ImGui::GetWindowWidth();
        float totalH = ImGui::GetWindowHeight();

        // LÝ DO CHỌN: Tỷ lệ phân chia 42% (min 450px) cho cột Banner thương hiệu và phần còn lại cho Form
        float bannerW = std::max(450.0f, totalW * 0.42f);
        float formW = totalW - bannerW;

        // 1. Cột trái: Banner nhận diện thương hiệu & Sân bóng vector illustration
        renderBanner(bannerW, totalH);

        ImGui::SameLine(0.0f, 0.0f);

        // 2. Cột phải: Form Đăng nhập / Đăng ký hiện đại
        renderFormCard(formW, totalH);
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
}

void AuthScreen::renderBanner(float width, float height) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, GUI::Theme::Colors::PrimaryDark);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32.0f, 28.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

    if (ImGui::BeginChild("##AuthBanner", ImVec2(width, height), false, ImGuiWindowFlags_NoScrollbar)) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 bannerMin = ImGui::GetWindowPos();
        ImVec2 bannerMax = ImVec2(bannerMin.x + width, bannerMin.y + height);

        // Gradient nền chuyển sắc nhẹ từ xanh rừng đậm sang xanh lá thương hiệu
        ImU32 colTop = IM_COL32(14, 68, 46, 255);    // #0E442E
        ImU32 colBottom = IM_COL32(22, 134, 91, 255); // #16865B
        drawList->AddRectFilledMultiColor(bannerMin, bannerMax, colTop, colTop, colBottom, colBottom);

        // ==========================================
        // 1. LOGO & HEADER THƯƠNG HIỆU
        // ==========================================
        ImVec2 crestCenter(bannerMin.x + 48.0f, bannerMin.y + 44.0f);
        float crestR = 20.0f;

        // Vẽ huy hiệu logo hình tròn có đường may quả bóng
        drawList->AddCircleFilled(crestCenter, crestR, IM_COL32(255, 255, 255, 40));
        drawList->AddCircle(crestCenter, crestR, IM_COL32(255, 255, 255, 200), 24, 2.0f);
        drawList->AddCircleFilled(crestCenter, 5.0f, IM_COL32_WHITE);
        for (int k = 0; k < 5; ++k) {
            float a = k * (3.14159f * 2.0f / 5.0f) - 1.57f;
            ImVec2 pEdge(crestCenter.x + crestR * std::cos(a), crestCenter.y + crestR * std::sin(a));
            ImVec2 pInner(crestCenter.x + 5.0f * std::cos(a), crestCenter.y + 5.0f * std::sin(a));
            drawList->AddLine(pInner, pEdge, IM_COL32(255, 255, 255, 180), 1.5f);
        }

        ImGui::SetCursorPos(ImVec2(80.0f, 24.0f));
        ImGui::BeginGroup();

        ImFont* headingFont = GUI::Theme::getFonts().heading;
        if (headingFont != nullptr) ImGui::PushFont(headingFont);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Surface);
        ImGui::TextUnformatted(u8"MINIFOOTBALL PRO");
        ImGui::PopStyleColor();
        if (headingFont != nullptr) ImGui::PopFont();

        ImFont* smallFont = GUI::Theme::getFonts().small;
        if (smallFont != nullptr) ImGui::PushFont(smallFont);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.96f, 0.90f, 0.95f));
        ImGui::TextUnformatted(u8"Hệ Thống Quản Lý Sân Bóng Mini Chuyên Nghiệp");
        ImGui::PopStyleColor();
        if (smallFont != nullptr) ImGui::PopFont();

        ImGui::EndGroup();

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 18.0f);

        // ==========================================
        // 2. VECTOR ILLUSTRATION: SÂN BÓNG MINI SẮC NÉT
        // ==========================================
        float availW = ImGui::GetContentRegionAvail().x;
        float pitchW = std::min(availW, 370.0f);
        float pitchH = pitchW * 0.58f; // Tỷ lệ chuẩn 16:9 cho sân mini

        float pitchStartX = ImGui::GetCursorScreenPos().x + (availW - pitchW) * 0.5f;
        float pitchStartY = ImGui::GetCursorScreenPos().y;

        ImVec2 p0(pitchStartX, pitchStartY);
        ImVec2 p1(pitchStartX + pitchW, pitchStartY + pitchH);

        // Đổ bóng nền sân bóng
        drawList->AddRectFilled(
            ImVec2(p0.x - 2.0f, p0.y + 4.0f),
            ImVec2(p1.x + 2.0f, p1.y + 8.0f),
            IM_COL32(0, 0, 0, 30),
            8.0f
        );

        // Nền mặt cỏ xanh sẫm
        ImU32 grassBase = IM_COL32(15, 84, 56, 255); // #0F5438
        drawList->AddRectFilled(p0, p1, grassBase, 8.0f);

        // Vẽ 8 sọc cỏ xen kẽ màu sáng hơn
        int numStripes = 8;
        float stripeW = pitchW / numStripes;
        ImU32 stripeCol = IM_COL32(20, 108, 73, 255); // #146C49
        for (int i = 0; i < numStripes; i += 2) {
            ImVec2 s0(p0.x + i * stripeW, p0.y);
            ImVec2 s1(p0.x + (i + 1) * stripeW, p1.y);
            drawList->AddRectFilled(s0, s1, stripeCol);
        }

        // Đường biên ngoài (Touchline & Goal Line)
        ImU32 lineCol = IM_COL32(255, 255, 255, 225);
        drawList->AddRect(p0, p1, lineCol, 8.0f, 0, 2.0f);

        // Đường giữa sân (Halfway Line)
        float midX = p0.x + pitchW * 0.5f;
        drawList->AddLine(ImVec2(midX, p0.y), ImVec2(midX, p1.y), lineCol, 2.0f);

        // Vòng tròn trung tâm & Chấm giao bóng
        ImVec2 pitchCenter(midX, p0.y + pitchH * 0.5f);
        float centerR = pitchH * 0.22f;
        drawList->AddCircle(pitchCenter, centerR, lineCol, 36, 2.0f);
        drawList->AddCircleFilled(pitchCenter, 3.5f, IM_COL32_WHITE);

        // Khu phạt đền 2 bên
        float boxW = pitchW * 0.16f;
        float boxH = pitchH * 0.54f;
        float boxTop = p0.y + (pitchH - boxH) * 0.5f;

        // Vùng cấm địa trái
        drawList->AddRect(ImVec2(p0.x, boxTop), ImVec2(p0.x + boxW, boxTop + boxH), lineCol, 0, 0, 2.0f);
        // Khu 5m50 nhỏ trái
        float smallBoxW = boxW * 0.50f;
        float smallBoxH = boxH * 0.55f;
        float smallBoxTop = p0.y + (pitchH - smallBoxH) * 0.5f;
        drawList->AddRect(ImVec2(p0.x, smallBoxTop), ImVec2(p0.x + smallBoxW, smallBoxTop + smallBoxH), lineCol, 0, 0, 1.5f);
        // Chấm phạt đền trái
        drawList->AddCircleFilled(ImVec2(p0.x + boxW * 0.72f, p0.y + pitchH * 0.5f), 2.5f, IM_COL32_WHITE);
        // Cầu môn trái với lưới
        float goalH = pitchH * 0.30f;
        float goalTop = p0.y + (pitchH - goalH) * 0.5f;
        drawList->AddRect(ImVec2(p0.x - 7.0f, goalTop), ImVec2(p0.x, goalTop + goalH), lineCol, 0, 0, 2.0f);
        for (float gy = goalTop + 4.0f; gy < goalTop + goalH; gy += 5.0f) {
            drawList->AddLine(ImVec2(p0.x - 7.0f, gy), ImVec2(p0.x, gy), IM_COL32(255, 255, 255, 120), 1.0f);
        }

        // Vùng cấm địa phải
        drawList->AddRect(ImVec2(p1.x - boxW, boxTop), ImVec2(p1.x, boxTop + boxH), lineCol, 0, 0, 2.0f);
        // Khu 5m50 nhỏ phải
        drawList->AddRect(ImVec2(p1.x - smallBoxW, smallBoxTop), ImVec2(p1.x, smallBoxTop + smallBoxH), lineCol, 0, 0, 1.5f);
        // Chấm phạt đền phải
        drawList->AddCircleFilled(ImVec2(p1.x - boxW * 0.72f, p0.y + pitchH * 0.5f), 2.5f, IM_COL32_WHITE);
        // Cầu môn phải với lưới
        drawList->AddRect(ImVec2(p1.x, goalTop), ImVec2(p1.x + 7.0f, goalTop + goalH), lineCol, 0, 0, 2.0f);
        for (float gy = goalTop + 4.0f; gy < goalTop + goalH; gy += 5.0f) {
            drawList->AddLine(ImVec2(p1.x, gy), ImVec2(p1.x + 7.0f, gy), IM_COL32(255, 255, 255, 120), 1.0f);
        }

        // 4 góc sân: Vòng cung phạt góc (Corner Arcs)
        float cornerR = 8.0f;
        drawList->PathArcTo(p0, cornerR, 0.0f, 1.57f, 8);
        drawList->PathStroke(lineCol, 0, 1.5f);
        drawList->PathArcTo(ImVec2(p1.x, p0.y), cornerR, 1.57f, 3.14f, 8);
        drawList->PathStroke(lineCol, 0, 1.5f);
        drawList->PathArcTo(p1, cornerR, 3.14f, 4.71f, 8);
        drawList->PathStroke(lineCol, 0, 1.5f);
        drawList->PathArcTo(ImVec2(p0.x, p1.y), cornerR, 4.71f, 6.28f, 8);
        drawList->PathStroke(lineCol, 0, 1.5f);

        // Quả bóng đá vector đặt tại vị trí chiến thuật
        ImVec2 ballPos(midX + 22.0f, p0.y + pitchH * 0.42f);
        float ballR = 9.0f;
        drawList->AddCircleFilled(ImVec2(ballPos.x + 1.0f, ballPos.y + 2.0f), ballR, IM_COL32(0, 0, 0, 50));
        drawList->AddCircleFilled(ballPos, ballR, IM_COL32(255, 255, 255, 255));
        drawList->AddCircle(ballPos, ballR, IM_COL32(30, 41, 59, 255), 18, 1.2f);
        drawList->AddCircleFilled(ballPos, 3.0f, IM_COL32(30, 41, 59, 255));

        ImGui::Dummy(ImVec2(pitchW, pitchH + 16.0f));

        // ==========================================
        // 3. GIỚI THIỆU TÍNH NĂNG NỔI BẬT (FEATURE CARDS)
        // ==========================================
        auto veTheTinhNang = [width](const char* tieuDe, const char* moTa) {
            ImVec2 c0 = ImGui::GetCursorScreenPos();
            float cW = ImGui::GetContentRegionAvail().x;
            ImVec2 c1(c0.x + cW, c0.y + 46.0f);
            ImDrawList* dl = ImGui::GetWindowDrawList();

            dl->AddRectFilled(c0, c1, IM_COL32(255, 255, 255, 20), 6.0f);
            dl->AddRect(c0, c1, IM_COL32(255, 255, 255, 35), 6.0f, 0, 1.0f);

            // Vạch xanh ngọc chỉ báo
            dl->AddRectFilled(c0, ImVec2(c0.x + 3.5f, c1.y), IM_COL32(34, 197, 94, 255), 6.0f, ImDrawFlags_RoundCornersLeft);

            ImGui::SetCursorScreenPos(ImVec2(c0.x + 12.0f, c0.y + 6.0f));
            ImGui::BeginGroup();
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Surface);
            ImGui::TextUnformatted(tieuDe);
            ImGui::PopStyleColor();

            ImFont* sFont = GUI::Theme::getFonts().small;
            if (sFont != nullptr) ImGui::PushFont(sFont);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.94f, 0.89f, 0.90f));
            ImGui::TextUnformatted(moTa);
            ImGui::PopStyleColor();
            if (sFont != nullptr) ImGui::PopFont();
            ImGui::EndGroup();

            ImGui::SetCursorScreenPos(ImVec2(c0.x, c1.y + 8.0f));
        };

        veTheTinhNang(u8"Quản lý sân & Lịch thi đấu thông minh", u8"Tự động phân bổ ca đá, kiểm soát khung giờ, chống trùng lịch.");
        veTheTinhNang(u8"Đặt sân trực tuyến & Giữ chỗ tức thì 24/7", u8"Tra cứu tình trạng sân theo thời gian thực, bảng giá minh bạch.");
        veTheTinhNang(u8"Khách hàng thân thiết & Giải đấu phong trào", u8"Tích điểm nâng hạng VIP, ưu đãi tự động, quản lý cúp giao hữu.");

        // ==========================================
        // 4. CHÂN BANNER & NÚT DEMO WIDGETS
        // ==========================================
        float bottomPosY = height - 52.0f;
        if (bottomPosY > ImGui::GetCursorPosY() + 10.0f) {
            ImGui::SetCursorPosY(bottomPosY);
        }

        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 255, 255, 25));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 255, 45));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 255, 15));
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Surface);
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(255, 255, 255, 60));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

        float demoButtonWidth = std::max(0.0f, ImGui::GetContentRegionAvail().x);
        if (ImGui::Button(u8"Xem Thư viện Widget Demo UI", ImVec2(demoButtonWidth, 34.0f))) {
            GUI::App::ScreenManager::getInstance().push(std::make_unique<GUI::Screens::DemoScreen>());
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(5);
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();
}

void AuthScreen::renderFormCard(float width, float height) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, GUI::Theme::Colors::Background);

    if (ImGui::BeginChild("##AuthFormArea", ImVec2(width, height), false)) {
        float cardWidth = std::min(460.0f, width - 48.0f);
        float startX = (width - cardWidth) * 0.5f;

        // Căn giữa form theo trục dọc với khoảng đệm hợp lý
        float cardHeight = std::min(height - 48.0f, activeTab_ == 0 ? 560.0f : 660.0f);
        float startY = std::max(24.0f, (height - cardHeight) * 0.5f);

        ImGui::SetCursorPos(ImVec2(startX, startY));

        ImGui::PushStyleColor(ImGuiCol_ChildBg, GUI::Theme::Colors::Surface);
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::Border);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(28.0f, 26.0f));

        if (ImGui::BeginChild("##AuthCardBox", ImVec2(cardWidth, cardHeight), true)) {
            // ==========================================
            // SEGMENTED SWITCH: [ ĐĂNG NHẬP ] [ ĐĂNG KÝ ]
            // ==========================================
            float totalAvail = ImGui::GetContentRegionAvail().x;
            float tabWidth = (totalAvail - 8.0f) * 0.5f;

            if (activeTab_ == 0) {
                Widgets::NutChinh(u8"Đăng nhập", ImVec2(tabWidth, 40.0f));
            } else {
                if (Widgets::NutPhu(u8"Đăng nhập", ImVec2(tabWidth, 40.0f))) {
                    activeTab_ = 0;
                }
            }

            ImGui::SameLine(0.0f, 8.0f);

            if (activeTab_ == 1) {
                Widgets::NutChinh(u8"Đăng ký tài khoản", ImVec2(tabWidth, 40.0f));
            } else {
                if (Widgets::NutPhu(u8"Đăng ký tài khoản", ImVec2(tabWidth, 40.0f))) {
                    activeTab_ = 1;
                }
            }

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
            ImGui::Separator();
            ImGui::PopStyleColor();
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
    ImFont* titleFont = GUI::Theme::getFonts().title;
    if (titleFont != nullptr) ImGui::PushFont(titleFont);
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
    ImGui::TextUnformatted(u8"Chào mừng trở lại!");
    ImGui::PopStyleColor();
    if (titleFont != nullptr) ImGui::PopFont();

    ImFont* smallFont = GUI::Theme::getFonts().small;
    if (smallFont != nullptr) ImGui::PushFont(smallFont);
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
    ImGui::TextUnformatted(u8"Nhập số điện thoại và mật khẩu để tiếp tục vào hệ thống");
    ImGui::PopStyleColor();
    if (smallFont != nullptr) ImGui::PopFont();

    ImGui::Spacing();
    ImGui::Spacing();

    // ==========================================
    // KHU VỰC TÀI KHOẢN DÙNG THỬ (DEMO CHIPS NHỎ GỌN)
    // ==========================================
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
    ImGui::TextUnformatted(u8"Tài khoản dùng thử (bấm để tự điền):");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    float chipWidth = (ImGui::GetContentRegionAvail().x - 8.0f) * 0.5f;

    // Nút nhanh 1: Chủ sân
    if (Widgets::NutPhu(u8"Chủ sân: 0901234567", ImVec2(chipWidth, 32.0f))) {
        loginSDT_ = "0901234567";
        loginMatKhau_ = "123456";
        loginLoiSDT_.clear();
        loginLoiMatKhau_.clear();
        loginLoiChung_.clear();
        Widgets::ToastThongTin(u8"Đã điền thông tin tài khoản Chủ sân mẫu");
    }

    ImGui::SameLine(0.0f, 8.0f);

    // Nút nhanh 2: Người thuê
    if (Widgets::NutPhu(u8"Người thuê: 0909876543", ImVec2(chipWidth, 32.0f))) {
        loginSDT_ = "0909876543";
        loginMatKhau_ = "123456";
        loginLoiSDT_.clear();
        loginLoiMatKhau_.clear();
        loginLoiChung_.clear();
        Widgets::ToastThongTin(u8"Đã điền thông tin tài khoản Người thuê mẫu");
    }

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // Ô SĐT
    Widgets::InputPhone(
        u8"Số điện thoại:",
        "login_sdt",
        loginSDT_,
        loginLoiSDT_.empty() ? nullptr : loginLoiSDT_.c_str(),
        u8"Nhập số điện thoại (10 chữ số)"
    );
    ImGui::Spacing();

    // Ô Mật khẩu
    Widgets::InputPassword(
        u8"Mật khẩu:",
        "login_pw",
        loginMatKhau_,
        loginHienMatKhau_,
        loginLoiMatKhau_.empty() ? nullptr : loginLoiMatKhau_.c_str(),
        u8"Nhập mật khẩu tài khoản..."
    );

    // Phím Enter tự động kích hoạt đăng nhập
    if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter)) {
        xuLyDangNhap();
    }

    ImGui::Spacing();

    // Thông báo lỗi nếu có
    if (!loginLoiChung_.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Danger);
        ImGui::TextWrapped(u8"✕ %s", loginLoiChung_.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    // Nút Đăng nhập
    bool biKhoa = (thoiGianKhoaConLai_ > 0.0f);
    if (biKhoa) {
        ImGui::BeginDisabled();
        char lockText[64];
        std::snprintf(lockText, sizeof(lockText), u8"Tạm khóa (thử lại sau %.0fs)", std::ceil(thoiGianKhoaConLai_));
        Widgets::NutPhu(lockText, ImVec2(-1.0f, 42.0f));
        ImGui::EndDisabled();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Warning);
        ImGui::TextWrapped(u8"Bạn đã nhập sai mật khẩu 3 lần. Hệ thống tạm khóa 10 giây để bảo vệ an toàn.");
        ImGui::PopStyleColor();
    } else {
        if (Widgets::NutChinh(u8"Đăng nhập ngay", ImVec2(-1.0f, 42.0f))) {
            xuLyDangNhap();
        }
    }
}

void AuthScreen::renderTabDangKy() {
    ImFont* titleFont = GUI::Theme::getFonts().title;
    if (titleFont != nullptr) ImGui::PushFont(titleFont);
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
    ImGui::TextUnformatted(u8"Tạo tài khoản mới");
    ImGui::PopStyleColor();
    if (titleFont != nullptr) ImGui::PopFont();

    ImFont* smallFont = GUI::Theme::getFonts().small;
    if (smallFont != nullptr) ImGui::PushFont(smallFont);
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
    ImGui::TextUnformatted(u8"Chọn vai trò của bạn để đăng ký vào hệ thống:");
    ImGui::PopStyleColor();
    if (smallFont != nullptr) ImGui::PopFont();

    ImGui::Spacing();

    // LÝ DO CHỌN: Hai thẻ vai trò tương tác trực quan với phân cấp rõ rệt
    float cardW = (ImGui::GetContentRegionAvail().x - 10.0f) * 0.5f;

    // Thẻ Người thuê
    bool isRenter = (regVaiTro_ == 1);
    if (isRenter) {
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
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, isRenter ? 2.0f : 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

    if (ImGui::Button(u8"Người thuê sân", ImVec2(cardW, 44.0f))) {
        regVaiTro_ = 1;
    }
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);

    ImGui::SameLine();

    // Thẻ Chủ sân
    bool isOwner = (regVaiTro_ == 0);
    if (isOwner) {
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
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, isOwner ? 2.0f : 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

    if (ImGui::Button(u8"Chủ sân bóng", ImVec2(cardW, 44.0f))) {
        regVaiTro_ = 0;
    }
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // Họ và tên
    Widgets::InputText(
        regVaiTro_ == 0 ? u8"Họ tên chủ sân (*):" : u8"Họ và tên (*):",
        "reg_name",
        regHoTen_,
        regLoiHoTen_.empty() ? nullptr : regLoiHoTen_.c_str(),
        u8"Ví dụ: Nguyễn Văn A"
    );
    ImGui::Spacing();

    // Số điện thoại
    Widgets::InputPhone(
        u8"Số điện thoại đăng ký (*):",
        "reg_phone",
        regSDT_,
        regLoiSDT_.empty() ? nullptr : regLoiSDT_.c_str(),
        u8"0xxxxxxxxx"
    );
    ImGui::Spacing();

    // Dành riêng cho Chủ sân
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
            u8"Địa chỉ cụm sân (*):",
            "reg_diachi",
            regDiaChi_,
            regLoiDiaChi_.empty() ? nullptr : regLoiDiaChi_.c_str(),
            u8"Số nhà, tên đường, quận/huyện, thành phố..."
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

    if (Widgets::NutChinh(u8"Hoàn tất đăng ký tài khoản", ImVec2(-1.0f, 42.0f))) {
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

    auto ketQua = GUI::App::Facade::getInstance().dangNhap(loginSDT_, loginMatKhau_);

    if (ketQua.ok && ketQua.duLieu) {
        soLanDangNhapSai_ = 0;
        thoiGianKhoaConLai_ = 0.0f;

        Widgets::ToastThanhCong(u8"Đăng nhập thành công! Chào mừng " + ketQua.duLieu->get_name());

        GUI::App::Facade::getInstance().moManHinhChinhChoUser(
            ketQua.duLieu.get(),
            GUI::App::ScreenManager::getInstance()
        );
    } else {
        soLanDangNhapSai_++;
        loginLoiChung_ = ketQua.loi;

        if (soLanDangNhapSai_ >= 3) {
            thoiGianKhoaConLai_ = 10.0f;
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
        activeTab_ = 0;
        Widgets::ToastThanhCong(u8"Đăng ký tài khoản thành công! Bạn có thể đăng nhập ngay.");
    }
}

}  // namespace GUI::Screens::Auth
