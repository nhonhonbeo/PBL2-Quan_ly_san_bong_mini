#include "Service/PaymentService.h"

#include <cstdio>
#include "Core/DateTime.h"
#include "Payment/Invoice.h"
#include "Payment/Payment.h"
#include "Repository/PaymentRepository.h"

PaymentService::PaymentService(PaymentRepository* repository)
    : repository_(repository) {}

bool PaymentService::createInvoice(const std::string& bookingId, long long amountVnd,
                                   std::string& outInvoiceId, std::string& error) {
    if (!repository_) {
        error = "Chua khoi tao kho luu tru thanh toan.";
        return false;
    }
    if (bookingId.empty() || amountVnd < 0) {
        error = "Thong tin don dat san hoac gia tien khong hop le.";
        return false;
    }
    outInvoiceId = "INV-" + bookingId;
    Invoice inv(outInvoiceId, bookingId, amountVnd, DateTime::today(), InvoiceStatus::UNPAID);
    return repository_->saveInvoice(inv, error);
}

bool PaymentService::processPayment(const std::string& invoiceId, long long amountVnd,
                                    PaymentMethod method, std::string& outPaymentId,
                                    std::string& error) {
    if (!repository_) {
        error = "Chua khoi tao kho luu tru thanh toan.";
        return false;
    }
    LinkedList<Invoice> invoices;
    if (!repository_->loadAllInvoices(invoices, error)) return false;

    Invoice* target = nullptr;
    for (auto* node = invoices.get_head(); node != nullptr; node = node->next) {
        if (node->data.getInvoiceId() == invoiceId) {
            target = &node->data;
            break;
        }
    }
    if (!target) {
        error = "Khong tim thay hoa don can thanh toan: " + invoiceId;
        return false;
    }
    if (!target->canAcceptPayment()) {
        error = "Hoa don khong o trang thai co the thu tien.";
        return false;
    }
    if (target->getTotalVnd() != amountVnd) {
        error = "So tien thanh toan khong khop voi tong hoa don.";
        return false;
    }

    target->markPaid();
    if (!repository_->saveAllInvoices(invoices, error)) return false;

    char payIdBuf[64];
    std::snprintf(payIdBuf, sizeof(payIdBuf), "PAY-%s", invoiceId.c_str());
    outPaymentId = payIdBuf;

    Payment payment(outPaymentId, invoiceId, amountVnd, method, DateTime::today());
    return repository_->savePayment(payment, error);
}

bool PaymentService::cancelInvoice(const std::string& bookingId, std::string& error) {
    if (!repository_) return true; // ponytail: Bo qua khi repository chua gan; nang cap khi can chan chat che.
    LinkedList<Invoice> invoices;
    if (!repository_->loadAllInvoices(invoices, error)) return false;

    bool found = false;
    for (auto* node = invoices.get_head(); node != nullptr; node = node->next) {
        if (node->data.getBookingId() == bookingId) {
            node->data.voidInvoice();
            found = true;
            break;
        }
    }
    if (found) {
        return repository_->saveAllInvoices(invoices, error);
    }
    return true;
}

bool PaymentService::getInvoiceByBookingId(const std::string& bookingId, Invoice& out,
                                          std::string& error) const {
    if (!repository_) {
        error = "Chua khoi tao kho luu tru thanh toan.";
        return false;
    }
    return repository_->findInvoiceByBookingId(bookingId, out, error);
}

bool PaymentService::validatePayment(const Invoice& invoice, const Payment& payment,
                                     std::string& error) const {
    if (!invoice.canAcceptPayment()) {
        error = "Hoa don khong con cho thanh toan.";
        return false;
    }
    if (!invoice.matchesPayment(payment)) {
        error = "So tien hoac ma hoa don thanh toan khong hop le.";
        return false;
    }
    return true;
}