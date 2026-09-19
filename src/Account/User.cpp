#include "../../include/Account/User.h"

User::User(const std::string& user_id, const std::string& user_name, const std::string& password, const std::string& name, const std::string& phone_number) 
    : user_id(user_id), user_name(user_name), password(password), name(name), phone_number(phone_number) {}

User::~User() {}

std::string User::get_user_id() const { return user_id; }
std::string User::get_user_name() const { return user_name; }
std::string User::get_name() const { return name; }
std::string User::get_phone_number() const { return phone_number; }

void User::rename(const std::string& new_name) { this->name = new_name; }
void User::update_pw(const std::string& new_password) { this->password = new_password; }
