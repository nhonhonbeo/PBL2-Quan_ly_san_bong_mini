#include "Service/CustomerService.h"

#include <algorithm>
#include <cstdio>

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
    if (!isBlacklisted(userIdOrPhone, blocked, blockReason, error)) {
        return 0;
    }
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
    error.clear();

    Blacklist bl("", "", "", false);
    std::string repoErr;
    bool found = customer_repository_.findActiveBlacklist(userId, bl, repoErr);

    // Truyền lỗi từ Repository lên tầng Service nếu truy vấn gặp lỗi
    if (!repoErr.empty()) {
        error = repoErr;
        return false;
    }

    // Truy vấn thành công: kiểm tra xem khách hàng có bị khóa hiệu lực hay không
    if (found && bl.isActive()) {
        outIsBlacklisted = true;
        outReason = bl.getReason();
    } else {
        outIsBlacklisted = false;
    }
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
    if (customerIdOrPhone.empty()) {
        error = u8"Mã khách hàng hoặc số điện thoại không được để trống.";
        return false;
    }
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
    if (customerIdOrPhone.empty()) {
        error = u8"Mã khách hàng hoặc số điện thoại không được để trống.";
        return false;
    }
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
            if (!isBlacklisted(node->data.get_phone_number(), blocked, reason, error)) {
                return false;
            }
            if (!blocked && !node->data.getUserId().empty()) {
                if (!isBlacklisted(node->data.getUserId(), blocked, reason, error)) {
                    return false;
                }
            }
            if (!blocked) {
                outCandidates.push_back(node->data);
            }
        }
    }
    return true;
}

// LÝ DO CHỌN: Chống đánh giá trùng lặp theo bookingId hoặc theo (customerId + fieldId)
// đảm bảo khách hàng chỉ gửi đánh giá sau khi đã hoàn tất ca đặt sân.
bool CustomerService::addReview(const Review& review, std::string& error) {
    if (review.getRating() < 1 || review.getRating() > 5) {
        error = u8"Đánh giá phải từ 1 đến 5 sao.";
        return false;
    }
    if (review.getComment().empty()) {
        error = u8"Nội dung đánh giá không được để trống.";
        return false;
    }
    if (!customer_repository_.hasCompletedBooking(review.getCustomerId(), review.getFieldId())) {
        error = u8"Chỉ có thể đánh giá sân sau khi đã hoàn tất ca thuê tại sân này.";
        return false;
    }

    LinkedList<Review> existingReviews;
    if (customer_repository_.loadAllReviews(existingReviews, error)) {
        for (auto* n = existingReviews.get_head(); n != nullptr; n = n->next) {
            if (!review.getBookingId().empty() && n->data.getBookingId() == review.getBookingId()) {
                error = u8"Đơn đặt sân này đã được đánh giá trước đó.";
                return false;
            }
        }
    }

    Review toSave = review;
    if (toSave.getReviewId().empty()) {
        int maxSeq = 0;
        for (auto* n = existingReviews.get_head(); n != nullptr; n = n->next) {
            const std::string& id = n->data.getReviewId();
            if (id.rfind("RV", 0) == 0 && id.length() > 2) {
                try {
                    int seq = std::stoi(id.substr(2));
                    if (seq > maxSeq) maxSeq = seq;
                } catch (...) {}
            }
        }
        char buf[32];
        std::snprintf(buf, sizeof(buf), "RV%02d", maxSeq + 1);
        toSave = Review(buf, review.getCustomerId(), review.getFieldId(),
                        review.getBookingId(), review.getRating(),
                        review.getCreatedAt().empty() ? DateTime::today() : review.getCreatedAt(),
                        review.getComment(), review.getCustomerName());
    }

    return customer_repository_.saveReview(toSave, error);
}

bool CustomerService::addOrUpdateReview(const Review& review, std::string& error) {
    if (review.getRating() < 1 || review.getRating() > 5) {
        error = u8"Đánh giá phải từ 1 đến 5 sao.";
        return false;
    }
    if (review.getComment().empty()) {
        error = u8"Nội dung đánh giá không được để trống.";
        return false;
    }
    Review toSave = review;
    LinkedList<Review> existing;
    if (!customer_repository_.loadAllReviews(existing, error)) {
        return false;
    }
    if (toSave.getReviewId().empty()) {
        int maxSeq = 0;
        for (auto* n = existing.get_head(); n != nullptr; n = n->next) {
            const std::string& id = n->data.getReviewId();
            if (id.rfind("RV", 0) == 0 && id.length() > 2) {
                try {
                    int seq = std::stoi(id.substr(2));
                    if (seq > maxSeq) maxSeq = seq;
                } catch (...) {}
            }
        }
        char buf[32];
        std::snprintf(buf, sizeof(buf), "RV%02d", maxSeq + 1);
        toSave = Review(buf, review.getCustomerId(), review.getFieldId(),
                        review.getBookingId(), review.getRating(),
                        review.getCreatedAt().empty() ? DateTime::today() : review.getCreatedAt(),
                        review.getComment(), review.getCustomerName());
    }
    return customer_repository_.saveReview(toSave, error);
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
    return recordSuccessfulBooking(userId, "", error);
}

