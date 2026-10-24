#include "GUI/Screens/Owner/DashboardScreen.h"

#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <imgui.h>

#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Card.h"

namespace GUI::Screens::Owner {

namespace {

// Hàm định dạng số tiền có dấu phân cách hàng nghìn.
std::string dinhDangTien(long long amount) {
    std::string s = std::to_string(amount);
    int n = static_cast<int>(s.length()) - 3;
    while (n > 0) {
        s.insert(n, ".");
        n -= 3;
    }
    return s;
}

}  // namespace

void DashboardScreen::onEnter() {
    // LÝ DO CHỌN: Toàn bộ dữ liệu ViewModel được nạp tập trung một lần duy nhất tại onEnter(),
    // tuyệt đối không truy vấn I/O hay tính toán nặng trong vòng lặp render().
    auto& facade = GUI::App::Facade::getInstance();
    metrics_ = {};
    loiBaoCao_.clear();
    doanhThu7Ngay_.clear();
    luotDatGanDay_.clear();
    facade.getBaoCaoDashboard(
        metrics_,
        doanhThu7Ngay_,
        luotDatGanDay_,
        loiBaoCao_
    );
}

void DashboardScreen::onExit() {
}

void DashboardScreen::render() {
    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    ImGui::Indent(8.0f);
    if (!loiBaoCao_.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
        ImGui::TextWrapped(u8"Không thể tải báo cáo: %s", loiBaoCao_.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }
    renderStatCards();
    ImGui::Spacing();
    renderRevenueChart();
    ImGui::Spacing();
    renderUpcomingBookings();
    ImGui::Unindent(8.0f);
    ImGui::Dummy(ImVec2(0.0f, 8.0f));
}

void DashboardScreen::renderStatCards() {
    float availW = ImGui::GetContentRegionAvail().x;
    float cardSpacing = 16.0f;
    float cardW = (availW - cardSpacing * 3.0f) / 4.0f;

    auto veTheKPI = [cardW](const char* id, const char* icon, const char* tieuDe, const char* giaTri, const char* moTa, ImVec4 accentColor) {
        ImGui::PushID(id);
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        ImVec2 p1 = ImVec2(p0.x + cardW, p0.y + 110.0f);

        ImDrawList* dl = ImGui::GetWindowDrawList();

        // 1. Bóng đổ mềm
        dl->AddRectFilled(ImVec2(p0.x, p0.y + 2.0f), ImVec2(p1.x, p1.y + 3.0f), IM_COL32(0, 0, 0, 14), 8.0f);

        // 2. Nền trắng
        dl->AddRectFilled(p0, p1, GUI::Theme::Colors::toU32(GUI::Theme::Colors::WhiteCard), 8.0f);
        dl->AddRect(p0, p1, GUI::Theme::Colors::toU32(GUI::Theme::Colors::BorderLight), 8.0f, 0, 1.0f);

        // 3. Vạch màu chỉ báo trên đỉnh
        dl->AddRectFilled(p0, ImVec2(p1.x, p0.y + 4.0f), GUI::Theme::Colors::toU32(accentColor), 8.0f, ImDrawFlags_RoundCornersTop);

        // 4. Nội dung text
        ImGui::SetCursorScreenPos(ImVec2(p0.x + 16.0f, p0.y + 16.0f));
        ImGui::BeginGroup();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::Text("%s  %s", icon, tieuDe);
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::TextUnformatted(giaTri);
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, accentColor);
        ImGui::TextUnformatted(moTa);
        ImGui::PopStyleColor();

        ImGui::EndGroup();

        ImGui::SetCursorScreenPos(ImVec2(p0.x + cardW, p0.y));
        ImGui::Dummy(ImVec2(0.0f, 110.0f));
        ImGui::PopID();
    };

    // Thẻ 1: Tổng số sân
    veTheKPI(
        "kpi_tong_san",
        "",
        u8"Tổng số sân bóng",
        (std::to_string(metrics_.totalFields) + u8" sân").c_str(),
        u8"Sân thuộc tài khoản của bạn",
        GUI::Theme::Colors::GrassGreen
    );
    ImGui::SameLine(0.0f, cardSpacing);

    // Thẻ 2: Lượt đặt hôm nay
    veTheKPI(
        "kpi_luot_dat",
        "",
        u8"Lượt đặt hôm nay",
        (std::to_string(metrics_.bookingsToday) + u8" lượt").c_str(),
        u8"Đơn chưa bị hủy",
        GUI::Theme::Colors::BlueInfo
    );
    ImGui::SameLine(0.0f, cardSpacing);

    // Thẻ 3: Doanh thu tháng
    std::string dtStr = dinhDangTien(metrics_.monthlyRevenueVnd) + u8" VNĐ";
    veTheKPI(
        "kpi_doanh_thu",
        "",
        u8"Doanh thu tháng này",
        dtStr.c_str(),
        u8"Đơn hoàn tất hoặc đã thanh toán",
        GUI::Theme::Colors::GoldAccent
    );
    ImGui::SameLine(0.0f, cardSpacing);

    // Thẻ 4: Khách hàng mới
    veTheKPI(
        "kpi_khach_moi",
        "",
        u8"Khách hàng mới",
        (std::to_string(metrics_.newCustomersThisMonth) + u8" khách").c_str(),
        u8"Đơn đầu tiên trong tháng này",
        ImVec4(0.55f, 0.35f, 0.85f, 1.0f)
    );
}

void DashboardScreen::renderRevenueChart() {
    Widgets::Card("card_revenue_chart", u8"Biểu đồ doanh thu 7 ngày gần nhất", [this]() {
        if (doanhThu7Ngay_.isEmpty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::TextUnformatted(u8"Chưa có dữ liệu doanh thu");
            ImGui::PopStyleColor();
            return;
        }

        float availW = ImGui::GetContentRegionAvail().x;
        float chartH = 220.0f;
        float labelYWidth = 80.0f;
        float plotW = availW - labelYWidth - 10.0f;
        float plotH = chartH - 40.0f; // Dành 40px cho nhãn trục X ở đáy

        ImVec2 chartStart = ImGui::GetCursorScreenPos();
        ImVec2 plotStart = ImVec2(chartStart.x + labelYWidth, chartStart.y + 10.0f);
        ImVec2 plotEnd   = ImVec2(plotStart.x + plotW, plotStart.y + plotH);

        ImDrawList* dl = ImGui::GetWindowDrawList();

        // 1. Tìm giá trị cực đại để chia tỷ lệ trục Y
        long long maxVal = 0;
        for (auto* node = doanhThu7Ngay_.get_head(); node != nullptr; node = node->next) {
            if (node->data.amountVnd > maxVal) {
                maxVal = node->data.amountVnd;
            }
        }
        const long long donViLamTron = 1000000LL;
        maxVal = ((maxVal + donViLamTron - 1) / donViLamTron) * donViLamTron;

        // 2. Vẽ 4 vạch lưới ngang và nhãn trục Y
        int numGridLines = 4;
        for (int i = 0; i <= numGridLines; ++i) {
            float ratio = static_cast<float>(i) / numGridLines;
            float lineY = plotEnd.y - ratio * plotH;

            // Đường dóng ngang mờ
            dl->AddLine(
                ImVec2(plotStart.x, lineY),
                ImVec2(plotEnd.x, lineY),
                GUI::Theme::Colors::toU32(GUI::Theme::Colors::BorderLight, 0.6f),
                1.0f
            );

            // Nhãn số tiền trục Y
            long long curVal = static_cast<long long>(ratio * maxVal);
            std::string labelY = std::to_string(curVal / 1000000LL) + u8" tr";
            if (curVal == 0) labelY = "0";

            ImVec2 txtSize = ImGui::CalcTextSize(labelY.c_str());
            dl->AddText(
                ImVec2(plotStart.x - txtSize.x - 8.0f, lineY - txtSize.y * 0.5f),
                GUI::Theme::Colors::toU32(GUI::Theme::Colors::MutedText),
                labelY.c_str()
            );
        }

        // 3. Đường đáy trục hoành X
        dl->AddLine(
            ImVec2(plotStart.x, plotEnd.y),
            ImVec2(plotEnd.x, plotEnd.y),
            GUI::Theme::Colors::toU32(GUI::Theme::Colors::Gray),
            1.5f
        );

        // 4. Vẽ các cột doanh thu
        if (!doanhThu7Ngay_.isEmpty() && maxVal > 0) {
            const size_t numCols = static_cast<size_t>(doanhThu7Ngay_.size());
            float colSlotW = plotW / static_cast<float>(numCols);
            float barW = colSlotW * 0.45f;

            ImVec2 mousePos = ImGui::GetIO().MousePos;

            size_t i = 0;
            for (auto* node = doanhThu7Ngay_.get_head(); node != nullptr; node = node->next, ++i) {
                const ReportRevenueDay& item = node->data;
                float barRatio = static_cast<float>(item.amountVnd) / static_cast<float>(maxVal);
                float curBarH = barRatio > 0.0f ? barRatio * plotH : 0.0f;

                float centerX = plotStart.x + i * colSlotW + colSlotW * 0.5f;
                ImVec2 barMin(centerX - barW * 0.5f, plotEnd.y - curBarH);
                ImVec2 barMax(centerX + barW * 0.5f, plotEnd.y);

                // Kiểm tra xem chuột có hover vào vùng cột không
                bool isHovered = (mousePos.x >= barMin.x && mousePos.x <= barMax.x &&
                                  mousePos.y >= barMin.y && mousePos.y <= barMax.y);

                // Màu cột: Xanh cỏ #1B8A5A; khi hover đổi sang xanh sáng nổi bật #22C55E
                ImU32 barCol = isHovered
                    ? IM_COL32(34, 197, 94, 255)
                    : GUI::Theme::Colors::toU32(GUI::Theme::Colors::GrassGreen);

                // Vẽ cột bo góc trên
                if (item.amountVnd > 0) {
                    dl->AddRectFilled(barMin, barMax, barCol, 4.0f, ImDrawFlags_RoundCornersTop);
                }

                // Nhãn trục X dưới chân cột (T2, T3, T4...)
                ImVec2 xLabelSize = ImGui::CalcTextSize(item.label.c_str());
                dl->AddText(
                    ImVec2(centerX - xLabelSize.x * 0.5f, plotEnd.y + 8.0f),
                    GUI::Theme::Colors::toU32(GUI::Theme::Colors::DarkText),
                    item.label.c_str()
                );

                // Hiển thị Tooltip chi tiết khi rê chuột
                if (isHovered) {
                    ImGui::BeginTooltip();
                    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
                    ImGui::Text(u8"Ngày: %s", item.label.c_str());
                    ImGui::Separator();
                    ImGui::Text(u8"Doanh thu: %s VNĐ", dinhDangTien(item.amountVnd).c_str());
                    ImGui::PopStyleColor();
                    ImGui::EndTooltip();
                }
            }
        }

        // Đẩy layout xuống dưới biểu đồ
        ImGui::Dummy(ImVec2(availW, chartH));
    });
}

void DashboardScreen::renderUpcomingBookings() {
    Widgets::Card("card_upcoming_bookings", u8"Danh sách đặt sân gần đây", [this]() {
        if (luotDatGanDay_.isEmpty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::TextUnformatted(u8"Chưa có lượt đặt nào");
            ImGui::PopStyleColor();
            return;
        }

        ImGuiTableFlags flags = ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_BordersInnerV |
                                ImGuiTableFlags_BordersOuter |
                                ImGuiTableFlags_SizingStretchProp;

        if (ImGui::BeginTable("##tbl_upcoming", 8, flags)) {
            ImGui::TableSetupColumn(u8"Mã đơn", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn(u8"Mã sân", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn(u8"Ngày", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn(u8"Khách hàng", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(u8"Số điện thoại", ImGuiTableColumnFlags_WidthFixed, 110.0f);
            ImGui::TableSetupColumn(u8"Khung giờ", ImGuiTableColumnFlags_WidthFixed, 130.0f);
            ImGui::TableSetupColumn(u8"Tiền sân (VNĐ)", ImGuiTableColumnFlags_WidthFixed, 110.0f);
            ImGui::TableSetupColumn(u8"Trạng thái", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableHeadersRow();

            for (auto* node = luotDatGanDay_.get_head(); node != nullptr; node = node->next) {
                const ReportBookingRecord& don = node->data;
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(don.bookingId.c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(don.fieldId.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(don.date.c_str());

                ImGui::TableSetColumnIndex(3);
                ImGui::TextUnformatted(don.customerName.c_str());

                ImGui::TableSetColumnIndex(4);
                ImGui::TextUnformatted(don.customerPhone.c_str());

                ImGui::TableSetColumnIndex(5);
                ImGui::TextUnformatted(don.timeSlot.c_str());

                ImGui::TableSetColumnIndex(6);
                ImGui::Text("%s VNĐ", dinhDangTien(don.amountVnd).c_str());

                ImGui::TableSetColumnIndex(7);
                if (don.status == u8"Đã thanh toán" || don.status == u8"Đã hoàn thành") {
                    Widgets::BadgeDaDat(don.status.c_str());
                } else {
                    Widgets::Badge(don.status.c_str(), GUI::Theme::Colors::BlueInfo, GUI::Theme::Colors::WhiteCard);
                }
            }

            ImGui::EndTable();
        }
    });
}

}  // namespace GUI::Screens::Owner
