#pragma once

#include <memory>
#include <string>
#include <vector>

class AccountRepository;
class User;

// LÝ DO CHỌN: AccountService quản lý trạng thái tài khoản người dùng và xác thực,
// loại bỏ biến toàn cục ẩn danh để đảm bảo tính đóng gói hướng đối tượng.
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
