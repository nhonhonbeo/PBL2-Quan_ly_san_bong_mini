#pragma once
#include <string>
class Invoice; class Payment;
class PaymentService {
public:
    bool validatePayment(const Invoice& invoice, const Payment& payment, std::string& error) const;
};
