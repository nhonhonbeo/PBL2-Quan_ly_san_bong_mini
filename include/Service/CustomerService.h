#ifndef CUSTOMER_SERVICE_H
#define CUSTOMER_SERVICE_H

#include <string>

class CustomerRepository;
class Discount;
class Review;

// LÝ DO CHỌN: CustomerService là điểm duy nhất áp dụng rule Customer, tránh
// để Repository hoặc GUI tự quyết định blacklist/ưu đãi.
class CustomerService {
private:
    CustomerRepository& customer_repository_;

public:
    explicit CustomerService(CustomerRepository& customerRepository);

    // Phân biệt thành công kiểm tra với lỗi truy xuất file/hệ thống
    bool isBlacklisted(const std::string& userId, bool& outIsBlacklisted,
                       std::string& outReason, std::string& error) const;
    bool isBlacklisted(const std::string& userId, bool& outIsBlacklisted,
                       std::string& outReason) const;

    bool recordSuccessfulBooking(const std::string& userId,
                                 std::string& error);
    bool recordCancellation(const std::string& userId, std::string& error);
    bool addReview(const Review& review, std::string& error);
    bool findAvailableDiscount(const std::string& userId, Discount& out,
                               std::string& error) const;
};

#endif