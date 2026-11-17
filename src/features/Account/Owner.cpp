#include "Account/Owner.h"

#include <iostream>

Owner::Owner(const std::string& user_id, const std::string& user_name, const std::string& password,
             const std::string& name, const std::string& phone_number,
             const std::string& field_name, const std::string& field_address,
             const std::string& field_rules, const std::string& open_time,
             const std::string& close_time, const std::string owner_name)
    : User(user_id, user_name, password, name, phone_number),
      facility_{field_name, field_address, field_rules, open_time, close_time, true, true},
      owner_name(owner_name) {}

void Owner::show_menu() const {
    std::cout << "\nMenu cho chu san\n";
}

std::string Owner::get_account_type() const { return "Owner"; }

const FacilityConfig& Owner::getFacilityConfig() const { return facility_; }
void Owner::setFacilityConfig(const FacilityConfig& config) { facility_ = config; }

std::string Owner::get_field_name() const { return facility_.facility_name; }
void Owner::set_field_name(const std::string& field_name) { facility_.facility_name = field_name; }

std::string Owner::get_field_address() const { return facility_.field_address; }
void Owner::set_field_address(const std::string& field_address) { facility_.field_address = field_address; }

std::string Owner::get_field_rules() const { return facility_.field_rules; }
void Owner::set_field_rules(const std::string& field_rules) { facility_.field_rules = field_rules; }

std::string Owner::get_open_time() const { return facility_.open_time; }
void Owner::set_open_time(const std::string& open_time) { facility_.open_time = open_time; }

std::string Owner::get_close_time() const { return facility_.close_time; }
void Owner::set_close_time(const std::string& close_time) { facility_.close_time = close_time; }

bool Owner::allows60MinuteSlots() const { return facility_.allows_60_minute_slots; }
bool Owner::allows90MinuteSlots() const { return facility_.allows_90_minute_slots; }
void Owner::setAllowedSlotDurations(bool allows60, bool allows90) {
    facility_.allows_60_minute_slots = allows60;
    facility_.allows_90_minute_slots = allows90;
}

std::string Owner::get_owner_name() const { return owner_name; }
void Owner::set_owner_name(const std::string& new_owner_name) { owner_name = new_owner_name; }
