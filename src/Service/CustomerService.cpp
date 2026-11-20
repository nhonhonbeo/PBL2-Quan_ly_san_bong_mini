#include "Service/CustomerService.h"

#include <cstdio>

#include "Account/Customer.h"
#include "Core/DateTime.h"
#include "Core/IdGenerator.h"
#include "Customer/Review.h"
#include "Repository/CustomerRepository.h"

CustomerService::CustomerService(CustomerRepository& customerRepository)
    : customer_repository_(customerRepository) {}

// LÝ DO CHỌN: Chống đánh giá trùng lặp theo bookingId hoặc theo (customerId + fieldId)
// đảm bảo khách hàng chỉ gửi đánh giá sau khi đã hoàn tất ca đặt sân.
bool CustomerService::addReview(const Review& review, std::string& error) {
    // New reviews always receive a generated ID. This is not an edit entry point.
    return addOrUpdateReview(Review("", review.getCustomerId(), review.getFieldId(),
        review.getBookingId(), review.getRating(), review.getCreatedAt(),
        review.getComment(), review.getCustomerName()), error);
}

bool CustomerService::addOrUpdateReview(const Review& review, std::string& error) {
    if (review.getRating() < 1 || review.getRating() > 5 ||
        review.getComment().find_first_not_of(" \t\r\n") == std::string::npos) {
        error = "Invalid rating or empty comment"; return false;
    }
    Customer actor;
    if (!customer_repository_.findCustomerByUserId(review.getCustomerId(), actor, error)) return false;
    if (!customer_repository_.hasCompletedBooking(actor.getUserId(), review.getFieldId())) {
        error = u8"Chỉ có thể đánh giá sân sau khi đã hoàn tất ca thuê tại sân này."; return false;
    }
    LinkedList<Review> existing;
    if (!customer_repository_.loadAllReviews(existing, error)) return false;
    IdGenerator generator("RV", 2);
    bool editing = !review.getReviewId().empty(), found = false;
    std::string date = DateTime::today();
    for (auto* n = existing.get_head(); n; n = n->next) {
        const auto& old = n->data;
        generator.observe(old.getReviewId());
        const bool sameOwner = old.getCustomerId() == actor.getUserId() ||
                               old.getCustomerId() == actor.get_phone_number();
        if (editing && old.getReviewId() == review.getReviewId()) {
            if (!sameOwner || old.getFieldId() != review.getFieldId()) { error = "Review ownership mismatch"; return false; }
            found = true; date = old.getCreatedAt();
        } else if (!editing && sameOwner && old.getFieldId() == review.getFieldId()) {
            error = "Review already exists for this field"; return false;
        }
    }
    if (editing && !found) { error = "Review not found"; return false; }
    Review toSave(editing ? review.getReviewId() : generator.next(), actor.getUserId(),
        review.getFieldId(), "", review.getRating(), date, review.getComment(), actor.get_name());
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
    const Customer originalCustomer = customer;
    customer.recordBooking();
    if (!customer_repository_.updateCustomer(customer, error)) {
        return false;
    }
    if (!bookingId.empty() &&
        !customer_repository_.recordBookingId(
            bookingId, userId, "BOOKING_COMPLETED", error)) {
        std::string rollbackError;
        if (!customer_repository_.updateCustomer(originalCustomer,
                                                 rollbackError)) {
            error += "; không thể khôi phục thống kê khách hàng: " +
                     rollbackError;
        }
        return false;
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
    const Customer originalCustomer = customer;
    customer.recordCancellation();
    if (!customer_repository_.updateCustomer(customer, error)) {
        return false;
    }
    if (!bookingId.empty() &&
        !customer_repository_.recordBookingId(
            bookingId, userId, "BOOKING_CANCELLED", error)) {
        std::string rollbackError;
        if (!customer_repository_.updateCustomer(originalCustomer,
                                                 rollbackError)) {
            error += "; không thể khôi phục thống kê khách hàng: " +
                     rollbackError;
        }
        return false;
    }
    return true;
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

