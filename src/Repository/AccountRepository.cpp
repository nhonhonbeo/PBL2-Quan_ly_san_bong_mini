#include "Repository/AccountRepository.h"

#include "Storage/FileManager.h"

AccountRepository::AccountRepository(FileManager& manager)
    : file_manager_(manager) {}

bool AccountRepository::load(std::vector<std::shared_ptr<User>>& accounts,
                             std::string& error) const {
    return FileManager::docDanhSachUser(accounts, "data/accounts.txt", error);
}

bool AccountRepository::save(const std::vector<std::shared_ptr<User>>& accounts,
                             std::string& error) const {
    return FileManager::ghiDanhSachUser(accounts, "data/accounts.txt", error);
}
