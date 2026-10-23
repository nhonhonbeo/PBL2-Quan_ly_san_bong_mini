#pragma once

#include <string>

// LÝ DO CHỌN: BookingDetail giữ từng ca trong một đơn, cho phép một đơn đặt nhiều ca liên tiếp.
class BookingDetail {
    std::string slot_id_;
    std::string start_time_;
    std::string end_time_;
    long long amount_vnd_;
public:
    BookingDetail();
    BookingDetail(const std::string& slotId, const std::string& startTime,
                  const std::string& endTime, long long amountVnd);
    const std::string& getSlotId() const;
    const std::string& getStartTime() const;
    const std::string& getEndTime() const;
    long long getAmountVnd() const;
};
