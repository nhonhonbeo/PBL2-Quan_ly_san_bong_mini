#ifndef USER_H
#define USER_H
#include<string>
using namespace std;

class User{
protected: 
    std::string user_id;
    std::string user_name;
    std::string password;
    std::string name;
    std::string phone_number;
public: 
    User(
        const std::string& user_id,
        const std::string& user_name,
        const std::string& password,
        const std::string& name,
        const std::string& phone_number
    );
    virtual ~User();

    std::string get_user_id() const;
    std::string get_user_name() const;
    std::string get_name() const;
    std::string get_phone_number() const;

    void rename(const std::string& new_name);
    void update_pw(const std::string& new_password);

    virtual void show_menu() const = 0;
    virtual std::string get_account_type() const = 0;
};
#endif