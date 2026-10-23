#include "Payment/Invoice.h"
Invoice::Invoice() : total_vnd_(0), status_(InvoiceStatus::UNPAID) {}
Invoice::Invoice(const std::string& id,const std::string& booking,long long total,const std::string& created,InvoiceStatus status):invoice_id_(id),booking_id_(booking),created_at_(created),total_vnd_(total),status_(status){}
const std::string& Invoice::getInvoiceId()const{return invoice_id_;} const std::string& Invoice::getBookingId()const{return booking_id_;} long long Invoice::getTotalVnd()const{return total_vnd_;} InvoiceStatus Invoice::getStatus()const{return status_;} void Invoice::markPaid(){status_=InvoiceStatus::PAID;} void Invoice::voidInvoice(){status_=InvoiceStatus::VOID;}
