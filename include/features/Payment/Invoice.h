#pragma once
#include <string>

enum class InvoiceStatus { UNPAID = 0, PAID = 1, VOID = 2 };

class Payment;

// LÝ DO CHỌN: Invoice quản lý tổng tiền và trạng thái thanh toán độc lập của Booking.
class Invoice {
private:
    std::string invoice_id_;
    std::string booking_id_;
    long long total_vnd_;
    std::string created_at_;
    InvoiceStatus status_;

public:
    Invoice();
    Invoice(const std::string& invoiceId, const std::string& bookingId,
            long long totalVnd, const std::string& createdAt,
            InvoiceStatus status = InvoiceStatus::UNPAID);

    const std::string& getInvoiceId() const;
    const std::string& getBookingId() const;
    long long getTotalVnd() const;
    const std::string& getCreatedAt() const;
    InvoiceStatus getStatus() const;

    void markPaid();
    void voidInvoice();

    // Nghiệp vụ tự thân của Hóa đơn (Rich Domain Model)
    bool isPaid() const;
    bool isUnpaid() const;
    bool isVoid() const;
    bool canAcceptPayment() const;
    bool matchesPayment(const Payment& payment) const;
};
