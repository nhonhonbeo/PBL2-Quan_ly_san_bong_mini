#pragma once
#include <string>
class ReportService {
public:
    long long calculateRevenue(const std::string& fromDate, const std::string& toDate) const;
};
