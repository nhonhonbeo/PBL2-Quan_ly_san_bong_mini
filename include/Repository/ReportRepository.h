#pragma once

#include <string>

#include "Core/LinkedList.h"

struct ReportFieldRecord {
    std::string fieldId;
    std::string ownerUserId;
    std::string name;
    int size = 5;
    long long hourlyRateVnd = 0;
    bool underMaintenance = false;
    std::string rules;
};

struct ReportBookingRecord {
    std::string bookingId;
    std::string fieldId;
    std::string customerId;
    std::string customerName;
    std::string customerPhone;
    std::string date;
    std::string timeSlot;
    std::string startTime;
    std::string endTime;
    int durationMinutes = 90;
    long long amountVnd = 0;
    std::string status;
    std::string ownerUserId;
};

// LÝ DO CHỌN: Repository đọc/ghi dữ liệu báo cáo bằng LinkedList và chỉ trả
// về bản ghi có đủ mã chủ sân, tránh gán nhầm dữ liệu cũ không rõ quyền sở hữu.
class ReportRepository {
public:
    bool loadFields(LinkedList<ReportFieldRecord>& fields,
                    std::string& error) const;
    bool saveFields(const LinkedList<ReportFieldRecord>& fields,
                    std::string& error) const;
    bool loadBookings(LinkedList<ReportBookingRecord>& bookings,
                      std::string& error) const;
    bool saveBookings(const LinkedList<ReportBookingRecord>& bookings,
                      std::string& error) const;
};
