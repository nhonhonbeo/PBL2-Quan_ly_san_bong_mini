#ifndef REVIEW_H
#define REVIEW_H

#include <string>

// LÝ DO CHỌN: Review giữ ID của khách, sân và booking để không nhân đôi danh
// sách review trong CustomerProfile hoặc Field.
class Review {
private:
    std::string review_id_;
    std::string customer_id_;
    std::string field_id_;
    std::string booking_id_;
    int rating_;
    std::string created_at_;
    std::string comment_;

public:
    Review(const std::string& reviewId, const std::string& customerId,
           const std::string& fieldId, const std::string& bookingId,
           int rating, const std::string& createdAt,
           const std::string& comment);

    const std::string& getReviewId() const;
    const std::string& getCustomerId() const;
    const std::string& getFieldId() const;
    const std::string& getBookingId() const;
    int getRating() const;
    const std::string& getCreatedAt() const;
    const std::string& getComment() const;

    // CustomerService kiểm tra quyền sửa review trước khi gọi.
    void updateComment(const std::string& comment);
};

#endif
