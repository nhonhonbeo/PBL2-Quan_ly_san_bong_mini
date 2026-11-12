#pragma once
#include <string>

enum class PaymentMethod { CASH = 0, TRANSFER = 1, OTHER = 2 };

class Invoice;

// LÝ DO CHỌN: Payment là bản ghi giao dịch thu tiền gắn liền với Invoice.
class Payment {
private:
    std::string payment_id_;
    std::string invoice_id_;
    long long amount_vnd_;
    PaymentMethod method_;
    std::string paid_at_;

public:
    Payment();
    Payment(const std::string& paymentId, const std::string& invoiceId,
            long long amountVnd, PaymentMethod method,
            const std::string& paidAt);

    const std::string& getPaymentId() const;
    const std::string& getInvoiceId() const;
    long long getAmountVnd() const;
    PaymentMethod getMethod() const;
    const std::string& getPaidAt() const;

    // Nghiệp vụ tự thân của Thanh toán (Rich Domain Model)
    bool isValid() const;
    std::string getMethodName() const;
    bool matchesInvoice(const Invoice& invoice) const;
};
