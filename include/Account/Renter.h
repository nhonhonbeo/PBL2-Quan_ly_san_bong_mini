#ifndef NGUOITHUE_H
#define NGUOITHUE_H
#include<string>
#include<User.h>

class Customer : public User{
private:
    int booking_count;
    int cancel_count;
public:
    Customer(
        const std::string& user_id,
        const std::string& user_name,
        const std::string& password,
        const std::string& name,
        const std::string& phone_number
    );
    
    void show_menu() const override;
    std::string get_account_type() const override;

    int get_booking_count() const;
    int get_cancel_count() const;

    void increase_booking_count() const;
    void cancel_count() const;

    bool reg_customer() const;
};
#endif