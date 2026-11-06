#pragma once

#include <memory>
#include <string>
#include <vector>

class AccountRepository;
class User;

// L? DO CH?N: AccountService qu?n l? tr?ng th?i t?i kho?n ng??i d?ng v? x?c th?c,
// lo?i b? bi?n to?n c?c ?n danh ?? ??m b?o t?nh ??ng g?i h??ng ??i t??ng.
class AccountService {
private:
    AccountRepository& account_repository_;
    std::vector<std::shared_ptr<User>> accounts_;

public:
    explicit AccountService(AccountRepository& accountRepository);

    bool loadAccounts(std::string& error);
    std::shared_ptr<User> authenticate(const std::string& phone,
                                       const std::string& password,
                                       std::string& error) const;
    bool resetPassword(const std::string& phone,
                       const std::string& password,
                       std::string& error);
    bool registerUser(const std::shared_ptr<User>& user, std::string& error);
    bool updateUser(const std::string& oldPhone,
                    const std::string& newName,
                    const std::string& newPhone,
                    std::string& error);
    const std::vector<std::shared_ptr<User>>& getAccounts() const;
};
