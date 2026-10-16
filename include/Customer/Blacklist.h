#ifndef BLACKLIST_H
#define BLACKLIST_H

#include <string>

// LÝ DO CHỌN: Blacklist là một bản ghi của một khách; Repository mới quản lý
// tập hợp bản ghi để Entity không phụ thuộc cấu trúc lưu trữ.
class Blacklist {
private:
    std::string customer_id_;
    std::string reason_;
    std::string created_at_;
    bool active_;

public:
    Blacklist(const std::string& customerId, const std::string& reason,
              const std::string& createdAt, bool active = true);

    const std::string& getCustomerId() const;
    const std::string& getReason() const;
    const std::string& getCreatedAt() const;
    bool isActive() const;

    // Việc có được gỡ blacklist hay không là rule của CustomerService.
    void deactivate();
};

#endif
