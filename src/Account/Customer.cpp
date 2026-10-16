#include "../../include/Account/Renter.h"
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
