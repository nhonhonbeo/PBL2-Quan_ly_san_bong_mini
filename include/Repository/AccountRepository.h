#pragma once
#include <memory>
#include <string>
#include <vector>
class User; class FileManager;
class AccountRepository {
    FileManager& file_manager_;
public:
    explicit AccountRepository(FileManager& fileManager);
    bool load(std::vector<std::shared_ptr<User>>& accounts, std::string& error) const;
    bool save(const std::vector<std::shared_ptr<User>>& accounts, std::string& error) const;
};
