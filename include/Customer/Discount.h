#ifndef DISCOUNT_H
#define DISCOUNT_H

#include <string>

enum class DiscountStatus {
    AVAILABLE,
    USED,
    EXPIRED
};

// LÝ DO CHỌN: Discount biểu diễn ưu đãi đã cấp cho một khách để lưu được trạng
// thái sử dụng; điều kiện cấp ưu đãi thuộc CustomerService.
class Discount {
private:
    std::string discount_id_;
    std::string customer_id_;
    int percentage_;
    int qualified_booking_count_;
    std::string granted_at_;
    DiscountStatus status_;
    std::string used_booking_id_;

public:
    Discount(const std::string& discountId, const std::string& customerId,
             int percentage, int qualifiedBookingCount,
             const std::string& grantedAt,
             DiscountStatus status = DiscountStatus::AVAILABLE,
             const std::string& usedBookingId = "");

    const std::string& getDiscountId() const;
    const std::string& getCustomerId() const;
    int getPercentage() const;
    int getQualifiedBookingCount() const;
    const std::string& getGrantedAt() const;
    DiscountStatus getStatus() const;
    const std::string& getUsedBookingId() const;
    bool isAvailable() const;

    // CustomerService chỉ gọi sau khi xác nhận Booking đủ điều kiện dùng ưu đãi.
    void markUsed(const std::string& bookingId);
    void markExpired();
};

#endif
