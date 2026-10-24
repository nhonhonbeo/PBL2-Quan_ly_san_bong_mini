#pragma once
#include <string>

#include "Core/LinkedList.h"
#include "Repository/ReportRepository.h"

struct ReportRevenueDay {
    std::string label;
    long long amountVnd = 0;
};

struct ReportDashboardMetrics {
    int totalFields = 0;
    int bookingsToday = 0;
    long long monthlyRevenueVnd = 0;
    int newCustomersThisMonth = 0;
};

// LÝ DO CHỌN: Tách phép tổng hợp khỏi giao diện và Facade; danh sách nghiệp vụ
// dùng LinkedList tự cài đặt, dữ liệu được giới hạn theo owner ID.
class ReportService {
public:
    bool buildOwnerDashboard(
        const std::string& ownerUserId,
        const ReportRepository& repository,
        ReportDashboardMetrics& metrics,
        LinkedList<ReportRevenueDay>& revenueByDay,
        LinkedList<ReportBookingRecord>& recentBookings,
        std::string& error
    ) const;
};
