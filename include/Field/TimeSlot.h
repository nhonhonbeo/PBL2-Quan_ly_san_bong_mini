#ifndef TIME_SLOT_H
#define TIME_SLOT_H

#include <string>

// LÝ DO CHỌN: TimeSlot là value object dùng chung; trạng thái trống/đã đặt
// thuộc Booking theo từng ngày, không thuộc TimeSlot.
class TimeSlot {
private:
    std::string start_time_;
    std::string end_time_;

public:
    TimeSlot(const std::string& startTime, const std::string& endTime);

    const std::string& getStartTime() const;
    const std::string& getEndTime() const;
};

#endif
