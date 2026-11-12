#pragma once

#include <memory>
#include <string>
#include <vector>

class User;
class FileManager;

// LÝ DO CHỌN: AccountRepository đọc và ghi danh sách tài khoản từ data/accounts.txt.
class AccountRepository {
private:
    FileManager& file_manager_;

public:
    explicit AccountRepository(FileManager& fileManager);
    bool load(std::vector<std::shared_ptr<User>>& accounts, std::string& error) const;
    bool save(const std::vector<std::shared_ptr<User>>& accounts, std::string& error) const;
};
