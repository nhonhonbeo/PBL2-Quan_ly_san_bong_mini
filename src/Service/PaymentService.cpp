#include "Service/PaymentService.h"

#include <cstdio>
#include "Core/DateTime.h"
#include "Payment/Invoice.h"
#include "Payment/Payment.h"
#include "Repository/PaymentRepository.h"

PaymentService::PaymentService(PaymentRepository& repository)
    : repository_(repository) {}

ServiceResultT<std::string> PaymentService::createInvoiceResult(
    const std::string& bookingId, long long amountVnd) {
    if (bookingId.empty() || amountVnd <= 0) {
        return ServiceResultT<std::string>::failure(
            ServiceErrorCode::InvalidData,
            "Thông tin đơn đặt sân hoặc giá tiền không hợp lệ."
        );
    }
    const std::string invoiceId = "INV-" + bookingId;
    Invoice invoice(invoiceId, bookingId, amountVnd, DateTime::today(),
                    InvoiceStatus::UNPAID);
    std::string error;
    if (!repository_.saveInvoice(invoice, error)) {
        return ServiceResultT<std::string>::failure(
            ServiceErrorCode::StorageFailure, std::move(error)
        );
    }
    return ServiceResultT<std::string>::success(invoiceId);
}

bool PaymentService::createInvoice(const std::string& bookingId, long long amountVnd,
                                   std::string& outInvoiceId, std::string& error) {
    auto result = createInvoiceResult(bookingId, amountVnd);
    if (!result) {
        error = result.message;
        return false;
    }
    outInvoiceId = std::move(*result.value);
    error.clear();
    return true;
}

ServiceResultT<std::string> PaymentService::processPaymentResult(
    const std::string& invoiceId, long long amountVnd, PaymentMethod method) {
    LinkedList<Invoice> invoices;
    std::string error;
    if (!repository_.loadAllInvoices(invoices, error)) {
        return ServiceResultT<std::string>::failure(
            ServiceErrorCode::StorageFailure, std::move(error)
        );
    }

    Invoice* target = invoices.findIf(
        [&invoiceId](const Invoice& invoice) {
            return invoice.getInvoiceId() == invoiceId;
        });
    if (!target) {
        return ServiceResultT<std::string>::failure(
            ServiceErrorCode::NotFound,
            "Không tìm thấy hóa đơn cần thanh toán: " + invoiceId
        );
    }
    if (!target->canAcceptPayment()) {
        return ServiceResultT<std::string>::failure(
            ServiceErrorCode::BusinessRuleViolation,
            "Hóa đơn không ở trạng thái có thể thu tiền."
        );
    }
    if (target->getTotalVnd() != amountVnd) {
        return ServiceResultT<std::string>::failure(
            ServiceErrorCode::InvalidData,
            "Số tiền thanh toán không khớp với tổng hóa đơn."
        );
    }

    const LinkedList<Invoice> originalInvoices = invoices;
    target->markPaid();
    if (!repository_.saveAllInvoices(invoices, error)) {
        return ServiceResultT<std::string>::failure(
            ServiceErrorCode::StorageFailure, std::move(error)
        );
    }

    char payIdBuf[64];
    std::snprintf(payIdBuf, sizeof(payIdBuf), "PAY-%s", invoiceId.c_str());
    const std::string paymentId = payIdBuf;
    Payment payment(paymentId, invoiceId, amountVnd, method, DateTime::today());
    if (!repository_.savePayment(payment, error)) {
        std::string rollbackError;
        if (!repository_.saveAllInvoices(originalInvoices, rollbackError)) {
            error += "; không thể khôi phục trạng thái hóa đơn: " + rollbackError;
        }
        return ServiceResultT<std::string>::failure(
            ServiceErrorCode::StorageFailure, std::move(error)
        );
    }
    return ServiceResultT<std::string>::success(paymentId);
}

