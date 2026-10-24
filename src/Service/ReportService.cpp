#include "Service/ReportService.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <ctime>
#include <stdexcept>

#include "Core/DateTime.h"

namespace {

int ngayThanhSo(const std::string& ngay) {
    if (ngay.size() != 10) {
        return 0;
    }
    try {
        int nam = 0;
        int thang = 0;
        int ngayTrongThang = 0;
        if (ngay[4] == '-' && ngay[7] == '-') {
            nam = std::stoi(ngay.substr(0, 4));
            thang = std::stoi(ngay.substr(5, 2));
            ngayTrongThang = std::stoi(ngay.substr(8, 2));
        } else if (ngay[2] == '/' && ngay[5] == '/') {
            ngayTrongThang = std::stoi(ngay.substr(0, 2));
            thang = std::stoi(ngay.substr(3, 2));
            nam = std::stoi(ngay.substr(6, 4));
        } else {
            return 0;
        }
        if (nam < 1900 || thang < 1 || thang > 12 ||
            ngayTrongThang < 1 || ngayTrongThang > 31) {
            return 0;
        }
        return nam * 10000 + thang * 100 + ngayTrongThang;
    } catch (const std::invalid_argument&) {
        return 0;
    } catch (const std::out_of_range&) {
        return 0;
    }
}

std::string dinhDangNgay(const std::tm& ngay) {
    char buffer[11];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%02d/%02d/%04d",
        ngay.tm_mday,
        ngay.tm_mon + 1,
        ngay.tm_year + 1900
    );
    return buffer;
}

std::string phanNgay(const ReportBookingRecord& booking) {
    const size_t viTri = booking.timeSlot.find('-');
    char buffer[9];
    std::snprintf(buffer, sizeof(buffer), "%08d", ngayThanhSo(booking.date));
    std::string gioBatDau = viTri == std::string::npos
        ? booking.timeSlot
        : booking.timeSlot.substr(0, viTri);
    gioBatDau.erase(
        std::remove(gioBatDau.begin(), gioBatDau.end(), ' '),
        gioBatDau.end()
    );
    return std::string(buffer) + gioBatDau;
}

bool laDonDaHoanTat(const std::string& status) {
    return status == u8"Đã hoàn thành" || status == u8"Đã thanh toán" ||
           status == "COMPLETED" || status == "PAID";
}

bool laDonDaHuy(const std::string& status) {
    return status == u8"Đã hủy" || status == "CANCELLED";
}

bool daChon(const LinkedList<std::string>& danhSach,
            const std::string& giaTri) {
    for (auto* node = danhSach.get_head(); node != nullptr; node = node->next) {
        if (node->data == giaTri) {
            return true;
        }
    }
    return false;
}

}  // Kết thúc không gian tên ẩn danh