bool CustomerService::recordSuccessfulBooking(const std::string& userId,
                                              const std::string& bookingId,
                                              std::string& error) {
    if (!bookingId.empty() && customer_repository_.isBookingRecorded(bookingId)) {
        // Đơn này đã được thống kê trước đó, không ghi nhận lặp
        return true;
    }
    Customer customer("", "", "", "", "");
    if (!customer_repository_.findCustomerByUserId(userId, customer, error)) {
        return false;
    }
    customer.recordBooking();
    if (!customer_repository_.updateCustomer(customer, error)) {
        return false;
    }
    if (!bookingId.empty()) {
        customer_repository_.recordBookingId(bookingId, userId, "BOOKING_COMPLETED", error);
    }
    return true;
}

bool CustomerService::recordCancellation(const std::string& userId,
                                         std::string& error) {
    return recordCancellation(userId, "", error);
}

bool CustomerService::recordCancellation(const std::string& userId,
                                         const std::string& bookingId,
                                         std::string& error) {
    if (!bookingId.empty() && customer_repository_.isBookingRecorded(bookingId)) {
        return true;
    }
    Customer customer("", "", "", "", "");
    if (!customer_repository_.findCustomerByUserId(userId, customer, error)) {
        return false;
    }
    customer.recordCancellation();
    if (!customer_repository_.updateCustomer(customer, error)) {
        return false;
    }
    if (!bookingId.empty()) {
        customer_repository_.recordBookingId(bookingId, userId, "BOOKING_CANCELLED", error);
    }
    return true;
}

bool CustomerService::validateDiscount(const Discount& discount, const std::string& customerId,
                                       std::string& error) const {
    if (!discount.isAvailable()) {
        error = u8"Mã ưu đãi đã được sử dụng hoặc đã hết hạn.";
        return false;
    }
    if (!discount.getCustomerId().empty() && discount.getCustomerId() != customerId) {
        Customer c("", "", "", "", "");
        std::string ignErr;
        if (customer_repository_.findCustomerByUserId(customerId, c, ignErr)) {
            if (discount.getCustomerId() != c.get_phone_number() && discount.getCustomerId() != c.getUserId()) {
                error = u8"Mã ưu đãi này không thuộc về tài khoản của bạn.";
                return false;
            }
        } else {
            error = u8"Mã ưu đãi này không thuộc về tài khoản của bạn.";
            return false;
        }
    }
    bool blocked = false;
    std::string blockReason;
    if (!isBlacklisted(customerId, blocked, blockReason, error)) {
        return false;
    }
    if (blocked) {
        error = u8"Tài khoản đang bị khóa, không được áp dụng ưu đãi.";
        return false;
    }
    return true;
}

bool CustomerService::useDiscount(const std::string& discountId, const std::string& customerId,
                                  const std::string& bookingId, std::string& error) {
    LinkedList<Discount> discounts;
    if (!customer_repository_.loadAllDiscounts(discounts, error)) return false;
    for (auto* n = discounts.get_head(); n != nullptr; n = n->next) {
        if (n->data.getDiscountId() == discountId) {
            if (!validateDiscount(n->data, customerId, error)) {
                return false;
            }
            n->data.markUsed(bookingId);
            return customer_repository_.saveDiscount(n->data, error);
        }
    }
    error = u8"Không tìm thấy mã ưu đãi: " + discountId;
    return false;
}

// LÝ DO CHỌN: Kiểm tra cả danh sách mã giảm giá đã cấp; không tái cấp mã AVAILABLE nếu khách đã có
// mã khả dụng cùng loại, và không cho sử dụng mã đã đánh dấu USED hoặc EXPIRED.
bool CustomerService::findAvailableDiscount(const std::string& userId,
                                            Discount& out,
                                            std::string& error) const {
    bool blocked = false;
    std::string blockReason;
    if (!isBlacklisted(userId, blocked, blockReason, error)) {
        return false;
    }
    if (blocked) {
        error = u8"Khách hàng bị chặn không được hưởng ưu đãi.";
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
        error = u8"Chưa đạt số lần đặt tối thiểu để nhận ưu đãi.";
        return false;
    }

    LinkedList<Discount> userDiscounts;
    if (customer_repository_.findDiscountsByCustomerId(userId, userDiscounts, error)) {
        for (auto* n = userDiscounts.get_head(); n != nullptr; n = n->next) {
            if (n->data.isAvailable() && n->data.getPercentage() == percentage) {
                out = n->data;
                return true;
            }
        }
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

