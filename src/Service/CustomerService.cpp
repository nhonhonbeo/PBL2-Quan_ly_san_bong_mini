#include "Service/CustomerService.h"

#include "Account/Customer.h"
#include "Core/DateTime.h"
#include "Customer/Blacklist.h"
#include "Customer/Discount.h"
#include "Customer/Review.h"
#include "Repository/CustomerRepository.h"

CustomerService::CustomerService(CustomerRepository& customerRepository)
    : customer_repository_(customerRepository) {}

bool CustomerService::isBlacklisted(const std::string& userId,
                                    bool& outIsBlacklisted,
                                    std::string& outReason,
                                    std::string& error) const {
    outIsBlacklisted = false;
    outReason.clear();
    Blacklist bl("", "", "", false);
    std::string repoErr;
    if (customer_repository_.findActiveBlacklist(userId, bl, repoErr)) {
        if (bl.isActive()) {
            outIsBlacklisted = true;
            outReason = bl.getReason();
        }
    }
    (void)error;
    return true;
}

bool CustomerService::isBlacklisted(const std::string& userId,
                                    bool& outIsBlacklisted,
                                    std::string& outReason) const {
    std::string err;
    return isBlacklisted(userId, outIsBlacklisted, outReason, err);
}

bool CustomerService::recordSuccessfulBooking(const std::string& userId,
                                              std::string& error) {
    Customer customer("", "", "", "", "");
    if (!customer_repository_.findCustomerByUserId(userId, customer, error)) {
        return false;
    }
    customer.recordBooking();
    return customer_repository_.updateCustomer(customer, error);
}

bool CustomerService::recordCancellation(const std::string& userId,
                                         std::string& error) {
    Customer customer("", "", "", "", "");
    if (!customer_repository_.findCustomerByUserId(userId, customer, error)) {
        return false;
    }
    customer.recordCancellation();
    return customer_repository_.updateCustomer(customer, error);
}

bool CustomerService::addReview(const Review& review, std::string& error) {
    if (review.getRating() < 1 || review.getRating() > 5) {
        error = "Đánh giá phải từ 1 đến 5 sao.";
        return false;
    }
    if (review.getComment().empty()) {
        error = "Nội dung đánh giá không được để trống.";
        return false;
    }
    if (!customer_repository_.hasCompletedBooking(review.getCustomerId(), review.getFieldId())) {
        error = "Chỉ có thể đánh giá sân sau khi đã hoàn tất ca thuê tại sân này.";
        return false;
    }
    return customer_repository_.saveReview(review, error);
}

bool CustomerService::findAvailableDiscount(const std::string& userId,
                                            Discount& out,
                                            std::string& error) const {
    bool blocked = false;
    std::string blockReason;
    isBlacklisted(userId, blocked, blockReason);
    if (blocked) {
        error = "Khách hàng bị chặn không được hưởng ưu đãi.";
        return false;
    }

    Customer customer("", "", "", "", "");
    if (!customer_repository_.findCustomerByUserId(userId, customer, error)) {
        return false;
    }

    const int bookings = customer.getBookingCount();
    int percentage = 0;
    if (bookings >= 20) {
        percentage = 15;
    } else if (bookings >= 10) {
        percentage = 10;
    } else if (bookings >= 5) {
        percentage = 5;
    } else {
        error = "Chưa đạt số lần đặt tối thiểu để nhận ưu đãi.";
        return false;
    }

    char idBuf[64];
    std::snprintf(idBuf, sizeof(idBuf), "DC-%s-%d", userId.c_str(), percentage);
    out = Discount(idBuf, userId, percentage, bookings,
                   DateTime::today(), DiscountStatus::AVAILABLE);
    return customer_repository_.saveDiscount(out, error);
}