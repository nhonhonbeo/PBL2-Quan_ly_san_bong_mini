#pragma once

#include <string>
#include "Payment/Invoice.h"
#include "Payment/Payment.h"

class PaymentRepository;

// LÝ DO CHỌN: PaymentService xử lý vòng đời thanh toán: lập hóa đơn khi đặt sân,
// thu tiền khi hoàn tất đơn, và hủy hóa đơn khi đơn bị hủy.
class PaymentService {
private:
    PaymentRepository* repository_;

public:
    explicit PaymentService(PaymentRepository* repository = nullptr);

    bool createInvoice(const std::string& bookingId, long long amountVnd,
                       std::string& outInvoiceId, std::string& error);
    bool processPayment(const std::string& invoiceId, long long amountVnd,
                        PaymentMethod method, std::string& outPaymentId,
                        std::string& error);
    bool cancelInvoice(const std::string& bookingId, std::string& error);
    bool getInvoiceByBookingId(const std::string& bookingId, Invoice& out,
                               std::string& error) const;
    bool validatePayment(const Invoice& invoice, const Payment& payment,
                         std::string& error) const;
};