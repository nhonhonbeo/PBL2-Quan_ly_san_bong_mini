#include "Service/CustomerService.h"

#include <algorithm>

#include "Account/Customer.h"
#include "Core/DateTime.h"
#include "Customer/Blacklist.h"
#include "Customer/Discount.h"
#include "Customer/Review.h"
#include "Repository/CustomerRepository.h"

CustomerService::CustomerService(CustomerRepository& customerRepository)
    : customer_repository_(customerRepository),
      discount_thresholds_{
          {1, 5, 5, u8"Khách thân thiết"},
          {2, 10, 10, u8"Khách hàng VIP Bạc"},
          {3, 20, 15, u8"Khách hàng VIP Vàng"}
      } {}

const std::vector<CustomerDiscountThreshold>& CustomerService::getDiscountThresholds() const {
    return discount_thresholds_;
}

bool CustomerService::updateDiscountThreshold(int id, int minBookings,
                                             int percentage,
                                             std::string& error) {
    for (auto& t : discount_thresholds_) {
        if (t.id == id) {
            t.minBookings = minBookings;
            t.percentage = percentage;
            return true;
        }
    }
    error = "Khong tim thay nguong uu dai can sua.";
    return false;
}

int CustomerService::getCustomerDiscountPercentage(const std::string& userIdOrPhone,
                                                  std::string& error) const {
    bool blocked = false;
    std::string blockReason;
    isBlacklisted(userIdOrPhone, blocked, blockReason);
    if (blocked) {
        return 0;
    }

    Customer c("", "", "", "", "");
    if (!customer_repository_.findCustomerByUserId(userIdOrPhone, c, error)) {
        return 0;
    }

    auto sorted = discount_thresholds_;
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
        return a.minBookings > b.minBookings;
    });

    for (const auto& t : sorted) {
        if (c.getBookingCount() >= t.minBookings) {
            return t.percentage;
        }
    }
    return 0;
}

bool CustomerService::applyDiscountsToAll(std::string& error) {
    LinkedList<Customer> list;
    if (!customer_repository_.loadAllCustomers(list, error)) {
        return false;
    }
    return true;
}

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

bool CustomerService::addToBlacklist(const std::string& customerIdOrPhone,
                                    const std::string& reason,
                                    std::string& error) {
    Blacklist bl(
        customerIdOrPhone,
        reason.empty() ? u8"Hủy sân nhiều lần không thông báo trước" : reason,
        DateTime::today(),
        true
    );
    return customer_repository_.saveBlacklist(bl, error);
}

bool CustomerService::removeFromBlacklist(const std::string& customerIdOrPhone,
                                         std::string& error) {
    return customer_repository_.removeBlacklist(customerIdOrPhone, error);
}

bool CustomerService::getAllBlacklists(LinkedList<Blacklist>& outList,
                                      std::string& error) const {
    return customer_repository_.loadAllBlacklists(outList, error);
}

bool CustomerService::getBlacklistCandidates(int minCancellations,
                                             LinkedList<Customer>& outCandidates,
                                             std::string& error) const {
    outCandidates.clear();
    LinkedList<Customer> list;
    if (!customer_repository_.loadAllCustomers(list, error)) {
        return false;
    }

    for (auto* node = list.get_head(); node != nullptr; node = node->next) {
        if (node->data.getCancelCount() >= minCancellations) {
            bool blocked = false;
            std::string reason;
            isBlacklisted(node->data.get_phone_number(), blocked, reason);
            if (!blocked && !node->data.getUserId().empty()) {
                isBlacklisted(node->data.getUserId(), blocked, reason);
            }
            if (!blocked) {
                outCandidates.push_back(node->data);
            }
        }
    }
    return true;
}

bool CustomerService::addReview(const Review& review, std::string& error) {
    if (review.getRating() < 1 || review.getRating() > 5) {
        error = "??nh gi? ph?i t? 1 ??n 5 sao.";
        return false;
    }
    if (review.getComment().empty()) {
        error = "N?i dung ??nh gi? kh?ng ???c ?? tr?ng.";
        return false;
    }
    if (!customer_repository_.hasCompletedBooking(review.getCustomerId(), review.getFieldId())) {
        error = "Ch? c? th? ??nh gi? s?n sau khi ?? ho?n t?t ca thu? t?i s?n n?y.";
        return false;
    }
    return customer_repository_.saveReview(review, error);
}

bool CustomerService::addOrUpdateReview(const Review& review, std::string& error) {
    if (review.getRating() < 1 || review.getRating() > 5) {
        error = "??nh gi? ph?i t? 1 ??n 5 sao.";
        return false;
    }
    if (review.getComment().empty()) {
        error = "N?i dung ??nh gi? kh?ng ???c ?? tr?ng.";
        return false;
    }
    return customer_repository_.saveReview(review, error);
}

bool CustomerService::getAllReviews(LinkedList<Review>& outReviews,
                                   std::string& error) const {
    return customer_repository_.loadAllReviews(outReviews, error);
}

bool CustomerService::getAverageRatingOverall(float& outScore,
                                             std::string& error) const {
    LinkedList<Review> list;
    if (!customer_repository_.loadAllReviews(list, error)) {
        return false;
    }
    if (list.size() == 0) {
        outScore = 0.0f;
        return true;
    }
    float sum = 0.0f;
    for (auto* node = list.get_head(); node != nullptr; node = node->next) {
        sum += node->data.getRating();
    }
    outScore = sum / static_cast<float>(list.size());
    return true;
}

bool CustomerService::getAverageRatingsMap(std::map<std::string, float>& outMap,
                                          std::string& error) const {
    outMap.clear();
    LinkedList<Review> list;
    if (!customer_repository_.loadAllReviews(list, error)) {
        return false;
    }
    std::map<std::string, std::pair<float, int>> counts;
    for (auto* node = list.get_head(); node != nullptr; node = node->next) {
        counts[node->data.getFieldId()].first += node->data.getRating();
        counts[node->data.getFieldId()].second++;
    }
    for (const auto& item : counts) {
        if (item.second.second > 0) {
            outMap[item.first] = item.second.first / static_cast<float>(item.second.second);
        } else {
            outMap[item.first] = 0.0f;
        }
    }
    return true;
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

bool CustomerService::findAvailableDiscount(const std::string& userId,
                                            Discount& out,
                                            std::string& error) const {
    bool blocked = false;
    std::string blockReason;
    isBlacklisted(userId, blocked, blockReason);
    if (blocked) {
        error = "Kh?ch h?ng b? ch?n kh?ng ???c h??ng ?u ??i.";
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
        error = "Ch?a ??t s? l?n ??t t?i thi?u ?? nh?n ?u ??i.";
        return false;
    }

    char idBuf[64];
    std::snprintf(idBuf, sizeof(idBuf), "DC-%s-%d", userId.c_str(), percentage);
    out = Discount(idBuf, userId, percentage, bookings,
                   DateTime::today(), DiscountStatus::AVAILABLE);
    return customer_repository_.saveDiscount(out, error);
}

bool CustomerService::getAllCustomers(LinkedList<Customer>& outList,
                                      std::string& error) const {
    return customer_repository_.loadAllCustomers(outList, error);
}

bool CustomerService::findCustomer(const std::string& idOrPhone,
                                   Customer& out,
                                   std::string& error) const {
    return customer_repository_.findCustomerByUserId(idOrPhone, out, error);
}
