#ifndef REVIEW_H
#define REVIEW_H

#include <string>

// LÝ DO CHỌN: Review lưu trữ nhận xét và điểm số của khách hàng theo sân,
// liên kết trực tiếp với tập tin data/reviews.txt.
class Review {
private:
    std::string review_id_;
    std::string customer_name_;
    std::string customer_id_;
    std::string field_id_;
    std::string booking_id_;
    int rating_;
    std::string created_at_;
    std::string comment_;

public:
    Review(const std::string& reviewId = "",
           const std::string& customerId = "",
           const std::string& fieldId = "",
           const std::string& bookingId = "",
           int rating = 5,
           const std::string& createdAt = "",
           const std::string& comment = "",
           const std::string& customerName = "");

    const std::string& getReviewId() const;
    const std::string& getCustomerName() const;
    const std::string& getCustomerId() const;
    const std::string& getFieldId() const;
    const std::string& getBookingId() const;
    int getRating() const;
    const std::string& getCreatedAt() const;
    const std::string& getComment() const;

    void setCustomerName(const std::string& name);
    void setRating(int rating);
    void updateComment(const std::string& comment);
};

#endif
