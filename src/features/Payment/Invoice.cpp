#include "Payment/Invoice.h"
#include "Payment/Payment.h"

Invoice::Invoice() : total_vnd_(0), status_(InvoiceStatus::UNPAID) {}

Invoice::Invoice(const std::string& id, const std::string& booking,
                 long long total, const std::string& created,
                 InvoiceStatus status)
    : invoice_id_(id), booking_id_(booking), total_vnd_(total),
      created_at_(created), status_(status) {}

const std::string& Invoice::getInvoiceId() const { return invoice_id_; }
const std::string& Invoice::getBookingId() const { return booking_id_; }
long long Invoice::getTotalVnd() const { return total_vnd_; }
const std::string& Invoice::getCreatedAt() const { return created_at_; }
InvoiceStatus Invoice::getStatus() const { return status_; }

void Invoice::markPaid() { status_ = InvoiceStatus::PAID; }
void Invoice::voidInvoice() { status_ = InvoiceStatus::VOID; }

bool Invoice::isPaid() const { return status_ == InvoiceStatus::PAID; }
bool Invoice::isUnpaid() const { return status_ == InvoiceStatus::UNPAID; }
bool Invoice::isVoid() const { return status_ == InvoiceStatus::VOID; }

bool Invoice::canAcceptPayment() const {
    return isUnpaid() && total_vnd_ > 0;
}

bool Invoice::matchesPayment(const Payment& payment) const {
    return invoice_id_ == payment.getInvoiceId() && total_vnd_ == payment.getAmountVnd();
}
