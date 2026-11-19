#include <cassert>
#include <iostream>
#include <string>

#include "Payment/Invoice.h"
#include "Payment/Payment.h"
#include "Repository/PaymentRepository.h"
#include "Service/PaymentService.h"
#include "Storage/FileManager.h"

class FailingPaymentRepository : public PaymentRepository {
public:
    explicit FailingPaymentRepository(FileManager& fileManager)
        : PaymentRepository(fileManager) {
        invoices_.push_back(Invoice(
            "INV-BK-TEST", "BK-TEST", 100000, "2026-11-17",
            InvoiceStatus::UNPAID
        ));
    }

    bool loadAllInvoices(LinkedList<Invoice>& out,
                         std::string&) const override {
        out = invoices_;
        return true;
    }

    bool saveAllInvoices(const LinkedList<Invoice>& invoices,
                         std::string&) const override {
        invoices_ = invoices;
        return true;
    }

    bool savePayment(const Payment&, std::string& error) override {
        error = "Simulated payment write failure.";
        return false;
    }

    InvoiceStatus invoiceStatus() const {
        return invoices_.get_head()->data.getStatus();
    }

private:
    mutable LinkedList<Invoice> invoices_;
};

int main() {
    FileManager fileManager;
    FailingPaymentRepository repository(fileManager);
    PaymentService service(repository);
    std::string paymentId;
    std::string error;

    const bool paid = service.processPayment(
        "INV-BK-TEST", 100000, PaymentMethod::TRANSFER, paymentId, error
    );

    assert(!paid);
    assert(repository.invoiceStatus() == InvoiceStatus::UNPAID);
    assert(paymentId.empty());
    assert(!error.empty());
    std::cout << "[PASS] Failed payment persistence does not leave a paid invoice.\n";
}
