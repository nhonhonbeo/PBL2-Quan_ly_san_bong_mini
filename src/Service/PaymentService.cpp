#include "Service/PaymentService.h"
#include "Payment/Invoice.h"
#include "Payment/Payment.h"
bool PaymentService::validatePayment(const Invoice& invoice,const Payment& payment,std::string& error)const{if(invoice.getStatus()!=InvoiceStatus::UNPAID){error="Hoa don khong con cho thanh toan";return false;}if(invoice.getInvoiceId()!=payment.getInvoiceId()||invoice.getTotalVnd()!=payment.getAmountVnd()){error="So tien thanh toan khong hop le";return false;}return true;}
