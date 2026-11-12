#pragma once

#include <string>
#include "Core/LinkedList.h"
#include "Payment/Invoice.h"
#include "Payment/Payment.h"

class FileManager;

// LÝ DO CHỌN: PaymentRepository quản lý lưu trữ hóa đơn và giao dịch thanh toán
// vào data/invoices.txt và data/payments.txt bằng LinkedList.
class PaymentRepository {
private:
    FileManager& file_manager_;

public:
    explicit PaymentRepository(FileManager& fileManager);

    // Hóa đơn (data/invoices.txt)
    bool loadAllInvoices(LinkedList<Invoice>& out, std::string& error) const;
    bool saveAllInvoices(const LinkedList<Invoice>& invoices, std::string& error) const;
    bool saveInvoice(const Invoice& invoice, std::string& error);
    bool findInvoiceByBookingId(const std::string& bookingId, Invoice& out, std::string& error) const;

    // Giao dịch thanh toán (data/payments.txt)
    bool loadAllPayments(LinkedList<Payment>& out, std::string& error) const;
    bool saveAllPayments(const LinkedList<Payment>& payments, std::string& error) const;
    bool savePayment(const Payment& payment, std::string& error);
    bool findPaymentByInvoiceId(const std::string& invoiceId, Payment& out, std::string& error) const;
};