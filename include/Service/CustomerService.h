#ifndef CUSTOMER_SERVICE_H
#define CUSTOMER_SERVICE_H

#include <map>
#include <string>
#include "Core/LinkedList.h"
#include "Customer/Review.h"

class Customer;
class CustomerRepository;

// LÝ DO CHỌN: CustomerService tập trung quản lý hồ sơ, thống kê booking và đánh giá.
class CustomerService {
private:
    CustomerRepository& customer_repository_;

public:
    explicit CustomerService(CustomerRepository& customerRepository);

    // Đánh giá
    bool addReview(const Review& review, std::string& error);
    bool addOrUpdateReview(const Review& review, std::string& error);
    bool getAllReviews(LinkedList<Review>& outReviews, std::string& error) const;
    bool getAverageRatingOverall(float& outScore, std::string& error) const;
    bool getAverageRatingsMap(std::map<std::string, float>& outMap, std::string& error) const;

    // Khách hàng & thống kê (chống ghi nhận trùng lặp bằng bookingId)
    bool recordSuccessfulBooking(const std::string& userId, std::string& error);
    bool recordSuccessfulBooking(const std::string& userId, const std::string& bookingId, std::string& error);
    bool recordCancellation(const std::string& userId, std::string& error);
    bool recordCancellation(const std::string& userId, const std::string& bookingId, std::string& error);

    bool getAllCustomers(LinkedList<Customer>& outList, std::string& error) const;
    bool findCustomer(const std::string& idOrPhone, Customer& out, std::string& error) const;
};

#endif
