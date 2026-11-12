#include "Payment/Payment.h"
#include "Payment/Invoice.h"

Payment::Payment() : amount_vnd_(0), method_(PaymentMethod::CASH) {}

Payment::Payment(const std::string& id, const std::string& invoice,
                 long long amount, PaymentMethod method,
                 const std::string& paid)
    : payment_id_(id), invoice_id_(invoice), amount_vnd_(amount),
      method_(method), paid_at_(paid) {}

const std::string& Payment::getPaymentId() const { return payment_id_; }
const std::string& Payment::getInvoiceId() const { return invoice_id_; }
long long Payment::getAmountVnd() const { return amount_vnd_; }
PaymentMethod Payment::getMethod() const { return method_; }
const std::string& Payment::getPaidAt() const { return paid_at_; }

bool Payment::isValid() const {
    return !payment_id_.empty() && !invoice_id_.empty() && amount_vnd_ > 0;
}

std::string Payment::getMethodName() const {
    switch (method_) {
        case PaymentMethod::CASH: return u8"Tiền mặt";
        case PaymentMethod::TRANSFER: return u8"Chuyển khoản";
        default: return u8"Khác";
    }
}

bool Payment::matchesInvoice(const Invoice& invoice) const {
    return invoice_id_ == invoice.getInvoiceId() && amount_vnd_ == invoice.getTotalVnd();
}
