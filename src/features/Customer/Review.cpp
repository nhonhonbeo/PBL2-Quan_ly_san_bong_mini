#include "Customer/Review.h"

Review::Review(const std::string& reviewId, const std::string& customerId,
               const std::string& fieldId, const std::string& bookingId,
               int rating, const std::string& createdAt,
               const std::string& comment)
    : review_id_(reviewId), customer_id_(customerId), field_id_(fieldId),
      booking_id_(bookingId), rating_(rating), created_at_(createdAt),
      comment_(comment) {}

const std::string& Review::getReviewId() const { return review_id_; }

const std::string& Review::getCustomerId() const { return customer_id_; }

const std::string& Review::getFieldId() const { return field_id_; }

const std::string& Review::getBookingId() const { return booking_id_; }

int Review::getRating() const { return rating_; }

const std::string& Review::getCreatedAt() const { return created_at_; }

const std::string& Review::getComment() const { return comment_; }

void Review::updateComment(const std::string& comment) { comment_ = comment; }
