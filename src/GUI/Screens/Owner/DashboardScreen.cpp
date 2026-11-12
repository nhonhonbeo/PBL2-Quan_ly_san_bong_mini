#include "GUI/Screens/Owner/DashboardScreen.h"

#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <imgui.h>

#include "GUI/Theme/Fonts.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Card.h"

namespace GUI::Screens::Owner {

namespace {

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
    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    if (!loiBaoCao_.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Danger);
        ImGui::TextWrapped(u8"[!] Không thể tải báo cáo: %s", loiBaoCao_.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }
    renderStatCards();
    ImGui::Spacing();
    renderRevenueChart();
    ImGui::Spacing();
    renderUpcomingBookings();
    ImGui::Dummy(ImVec2(0.0f, 8.0f));
}

void DashboardScreen::renderStatCards() {
    float availW = ImGui::GetContentRegionAvail().x;
    float cardSpacing = 16.0f;
    float cardW = (availW - cardSpacing * 3.0f) / 4.0f;

    auto veTheKPI = [cardW](const char* id, const char* icon, const char* tieuDe, const char* giaTri, const char* moTa, ImVec4 accentColor) {
        ImGui::PushID(id);
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        float cardH = 118.0f;
        ImVec2 p1 = ImVec2(p0.x + cardW, p0.y + cardH);

        ImDrawList* dl = ImGui::GetWindowDrawList();

        // 1. Bóng đổ khuếch tán nhẹ
        dl->AddRectFilled(ImVec2(p0.x, p0.y + 2.0f), ImVec2(p1.x, p1.y + 4.0f), IM_COL32(0, 0, 0, 10), 8.0f);

        // 2. Nền trắng và viền chuẩn
        dl->AddRectFilled(p0, p1, GUI::Theme::Colors::toU32(GUI::Theme::Colors::Surface), 8.0f);
        dl->AddRect(p0, p1, GUI::Theme::Colors::toU32(GUI::Theme::Colors::Border), 8.0f, 0, 1.0f);

        // 3. Vạch màu chỉ báo trên đỉnh
        dl->AddRectFilled(p0, ImVec2(p1.x, p0.y + 3.5f), GUI::Theme::Colors::toU32(accentColor), 8.0f, ImDrawFlags_RoundCornersTop);

        // 4. Nội dung text
        ImGui::SetCursorScreenPos(ImVec2(p0.x + 18.0f, p0.y + 14.0f));
        ImGui::BeginGroup();

        ImFont* smallFont = GUI::Theme::getFonts().small;
        if (smallFont != nullptr) ImGui::PushFont(smallFont);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
        if (icon && icon[0] != '\0') ImGui::Text("%s  %s", icon, tieuDe); else ImGui::TextUnformatted(tieuDe);
        ImGui::PopStyleColor();
        if (smallFont != nullptr) ImGui::PopFont();

        ImGui::Spacing();

        ImFont* headingFont = GUI::Theme::getFonts().heading;
        if (headingFont != nullptr) ImGui::PushFont(headingFont);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
        ImGui::TextUnformatted(giaTri);
        ImGui::PopStyleColor();
        if (headingFont != nullptr) ImGui::PopFont();

        ImGui::Spacing();

        if (smallFont != nullptr) ImGui::PushFont(smallFont);
        ImGui::PushStyleColor(ImGuiCol_Text, accentColor);
        ImGui::TextUnformatted(moTa);
        ImGui::PopStyleColor();
        if (smallFont != nullptr) ImGui::PopFont();

        ImGui::EndGroup();

        ImGui::SetCursorScreenPos(ImVec2(p0.x + cardW, p0.y));
        ImGui::Dummy(ImVec2(0.0f, cardH));
        ImGui::PopID();
    };

    // Thẻ 1: Tổng số sân
    veTheKPI(
        "kpi_tong_san",
        "",
        u8"Tổng số sân bóng",
        (std::to_string(metrics_.totalFields) + u8" sân").c_str(),
        u8"Sân thuộc cơ sở của bạn",
        GUI::Theme::Colors::Primary
    );
    ImGui::SameLine(0.0f, cardSpacing);

    // Thẻ 2: Lượt đặt hôm nay
    veTheKPI(
        "kpi_luot_dat",
        "",
        u8"Lượt đặt hôm nay",
        (std::to_string(metrics_.bookingsToday) + u8" lượt").c_str(),
        u8"Đơn sẵn sàng phục vụ",
        GUI::Theme::Colors::Info
    );
    ImGui::SameLine(0.0f, cardSpacing);

    // Thẻ 3: Doanh thu tháng
    std::string dtStr = dinhDangTien(metrics_.monthlyRevenueVnd) + u8" đ";
    veTheKPI(
        "kpi_doanh_thu",
        "",
        u8"Doanh thu tháng này",
        dtStr.c_str(),
        u8"Đơn hoàn tất hoặc đã thanh toán",
        GUI::Theme::Colors::Warning
    );
    ImGui::SameLine(0.0f, cardSpacing);

    // Thẻ 4: Khách hàng mới
    veTheKPI(
        "kpi_khach_hang",
        "",
        u8"Khách hàng mới",
        (std::to_string(metrics_.newCustomersThisMonth) + u8" khách").c_str(),
        u8"Đăng ký trong tháng này",
        GUI::Theme::Colors::Success
    );
}

void DashboardScreen::renderRevenueChart() {
    Widgets::Card("card_revenue_chart", u8"Biểu đồ doanh thu 7 ngày gần nhất", [this]() {
        if (doanhThu7Ngay_.isEmpty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
            ImGui::TextUnformatted(u8"Chưa có dữ liệu doanh thu gần đây.");
            ImGui::PopStyleColor();
            return;
        }

        float availW = ImGui::GetContentRegionAvail().x;
        float chartH = 220.0f;
        float labelYWidth = 70.0f;
        float plotW = availW - labelYWidth - 10.0f;
        float plotH = chartH - 40.0f;

        ImVec2 chartStart = ImGui::GetCursorScreenPos();
        ImVec2 plotStart = ImVec2(chartStart.x + labelYWidth, chartStart.y + 10.0f);
        ImVec2 plotEnd   = ImVec2(plotStart.x + plotW, plotStart.y + plotH);

        ImDrawList* dl = ImGui::GetWindowDrawList();

        long long maxVal = 0;
        for (auto* node = doanhThu7Ngay_.get_head(); node != nullptr; node = node->next) {
            if (node->data.amountVnd > maxVal) {
                maxVal = node->data.amountVnd;
            }
        }
        const long long donViLamTron = 1000000LL;
        maxVal = ((maxVal + donViLamTron - 1) / donViLamTron) * donViLamTron;
        if (maxVal <= 0) maxVal = 1000000LL;

        // Vẽ 4 vạch lưới ngang và nhãn trục Y
        int numGridLines = 4;
        ImFont* smallFont = GUI::Theme::getFonts().small;

        for (int i = 0; i <= numGridLines; ++i) {
            float ratio = static_cast<float>(i) / numGridLines;
            float lineY = plotEnd.y - ratio * plotH;

            dl->AddLine(
                ImVec2(plotStart.x, lineY),
                ImVec2(plotEnd.x, lineY),
                GUI::Theme::Colors::toU32(GUI::Theme::Colors::Border, 0.5f),
                1.0f
            );

            long long curVal = static_cast<long long>(ratio * maxVal);
            std::string labelY = std::to_string(curVal / 1000000LL) + u8" tr";
            if (curVal == 0) labelY = "0";

            if (smallFont != nullptr) ImGui::PushFont(smallFont);
            ImVec2 txtSize = ImGui::CalcTextSize(labelY.c_str());
            dl->AddText(
                ImVec2(plotStart.x - txtSize.x - 8.0f, lineY - txtSize.y * 0.5f),
                GUI::Theme::Colors::toU32(GUI::Theme::Colors::TextSecondary),
                labelY.c_str()
            );
            if (smallFont != nullptr) ImGui::PopFont();
        }

        // Đường đáy trục hoành X
        dl->AddLine(
            ImVec2(plotStart.x, plotEnd.y),
            ImVec2(plotEnd.x, plotEnd.y),
            GUI::Theme::Colors::toU32(GUI::Theme::Colors::Border),
            1.5f
        );

        // Vẽ các cột doanh thu
        if (!doanhThu7Ngay_.isEmpty() && maxVal > 0) {
            const size_t numCols = static_cast<size_t>(doanhThu7Ngay_.size());
            float colSlotW = plotW / static_cast<float>(numCols);
            float barW = colSlotW * 0.42f;

            ImVec2 mousePos = ImGui::GetIO().MousePos;

            size_t i = 0;
            for (auto* node = doanhThu7Ngay_.get_head(); node != nullptr; node = node->next, ++i) {
                const ReportRevenueDay& item = node->data;
                float barRatio = static_cast<float>(item.amountVnd) / static_cast<float>(maxVal);
                float curBarH = barRatio > 0.0f ? barRatio * plotH : 0.0f;

                float centerX = plotStart.x + i * colSlotW + colSlotW * 0.5f;
                ImVec2 barMin(centerX - barW * 0.5f, plotEnd.y - curBarH);
                ImVec2 barMax(centerX + barW * 0.5f, plotEnd.y);

                bool isHovered = (mousePos.x >= barMin.x && mousePos.x <= barMax.x &&
                                  mousePos.y >= barMin.y && mousePos.y <= barMax.y);

                ImU32 barCol = isHovered
                    ? GUI::Theme::Colors::toU32(GUI::Theme::Colors::PrimaryHover)
                    : GUI::Theme::Colors::toU32(GUI::Theme::Colors::Primary);

                if (item.amountVnd > 0) {
                    dl->AddRectFilled(barMin, barMax, barCol, 4.0f, ImDrawFlags_RoundCornersTop);
                }

                if (smallFont != nullptr) ImGui::PushFont(smallFont);
                ImVec2 xLabelSize = ImGui::CalcTextSize(item.label.c_str());
                dl->AddText(
                    ImVec2(centerX - xLabelSize.x * 0.5f, plotEnd.y + 8.0f),
                    GUI::Theme::Colors::toU32(GUI::Theme::Colors::TextPrimary),
                    item.label.c_str()
                );
                if (smallFont != nullptr) ImGui::PopFont();

                if (isHovered) {
                    ImGui::BeginTooltip();
                    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
                    ImGui::Text(u8"Ngày: %s", item.label.c_str());
                    ImGui::Separator();
                    ImGui::Text(u8"Doanh thu: %s VNĐ", dinhDangTien(item.amountVnd).c_str());
                    ImGui::PopStyleColor();
                    ImGui::EndTooltip();
                }
            }
        }

        ImGui::Dummy(ImVec2(availW, chartH));
    });
}

void DashboardScreen::renderUpcomingBookings() {
    Widgets::Card("card_upcoming_bookings", u8"Danh sách lượt đặt sân gần đây", [this]() {
        if (luotDatGanDay_.isEmpty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
            ImGui::TextUnformatted(u8"Hiện chưa có lượt đặt sân nào gần đây.");
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
            ImGui::TableSetupColumn(u8"Số điện thoại", ImGuiTableColumnFlags_WidthFixed, 115.0f);
            ImGui::TableSetupColumn(u8"Khung giờ", ImGuiTableColumnFlags_WidthFixed, 130.0f);
            ImGui::TableSetupColumn(u8"Tiền sân (VNĐ)", ImGuiTableColumnFlags_WidthFixed, 115.0f);
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
                ImGui::Text("%s đ", dinhDangTien(don.amountVnd).c_str());

                ImGui::TableSetColumnIndex(7);
                if (don.status == u8"Đã thanh toán" || don.status == u8"Đã hoàn thành") {
                    Widgets::BadgeDaDat(don.status.c_str());
                } else if (don.status == u8"Đã hủy") {
                    Widgets::BadgeDanhSachDen(don.status.c_str());
                } else {
                    Widgets::Badge(don.status.c_str(), GUI::Theme::Colors::PrimaryLight, GUI::Theme::Colors::Primary);
                }
            }

            ImGui::EndTable();
        }
    });
}

}  // namespace GUI::Screens::Owner
