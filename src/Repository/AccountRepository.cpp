#include "Repository/AccountRepository.h"
#include "Storage/FileManager.h"
AccountRepository::AccountRepository(FileManager& manager):file_manager_(manager){} bool AccountRepository::load(std::vector<std::shared_ptr<User>>& accounts,std::string& error)const{if(!FileManager::docDanhSachUser(accounts)){error="Khong the nap tai khoan";return false;}return true;} bool AccountRepository::save(const std::vector<std::shared_ptr<User>>& accounts,std::string& error)const{if(!FileManager::ghiDanhSachUser(accounts)){error="Khong the luu tai khoan";return false;}return true;}
