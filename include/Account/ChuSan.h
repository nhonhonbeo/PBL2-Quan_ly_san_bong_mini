#ifndef OWNER_H
#define OWNER_H
#include<string>
using namespace std;

class Owner: public User{
private: 
    //field
    string field_name;
    string field_address;
    string field_rules;
    string field_count;

    //time
    string open_time;
    string close_time;
    
    //personal information
    string owner_name;

public:
    Owner(
        const string& user_id,
        const string& user_name,
        const string& password,
        const string& name,
        const string& phone_number,
        const string& field_name,
        const string& field_address,
        const string& field_rules,
        const string& open_time,
        const string& close_time,
        const string owner_name
    );

    void show_menu() const override;
    string get_account_type() const override; 

    string get_field_name() const;
    void set_field_name(const std::string& field_names);

    string get_field_address() const;
    void set_field_address(const std::string& new_field_address);

    string get_field_rules() const;
    void set_field_rules(const std::string& field_rules);

    string get_close_time() const;
    void set_open_time(const std::string& new_close_time);

    string get_close_time() const;
    void set_close_time(const std::string& new_close_time);

    string owner_name() const;
    void set_owner_name(const std::string& new_owner_name);

}