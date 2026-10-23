#pragma once
#include <string>
enum class InvoiceStatus { UNPAID, PAID, VOID };
// LÝ DO CHỌN: Invoice tách tổng tiền phải thu khỏi Booking để theo dõi thanh toán độc lập.
class Invoice {
    std::string invoice_id_, booking_id_, created_at_;
    long long total_vnd_;
    InvoiceStatus status_;
public:
    Invoice();
    Invoice(const std::string&, const std::string&, long long, const std::string&, InvoiceStatus = InvoiceStatus::UNPAID);
    const std::string& getInvoiceId() const; const std::string& getBookingId() const;
    long long getTotalVnd() const; InvoiceStatus getStatus() const; void markPaid(); void voidInvoice();
};