bool PaymentService::processPayment(const std::string& invoiceId, long long amountVnd,
                                    PaymentMethod method, std::string& outPaymentId,
                                    std::string& error) {
    auto result = processPaymentResult(invoiceId, amountVnd, method);
    if (!result) {
        error = result.message;
        return false;
    }
    outPaymentId = std::move(*result.value);
    error.clear();
    return true;
}

ServiceResult PaymentService::cancelInvoiceResult(const std::string& bookingId) {
    LinkedList<Invoice> invoices;
    std::string error;
    if (!repository_.loadAllInvoices(invoices, error)) {
        return ServiceResult::failure(
            ServiceErrorCode::StorageFailure, std::move(error)
        );
    }

    Invoice* invoice = invoices.findIf(
        [&bookingId](const Invoice& current) {
            return current.getBookingId() == bookingId;
        });
    if (invoice != nullptr) {
        invoice->voidInvoice();
        if (!repository_.saveAllInvoices(invoices, error)) {
            return ServiceResult::failure(
                ServiceErrorCode::StorageFailure, std::move(error)
            );
        }
    }
    return ServiceResult::success();
}

bool PaymentService::cancelInvoice(const std::string& bookingId,
                                   std::string& error) {
    ServiceResult result = cancelInvoiceResult(bookingId);
    error = result.message;
    return result.isSuccess();
}

ServiceResult PaymentService::rollbackInvoiceResult(const std::string& bookingId) {
    std::string error;
    if (!repository_.removeInvoice(bookingId, error)) {
        return ServiceResult::failure(
            ServiceErrorCode::StorageFailure, std::move(error)
        );
    }
    return ServiceResult::success();
}

bool PaymentService::rollbackInvoice(const std::string& bookingId,
                                     std::string& error) {
    ServiceResult result = rollbackInvoiceResult(bookingId);
    error = result.message;
    return result.isSuccess();
}

ServiceResult PaymentService::restoreInvoiceResult(const Invoice& invoice) {
    std::string error;
    if (!repository_.saveInvoice(invoice, error)) {
        return ServiceResult::failure(
            ServiceErrorCode::StorageFailure, std::move(error)
        );
    }
    return ServiceResult::success();
}

ServiceResultT<Invoice> PaymentService::getInvoiceByBookingIdResult(
    const std::string& bookingId) const {
    std::string error;
    LinkedList<Invoice> invoices;
    if (!repository_.loadAllInvoices(invoices, error)) {
        return ServiceResultT<Invoice>::failure(
            ServiceErrorCode::StorageFailure, std::move(error)
        );
    }
    for (auto* node = invoices.get_head(); node != nullptr; node = node->next) {
        if (node->data.getBookingId() == bookingId) {
            return ServiceResultT<Invoice>::success(node->data);
        }
    }
    return ServiceResultT<Invoice>::failure(
        ServiceErrorCode::NotFound,
        "Không tìm thấy hóa đơn cho đơn đặt sân: " + bookingId
    );
}

bool PaymentService::getInvoiceByBookingId(const std::string& bookingId,
                                          Invoice& out,
                                          std::string& error) const {
    auto result = getInvoiceByBookingIdResult(bookingId);
    if (!result) {
        error = result.message;
        return false;
    }
    out = std::move(*result.value);
    error.clear();
    return true;
}

ServiceResult PaymentService::validatePaymentResult(
    const Invoice& invoice, const Payment& payment) const {
    if (!invoice.canAcceptPayment()) {
        return ServiceResult::failure(
            ServiceErrorCode::BusinessRuleViolation,
            "Hóa đơn không còn chờ thanh toán."
        );
    }
    if (!invoice.matchesPayment(payment)) {
        return ServiceResult::failure(
            ServiceErrorCode::InvalidData,
            "Số tiền hoặc mã hóa đơn thanh toán không hợp lệ."
        );
    }
    return ServiceResult::success();
}

bool PaymentService::validatePayment(const Invoice& invoice,
                                    const Payment& payment,
                                    std::string& error) const {
    ServiceResult result = validatePaymentResult(invoice, payment);
    error = result.message;
    return result.isSuccess();
}
