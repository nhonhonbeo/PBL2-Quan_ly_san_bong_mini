#ifndef CUSTOMER_SERVICE_H
#define CUSTOMER_SERVICE_H

#include <map>
#include <string>
#include <vector>

#include "Core/LinkedList.h"
#include "Customer/Blacklist.h"
#include "Customer/Review.h"

class Customer;
class CustomerRepository;
class Discount;

struct CustomerDiscountThreshold {
    int id = 0;
    int minBookings = 5;
    int percentage = 5;
    std::string tierName;
};

// LÝ DO CHỌN: CustomerService là điểm duy nhất áp dụng các quy tắc ưu đãi,
// quản lý và đề xuất danh sách đen, và tổng hợp đánh giá chất lượng sân.
class CustomerService {
private:
    CustomerRepository& customer_repository_;
    std::vector<CustomerDiscountThreshold> discount_thresholds_;

public:
    explicit CustomerService(CustomerRepository& customerRepository);

    // Ưu đãi
    const std::vector<CustomerDiscountThreshold>& getDiscountThresholds() const;
    bool updateDiscountThreshold(int id, int minBookings, int percentage, std::string& error);
    int getCustomerDiscountPercentage(const std::string& userIdOrPhone, std::string& error) const;
    bool applyDiscountsToAll(std::string& error);

    // Danh sách đen
    bool isBlacklisted(const std::string& userId, bool& outIsBlacklisted,
                       std::string& outReason, std::string& error) const;
    bool isBlacklisted(const std::string& userId, bool& outIsBlacklisted,
                       std::string& outReason) const;
    bool addToBlacklist(const std::string& customerIdOrPhone,
                        const std::string& reason,
                        std::string& error);
    bool removeFromBlacklist(const std::string& customerIdOrPhone,
                             std::string& error);
    bool getAllBlacklists(LinkedList<Blacklist>& outList, std::string& error) const;
    bool getBlacklistCandidates(int minCancellations,
                                LinkedList<Customer>& outCandidates,
                                std::string& error) const;

    // Đánh giá
    bool addReview(const Review& review, std::string& error);
    bool addOrUpdateReview(const Review& review, std::string& error);
    bool getAllReviews(LinkedList<Review>& outReviews, std::string& error) const;
    bool getAverageRatingOverall(float& outScore, std::string& error) const;
    bool getAverageRatingsMap(std::map<std::string, float>& outMap, std::string& error) const;

    // Khách hàng & thống kê
    bool recordSuccessfulBooking(const std::string& userId, std::string& error);
    bool recordCancellation(const std::string& userId, std::string& error);
    bool findAvailableDiscount(const std::string& userId, Discount& out,
                               std::string& error) const;
    bool getAllCustomers(LinkedList<Customer>& outList, std::string& error) const;
    bool findCustomer(const std::string& idOrPhone, Customer& out, std::string& error) const;
};

#endif
