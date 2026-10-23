#include "Core/DateTime.h"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace {

bool parseDate(const std::string& value, int& year, int& month, int& day) {
    if (value.size() != 10U || value[4] != '-' || value[7] != '-') return false;
    std::istringstream input(value);
    char first = '\0';
    char second = '\0';
    if (!(input >> year >> first >> month >> second >> day) || first != '-' || second != '-') return false;
    if (month < 1 || month > 12 || day < 1) return false;
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maximum = days[month - 1];
    if (month == 2 && ((year % 400 == 0) || (year % 4 == 0 && year % 100 != 0))) maximum = 29;
    return day <= maximum;
}

bool parseTime(const std::string& value, int& hour, int& minute) {
    if (value.size() != 5U || value[2] != ':') return false;
    std::istringstream input(value);
    char separator = '\0';
    return static_cast<bool>(input >> hour >> separator >> minute) && separator == ':' &&
           hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
}

std::tm localNow() {
    const std::time_t raw = std::time(nullptr);
    std::tm result{};
#ifdef _WIN32
    localtime_s(&result, &raw);
#else
    localtime_r(&raw, &result);
#endif
    return result;
}

}  // namespace

std::string DateTime::today() {
    const std::tm current = localNow();
    std::ostringstream output;
    output << std::put_time(&current, "%Y-%m-%d");
    return output.str();
}

std::string DateTime::nowTime() {
    const std::tm current = localNow();
    std::ostringstream output;
    output << std::put_time(&current, "%H:%M");
    return output.str();
}

bool DateTime::isValidDate(const std::string& value) { int y = 0, m = 0, d = 0; return parseDate(value, y, m, d); }
bool DateTime::isValidTime(const std::string& value) { int h = 0, m = 0; return parseTime(value, h, m); }

int DateTime::compareDate(const std::string& left, const std::string& right) {
    if (!isValidDate(left) || !isValidDate(right)) return 0;
    return left == right ? 0 : (left < right ? -1 : 1);
}

int DateTime::compareTime(const std::string& left, const std::string& right) {
    if (!isValidTime(left) || !isValidTime(right)) return 0;
    return left == right ? 0 : (left < right ? -1 : 1);
}

bool DateTime::isPastDate(const std::string& value) { return isValidDate(value) && compareDate(value, today()) < 0; }
