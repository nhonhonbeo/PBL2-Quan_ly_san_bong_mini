#ifndef CUSTOMER_PROFILE_H
#define CUSTOMER_PROFILE_H

#include <string>

// LÝ DO CHỌN: Tên CustomerProfile tránh trùng class Customer hiện có trong
// Account khi module Account chưa được phép đổi tên thành Renter.
class CustomerProfile {
private:
    std::string user_id_;
    int booking_count_;
    int cancel_count_;

public:
    CustomerProfile(const std::string& userId, int bookingCount = 0,
                    int cancelCount = 0);

    const std::string& getUserId() const;
    int getBookingCount() const;
    int getCancelCount() const;

    // Chỉ CustomerService hoặc BookingService được gọi sau khi nghiệp vụ hợp lệ.
    void recordBooking();
    void recordCancellation();

    // Repository dùng khi khôi phục dữ liệu đã được lưu.
    void setStatistics(int bookingCount, int cancelCount);
};

#endif
