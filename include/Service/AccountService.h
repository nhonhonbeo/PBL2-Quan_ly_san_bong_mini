#pragma once

#include <memory>
#include <string>
#include <vector>

class AccountRepository;
class User;
class CustomerRepository;

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
    std::string generateNextUserId() const;
    bool registerCustomer(const std::string& name, const std::string& phone,
                          const std::string& pass, CustomerRepository& custRepo,
                          std::string& error);
    bool registerOwner(const std::string& name, const std::string& phone,
                       const std::string& pass, const std::string& facilityName,
                       const std::string& address, std::string& error);
    bool registerUser(const std::shared_ptr<User>& user, std::string& error);
    bool updateUser(const std::string& oldPhone,
                    const std::string& newName,
                    const std::string& newPhone,
                    std::string& error);
    const std::vector<std::shared_ptr<User>>& getAccounts() const;
};
