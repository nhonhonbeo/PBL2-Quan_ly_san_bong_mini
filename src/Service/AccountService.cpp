#include "Service/AccountService.h"

#include "Account/User.h"
#include "Repository/AccountRepository.h"

AccountService::AccountService(AccountRepository& repository)
    : account_repository_(repository), accounts_() {}

bool AccountService::loadAccounts(std::string& error) {
    return account_repository_.load(accounts_, error);
}

std::shared_ptr<User> AccountService::authenticate(const std::string& phone,
                                                   const std::string& password,
                                                   std::string& error) const {
    for (const auto& account : accounts_) {
        if (account && account->get_phone_number() == phone) {
            if (account->get_password() == password) {
                return account;
            }
            error = "Mat khau khong dung";
            return nullptr;
        }
    }
    error = "Khong tim thay tai khoan";
    return nullptr;
}

bool AccountService::resetPassword(const std::string& phone,
                                   const std::string& password,
                                   std::string& error) {
    for (const auto& account : accounts_) {
        if (account && account->get_phone_number() == phone) {
            account->update_pw(password);
            return account_repository_.save(accounts_, error);
        }
    }
    error = "Khong tim thay tai khoan";
    return false;
}

bool AccountService::registerUser(const std::shared_ptr<User>& user,
                                  std::string& error) {
    if (!user) {
        error = "Thong tin tai khoan khong hop le";
        return false;
    }
    for (const auto& existing : accounts_) {
        if (existing && existing->get_phone_number() == user->get_phone_number()) {
            error = "So dien thoai da duoc su dung";
            return false;
        }
    }
    accounts_.push_back(user);
    return account_repository_.save(accounts_, error);
}

bool AccountService::updateUser(const std::string& oldPhone,
                                const std::string& newName,
                                const std::string& newPhone,
                                std::string& error) {
    if (newPhone.empty()) {
        error = "So dien thoai khong duoc de trong";
        return false;
    }
    if (oldPhone != newPhone) {
        for (const auto& existing : accounts_) {
            if (existing && existing->get_phone_number() == newPhone) {
                error = "So dien thoai moi da duoc su dung";
                return false;
            }
        }
    }
    for (const auto& account : accounts_) {
        if (account && account->get_phone_number() == oldPhone) {
            account->rename(newName);
            account->update_phone_number(newPhone);
            return account_repository_.save(accounts_, error);
        }
    }
    error = "Khong tim thay tai khoan de cap nhat";
    return false;
}

const std::vector<std::shared_ptr<User>>& AccountService::getAccounts() const {
    return accounts_;
}
