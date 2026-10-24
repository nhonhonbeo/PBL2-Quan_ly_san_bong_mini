#include "Field/TimeSlot.h"

TimeSlot::TimeSlot(const std::string& startTime, const std::string& endTime)
    : start_time_(startTime), end_time_(endTime) {}

const std::string& TimeSlot::getStartTime() const { return start_time_; }

const std::string& TimeSlot::getEndTime() const { return end_time_; }