bool ReportService::buildOwnerDashboard(
    const std::string& ownerUserId,
    const ReportRepository& repository,
    ReportDashboardMetrics& metrics,
    LinkedList<ReportRevenueDay>& revenueByDay,
    LinkedList<ReportBookingRecord>& recentBookings,
    std::string& error
) const {
    metrics = {};
    revenueByDay.clear();
    recentBookings.clear();

    LinkedList<ReportFieldRecord> fields;
    LinkedList<ReportBookingRecord> bookings;
    if (!repository.loadFields(fields, error) ||
        !repository.loadBookings(bookings, error)) {
        return false;
    }

    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        if (node->data.ownerUserId == ownerUserId) {
            ++metrics.totalFields;
        }
    }

    const int today = ngayThanhSo(DateTime::today());
    if (today == 0) {
        error = "Không thể xác định ngày hiện tại để lập báo cáo.";
        return false;
    }
    const int monthKey = today / 100;
    const int nextMonthKey = monthKey % 100 == 12 ? monthKey + 89 : monthKey + 1;
    const int monthStart = monthKey * 100;
    const int nextMonthStart = nextMonthKey * 100;

    std::array<std::string, 7> lastSevenDates;
    std::array<int, 7> lastSevenKeys{};
    std::array<long long, 7> lastSevenRevenue{};
    std::tm currentDate{};
    {
        const int key = today;
        currentDate.tm_year = key / 10000 - 1900;
        currentDate.tm_mon = (key / 100) % 100 - 1;
        currentDate.tm_mday = key % 100;
        currentDate.tm_hour = 12;
        if (std::mktime(&currentDate) == static_cast<std::time_t>(-1)) {
            error = "Không thể xác định ngày hiện tại để lập báo cáo.";
            return false;
        }
    }
    for (int i = 6; i >= 0; --i) {
        std::tm chartDate = currentDate;
        chartDate.tm_mday -= i;
        chartDate.tm_isdst = -1;
        if (std::mktime(&chartDate) == static_cast<std::time_t>(-1)) {
            error = "Không thể tính khoảng ngày cho biểu đồ doanh thu.";
            return false;
        }
        const size_t index = static_cast<size_t>(6 - i);
        lastSevenDates[index] = dinhDangNgay(chartDate);
        lastSevenKeys[index] =
            (chartDate.tm_year + 1900) * 10000 +
            (chartDate.tm_mon + 1) * 100 + chartDate.tm_mday;
    }

    LinkedList<ReportBookingRecord> ownerBookings;
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        const ReportBookingRecord& booking = node->data;
        if (booking.ownerUserId != ownerUserId) {
            continue;
        }
        bool belongsToAnotherOwner = false;
        for (auto* fieldNode = fields.get_head();
             fieldNode != nullptr;
             fieldNode = fieldNode->next) {
            if (fieldNode->data.fieldId == booking.fieldId &&
                fieldNode->data.ownerUserId != ownerUserId) {
                belongsToAnotherOwner = true;
                break;
            }
        }
        if (belongsToAnotherOwner) {
            continue;
        }
        ownerBookings.push_back(booking);

        const int bookingDate = ngayThanhSo(booking.date);
        if (bookingDate == today && !laDonDaHuy(booking.status)) {
            ++metrics.bookingsToday;
        }
        if (bookingDate >= monthStart && bookingDate < nextMonthStart &&
            laDonDaHoanTat(booking.status)) {
            metrics.monthlyRevenueVnd += booking.amountVnd;
        }
        if (laDonDaHoanTat(booking.status)) {
            for (size_t i = 0; i < lastSevenKeys.size(); ++i) {
                if (bookingDate == lastSevenKeys[i]) {
                    lastSevenRevenue[i] += booking.amountVnd;
                    break;
                }
            }
        }
    }

    LinkedList<std::string> countedCustomers;
    for (auto* bookingNode = ownerBookings.get_head();
         bookingNode != nullptr;
         bookingNode = bookingNode->next) {
        const ReportBookingRecord& booking = bookingNode->data;
        if (booking.customerPhone.empty() ||
            daChon(countedCustomers, booking.customerPhone)) {
            continue;
        }
        int firstBookingDate = 0;
        for (auto* candidate = ownerBookings.get_head();
             candidate != nullptr;
             candidate = candidate->next) {
            if (candidate->data.customerPhone == booking.customerPhone) {
                const int candidateDate = ngayThanhSo(candidate->data.date);
                if (candidateDate > 0 &&
                    (firstBookingDate == 0 || candidateDate < firstBookingDate)) {
                    firstBookingDate = candidateDate;
                }
            }
        }
        countedCustomers.push_back(booking.customerPhone);
        if (firstBookingDate >= monthStart && firstBookingDate < nextMonthStart) {
            ++metrics.newCustomersThisMonth;
        }
    }

    bool hasRevenue = false;
    for (long long amount : lastSevenRevenue) {
        hasRevenue = hasRevenue || amount > 0;
    }
    if (hasRevenue) {
        for (size_t i = 0; i < lastSevenRevenue.size(); ++i) {
            revenueByDay.push_back({lastSevenDates[i], lastSevenRevenue[i]});
        }
    }

    LinkedList<std::string> selectedBookingIds;
    for (int count = 0; count < 5; ++count) {
        const ReportBookingRecord* newest = nullptr;
        std::string newestKey;
        for (auto* node = ownerBookings.get_head(); node != nullptr; node = node->next) {
            const ReportBookingRecord& booking = node->data;
            if (daChon(selectedBookingIds, booking.bookingId)) {
                continue;
            }
            const std::string key = phanNgay(booking);
            if (newest == nullptr || key > newestKey) {
                newest = &booking;
                newestKey = key;
            }
        }
        if (newest == nullptr) {
            break;
        }
        recentBookings.push_back(*newest);
        selectedBookingIds.push_back(newest->bookingId);
    }

    return true;
}
