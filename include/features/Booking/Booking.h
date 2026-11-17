#pragma once

#include <string>
#include "Core/LinkedList.h"
#include "Booking/BookingDetail.h"

struct VenueBookingRecord;

enum class BookingStatus { PENDING, CONFIRMED, COMPLETED, CANCELLED };

// LÝ DO CHỌN: LinkedList quản lý danh sách chi tiết các ca (BookingDetail) trong cùng đơn đặt;
// Booking là Aggregate Root đóng gói trọn vẹn nghiệp vụ chuyển đổi trạng thái và liên kết Persistence Record.
class Booking {
    std::string booking_id_, customer_id_, field_id_, owner_id_, date_;
    BookingStatus status_;
    LinkedList<BookingDetail> details_;
    long long total_vnd_;

public:
    Booking();
    Booking(const std::string& bookingId, const std::string& customerId,
            const std::string& fieldId, const std::string& date,
            BookingStatus status = BookingStatus::PENDING,
            const std::string& ownerId = "");
    Booking(const Booking& other);
    Booking& operator=(const Booking& other);

    const std::string& getBookingId() const;
    const std::string& getCustomerId() const;
    const std::string& getFieldId() const;
    const std::string& getDate() const;
    const std::string& getOwnerId() const;
    BookingStatus getStatus() const;
    long long getTotalVnd() const;
    const LinkedList<BookingDetail>& getDetails() const;

    void addDetail(const BookingDetail& detail);
    void setStatus(BookingStatus status);

    // Nghiệp vụ tự thân của Đơn đặt sân (Rich Domain Model)
    bool canBeCancelledBy(const std::string& customerId, std::string& error) const;
    bool cancel(const std::string& customerId, std::string& error);
    bool canBeCompletedBy(const std::string& ownerId, std::string& error) const;
    bool complete(const std::string& ownerId, std::string& error);
    bool isActive() const;
    bool isCompleted() const;
    bool isCancelled() const;
    long long calculateTotalVnd() const;

    // Chuyển đổi hai chiều Domain Entity <-> Persistence Record
    static std::string statusToText(BookingStatus status);
    static BookingStatus textToStatus(const std::string& status);
    static Booking fromRecord(const VenueBookingRecord& record);
    VenueBookingRecord toRecord() const;
};
