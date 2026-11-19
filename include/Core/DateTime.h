
#pragma once

#include <string>

class DateTime {
public:
    static std::string today();
    static std::string nowTime();
    static std::string toIsoDate(const std::string& value);
    static bool isValidDate(const std::string& value);
    static bool isValidTime(const std::string& value);
    static int compareDate(const std::string& left, const std::string& right);
    static int compareTime(const std::string& left, const std::string& right);
    static bool isPastDate(const std::string& value);
    static bool isPastSlot(const std::string& date,
                           const std::string& startTime);
    // Đổi số phút trong ngày sang "HH:MM" (một nguồn duy nhất cho toàn dự án).
    static std::string formatMinutes(int totalMinutes);
};
