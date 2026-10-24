#pragma once
#include <string>
enum class PaymentMethod { CASH, TRANSFER, OTHER };
// LÝ DO CHỌN: Payment là bản ghi giao dịch, giữ phương thức và thời điểm thanh toán.
class Payment {
    std::string payment_id_, invoice_id_, paid_at_;
    long long amount_vnd_;
    PaymentMethod method_;
public:
    Payment();
    Payment(const std::string&, const std::string&, long long, PaymentMethod, const std::string&);
    const std::string& getPaymentId() const; const std::string& getInvoiceId() const;
    long long getAmountVnd() const; PaymentMethod getMethod() const; const std::string& getPaidAt() const;
};
