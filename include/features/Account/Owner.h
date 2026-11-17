#ifndef OWNER_H
#define OWNER_H

#include <string>
#include "User.h"

// LÝ DO CHỌN: Tách thông tin cụm cơ sở thành Value Object FacilityConfig,
// áp dụng Composition over inheritance thay vì nhồi nhét trực tiếp vào Owner entity.
struct FacilityConfig {
    std::string facility_name;
    std::string field_address;
    std::string field_rules;
    std::string open_time = "06:00";
    std::string close_time = "23:00";
    bool allows_60_minute_slots = true;
    bool allows_90_minute_slots = true;
};

class Owner : public User {
private:
    FacilityConfig facility_;
    std::string owner_name;

public:
    Owner(
        const std::string& user_id,
        const std::string& user_name,
        const std::string& password,
        const std::string& name,
        const std::string& phone_number,
        const std::string& field_name,
        const std::string& field_address,
        const std::string& field_rules,
        const std::string& open_time,
        const std::string& close_time,
        const std::string owner_name
    );

    void show_menu() const override;
    std::string get_account_type() const override;

    const FacilityConfig& getFacilityConfig() const;
    void setFacilityConfig(const FacilityConfig& config);

    std::string get_field_name() const;
    void set_field_name(const std::string& field_names);

    std::string get_field_address() const;
    void set_field_address(const std::string& new_field_address);

    std::string get_field_rules() const;
    void set_field_rules(const std::string& field_rules);

    std::string get_open_time() const;
    void set_open_time(const std::string& new_open_time);

    std::string get_close_time() const;
    void set_close_time(const std::string& new_close_time);

    bool allows60MinuteSlots() const;
    bool allows90MinuteSlots() const;
    void setAllowedSlotDurations(bool allows60, bool allows90);

    std::string get_owner_name() const;
    void set_owner_name(const std::string& new_owner_name);
};

#endif
