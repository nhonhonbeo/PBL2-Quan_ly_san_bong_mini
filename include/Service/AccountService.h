#pragma once
#include <memory>
#include <string>
class AccountRepository; class User;
class AccountService {
    AccountRepository& account_repository_;
public:
    explicit AccountService(AccountRepository& accountRepository);
    bool loadAccounts(std::string& error);
    std::shared_ptr<User> authenticate(const std::string& phone, const std::string& password, std::string& error) const;
    bool resetPassword(const std::string& phone, const std::string& password, std::string& error);
};
