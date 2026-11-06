#include "Account/Customer.h"
#include <iostream>

using namespace std;

Customer::Customer(const string& user_id, const string& user_name, const string& password, const string& name, const string& phone_number)
    : User(user_id, user_name, password, name, phone_number), booking_count(0), cancel_count(0) {}

void Customer::show_menu() const {
    cout << "--- Menu Khach Hang ---\n";
    cout << "1. Dat san\n";
    cout << "2. Lich su dat san\n";
}

string Customer::get_account_type() const { 
    return "Customer"; 
}

int Customer::get_booking_count() const { 
    return booking_count; 
}

int Customer::get_cancel_count() const { 
    return cancel_count; 
}

void Customer::increase_booking_count() { 
    booking_count++; 
}

void Customer::increase_cancel_count() { 
    cancel_count++; 
}

bool Customer::reg_customer() const { 
    return booking_count >= 10; 
}

const std::string& Customer::getUserId() const { return user_id; }
int Customer::getBookingCount() const { return booking_count; }
int Customer::getCancelCount() const { return cancel_count; }
void Customer::recordBooking() { increase_booking_count(); }
void Customer::recordCancellation() { increase_cancel_count(); }

void Customer::setStatistics(int bookingCount, int cancelCount) {
    booking_count = bookingCount;
    cancel_count = cancelCount;
}

bool Customer::isBlacklistCandidate(int minCancellations) const {
    return cancel_count >= minCancellations;
}

bool Customer::qualifiesForDiscount(int minBookings) const {
    return booking_count >= minBookings;
}

bool Customer::canBook(bool isBlocked, std::string& error) const {
    if (isBlocked) {
        error = u8"Tài khoản đang bị khóa do vi phạm chính sách hủy sân.";
        return false;
    }
    return true;
}

int Customer::getReliabilityRate() const {
    int total = booking_count + cancel_count;
    if (total == 0) return 100;
    return (booking_count * 100) / total;
}
