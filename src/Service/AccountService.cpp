#include "Service/AccountService.h"
#include "Repository/AccountRepository.h"
#include "Account/User.h"
namespace { std::vector<std::shared_ptr<User>> accounts; }
AccountService::AccountService(AccountRepository& repository):account_repository_(repository){}
bool AccountService::loadAccounts(std::string& error){return account_repository_.load(accounts,error);} std::shared_ptr<User> AccountService::authenticate(const std::string& phone,const std::string& password,std::string& error)const{for(const auto& account:accounts)if(account&&account->get_phone_number()==phone){if(account->get_password()==password)return account;error="Mat khau khong dung";return nullptr;}error="Khong tim thay tai khoan";return nullptr;} bool AccountService::resetPassword(const std::string& phone,const std::string& password,std::string& error){for(const auto& account:accounts)if(account&&account->get_phone_number()==phone){account->update_pw(password);return account_repository_.save(accounts,error);}error="Khong tim thay tai khoan";return false;}
