#pragma once

#include <string>
#include "Payment/Invoice.h"
#include "Payment/Payment.h"
#include "Service/ServiceResult.h"

class PaymentRepository;

// LÝ DO CHỌN: PaymentService xử lý vòng đời thanh toán: lập hóa đơn khi đặt sân,
// thu tiền khi hoàn tất đơn, và hủy hóa đơn khi đơn bị hủy.
class PaymentService {
private:
    PaymentRepository& repository_;

public:
    explicit PaymentService(PaymentRepository& repository);
    virtual ~PaymentService() = default;

    ServiceResultT<std::string> createInvoiceResult(
        const std::string& bookingId, long long amountVnd);
    virtual bool createInvoice(const std::string& bookingId, long long amountVnd,
                               std::string& outInvoiceId, std::string& error);
    ServiceResultT<std::string> processPaymentResult(
        const std::string& invoiceId, long long amountVnd, PaymentMethod method);
    virtual bool processPayment(const std::string& invoiceId, long long amountVnd,
                                PaymentMethod method, std::string& outPaymentId,
                                std::string& error);
    ServiceResult cancelInvoiceResult(const std::string& bookingId);
    virtual bool cancelInvoice(const std::string& bookingId, std::string& error);
    ServiceResult rollbackInvoiceResult(const std::string& bookingId);
    virtual bool rollbackInvoice(const std::string& bookingId, std::string& error);
    ServiceResult restoreInvoiceResult(const Invoice& invoice);
    ServiceResultT<Invoice> getInvoiceByBookingIdResult(
        const std::string& bookingId) const;
    virtual bool getInvoiceByBookingId(const std::string& bookingId, Invoice& out,
                                       std::string& error) const;
    ServiceResult validatePaymentResult(const Invoice& invoice,
                                        const Payment& payment) const;
    virtual bool validatePayment(const Invoice& invoice, const Payment& payment,
                                 std::string& error) const;
};