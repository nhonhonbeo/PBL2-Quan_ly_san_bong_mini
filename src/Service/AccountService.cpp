#include "Service/AccountService.h"

#include <cctype>
#include <cstdio>
#include <algorithm>
#include "Account/Customer.h"
#include "Account/Owner.h"
#include "Account/User.h"
#include "Repository/AccountRepository.h"
#include "Repository/CustomerRepository.h"

namespace {

// LÝ DO CHỌN: Validation số điện thoại và mật khẩu ở tầng Service đảm bảo tính toàn vẹn dữ liệu
// ngay cả khi gọi ngoài GUI (kiến trúc độc lập).
bool kiemTraSdtHopLe(const std::string& phone, std::string& error) {
    if (phone.empty()) {
        error = u8"Số điện thoại không được để trống.";
        return false;
    }
    if (phone.length() != 10 || phone[0] != '0') {
        error = u8"Số điện thoại phải gồm đúng 10 chữ số và bắt đầu bằng số 0.";
        return false;
    }
    for (char c : phone) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            error = u8"Số điện thoại chỉ được chứa ký tự số.";
            return false;
        }
    }
    return true;
}

bool kiemTraMatKhauHopLe(const std::string& pass, std::string& error) {
    if (pass.length() < 6) {
        error = u8"Mật khẩu phải có ít nhất 6 ký tự.";
        return false;
    }
    return true;
}

} // namespace

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
    if (!kiemTraMatKhauHopLe(password, error)) return false;
    for (const auto& account : accounts_) {
        if (account && account->get_phone_number() == phone) {
            account->update_pw(password);
            return account_repository_.save(accounts_, error);
        }
    }
    error = "Khong tim thay tai khoan";
    return false;
}

// LÝ DO CHỌN: Quét mã số lớn nhất (max sequence) của các User ID có tiền tố "US"
// thay vì chỉ đếm accounts_.size() + 1, chống hoàn toàn nguy cơ trùng lặp ID khi xóa tài khoản.
std::string AccountService::generateNextUserId() const {
    int maxSeq = 0;
    for (const auto& u : accounts_) {
        if (!u) continue;
        const std::string& id = u->get_user_id();
        if (id.rfind("US", 0) == 0 && id.length() > 2) {
            try {
                int seq = std::stoi(id.substr(2));
                if (seq > maxSeq) maxSeq = seq;
            } catch (...) {}
        }
    }
    char buf[16];
    std::snprintf(buf, sizeof(buf), "US%02d", maxSeq + 1);
    return std::string(buf);
}

bool AccountService::registerCustomer(const std::string& name, const std::string& phone,
                                      const std::string& pass, CustomerRepository& custRepo,
                                      std::string& error) {
    if (name.empty()) {
        error = u8"Họ tên không được để trống.";
        return false;
    }
    if (!kiemTraSdtHopLe(phone, error)) return false;
    if (!kiemTraMatKhauHopLe(pass, error)) return false;

    std::string newId = generateNextUserId();
    auto cust = std::make_shared<Customer>(newId, phone, pass, name, phone);
    if (!registerUser(cust, error)) return false;
    custRepo.saveCustomer(*cust, error);
    return true;
}

bool AccountService::registerOwner(const std::string& name, const std::string& phone,
                                   const std::string& pass, const std::string& facilityName,
                                   const std::string& address, std::string& error) {
    if (name.empty()) {
        error = u8"Họ tên chủ sân không được để trống.";
        return false;
    }
    if (!kiemTraSdtHopLe(phone, error)) return false;
    if (!kiemTraMatKhauHopLe(pass, error)) return false;

    std::string newId = generateNextUserId();
    auto owner = std::make_shared<Owner>(newId, phone, pass, name, phone, facilityName, address,
                                         u8"Quy định tiêu chuẩn cụm sân", "06:00", "23:00", name);
    return registerUser(owner, error);
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
        if (existing && existing->get_user_id() == user->get_user_id()) {
            error = "Ma nguoi dung da ton tai";
            return false;
        }
        if (existing && existing->get_user_name() == user->get_user_name()) {
            error = "Ten dang nhap da duoc su dung";
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
    if (!kiemTraSdtHopLe(newPhone, error)) return false;
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
