#ifndef NGUOITHUE_H
#define NGUOITHUE_H
#include <string>
#include "Account/User.h"

// LÝ DO CHỌN: Customer kế thừa User, đóng gói toàn bộ thống kê và nghiệp vụ của khách hàng thuê sân.
class Customer : public User {
private:
    int booking_count;
    int cancel_count;
public:
    Customer(
        const std::string& user_id = "",
        const std::string& user_name = "",
        const std::string& password = "",
        const std::string& name = "",
        const std::string& phone_number = ""
    );
    
    void show_menu() const override;
    std::string get_account_type() const override;

    int get_booking_count() const;
    int get_cancel_count() const;

    void increase_booking_count();
    void increase_cancel_count();

    bool reg_customer() const;

    const std::string& getUserId() const;
    int getBookingCount() const;
    int getCancelCount() const;
    void recordBooking();
    void recordCancellation();
    void setStatistics(int bookingCount, int cancelCount);

    // Nghiệp vụ tự thân của Khách hàng (Rich Domain Model)
    bool isBlacklistCandidate(int minCancellations = 3) const;
    bool qualifiesForDiscount(int minBookings = 5) const;
    bool canBook(bool isBlocked, std::string& error) const;
    int getReliabilityRate() const;
};
#endif
