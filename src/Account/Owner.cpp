#include "../../include/Account/Owner.h"
#include <iostream>

Owner::Owner(const std::string& user_id, const std::string& user_name, const std::string& password, const std::string& name, const std::string& phone_number,
             const std::string& field_name, const std::string& field_address, const std::string& field_rules, const std::string& open_time, const std::string& close_time, const std::string owner_name)
    : User(user_id, user_name, password, name, phone_number),
      field_name(field_name), field_address(field_address), field_rules(field_rules), open_time(open_time), close_time(close_time), owner_name(owner_name) {}

void Owner::show_menu() const {
    std::cout << "--- Menu Chu San ---\n";
    std::cout << "1. Quan ly san bong\n";
    std::cout << "2. Thong tin ca nhan\n";
}

std::string Owner::get_account_type() const { return "Owner"; }
std::string Owner::get_field_name() const { return field_name; }
void Owner::set_field_name(const std::string& field_name) { this->field_name = field_name; }
std::string Owner::get_field_address() const { return field_address; }
void Owner::set_field_address(const std::string& field_address) { this->field_address = field_address; }
std::string Owner::get_field_rules() const { return field_rules; }
void Owner::set_field_rules(const std::string& field_rules) { this->field_rules = field_rules; }
std::string Owner::get_open_time() const { return open_time; }
void Owner::set_open_time(const std::string& open_time) { this->open_time = open_time; }
std::string Owner::get_close_time() const { return close_time; }
void Owner::set_close_time(const std::string& close_time) { this->close_time = close_time; }
std::string Owner::get_owner_name() const { return owner_name; }
void Owner::set_owner_name(const std::string& owner_name) { this->owner_name = owner_name; }
