#pragma once
#include <string>

#include "Core/LinkedList.h"

class ReportRepository;
struct ReportFieldRecord;
struct ReportBookingRecord;

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
private:
    const ReportRepository& repository_;

public:
    explicit ReportService(const ReportRepository& repository);

    bool loadFieldRecords(LinkedList<ReportFieldRecord>& fields,
                          std::string& error) const;
    bool loadBookingRecords(LinkedList<ReportBookingRecord>& bookings,
                            std::string& error) const;
    bool buildOwnerDashboard(
        const std::string& ownerUserId,
        ReportDashboardMetrics& metrics,
        LinkedList<ReportRevenueDay>& revenueByDay,
        LinkedList<ReportBookingRecord>& recentBookings,
        std::string& error
    ) const;
};
