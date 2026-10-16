#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include "Account/User.h"

namespace GUI::App {

// LÝ DO CHỌN: Singleton Session tập trung lưu trữ trạng thái phiên đăng nhập của người dùng toàn ứng dụng,
// giúp các màn hình và widget dễ dàng truy xuất thông tin người dùng hiện tại mà không cần truyền tham số rườm rà.
class Session {
public:
    static Session& getInstance() {
        static Session instance;
        return instance;
    }

    // Thiết lập phiên đăng nhập mới
    void login(std::shared_ptr<User> user, const std::string& role) {
        currentUser_ = user;
        role_ = role;
        loginTime_ = layThoiGianHienTai();
    }

    // Đăng xuất và dọn dẹp phiên làm việc
    void logout() {
        currentUser_.reset();
        role_.clear();
        loginTime_.clear();
    }

    // Kiểm tra trạng thái đã đăng nhập
    bool isLoggedIn() const {
        return currentUser_ != nullptr;
    }

    // Lấy con trỏ đối tượng người dùng hiện tại
    User* getUser() const {
        return currentUser_.get();
    }

    std::shared_ptr<User> getUserPtr() const {
        return currentUser_;
    }

    // Lấy vai trò hệ thống ("Owner" hoặc "Customer")
    std::string getRole() const {
        return role_;
    }

    // Tên hiển thị thân thiện tiếng Việt của vai trò
    std::string getRoleDisplayName() const {
        if (role_ == "Owner") {
            return u8"Chủ sân";
        }
        if (role_ == "Customer") {
            return u8"Người thuê";
        }
        return u8"Khách vãng lai";
    }

    // Lấy thời điểm đăng nhập dạng chuỗi (HH:mm:ss dd/MM/yyyy)
    std::string getLoginTime() const {
        return loginTime_;
    }

private:
    Session() = default;
    ~Session() = default;

    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

    std::string layThoiGianHienTai() {
        auto now = std::chrono::system_clock::now();
        std::time_t tt = std::chrono::system_clock::to_time_t(now);
        std::tm localTm{};
#if defined(_WIN32)
        localtime_s(&localTm, &tt);
#else
        localtime_r(&tt, &localTm);
#endif
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << localTm.tm_hour << ":"
            << std::setfill('0') << std::setw(2) << localTm.tm_min << ":"
            << std::setfill('0') << std::setw(2) << localTm.tm_sec << " "
            << std::setfill('0') << std::setw(2) << localTm.tm_mday << "/"
            << std::setfill('0') << std::setw(2) << (localTm.tm_mon + 1) << "/"
            << (localTm.tm_year + 1900);
        return oss.str();
    }

    std::shared_ptr<User> currentUser_ = nullptr;
    std::string role_;
    std::string loginTime_;
};

}  // namespace GUI::App
