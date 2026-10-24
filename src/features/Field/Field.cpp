#include "Field/Field.h"

Field::Field(const std::string& fieldId, const std::string& ownerUserId,
             const std::string& name, FieldSize size,
             long long baseHourlyRateVnd, const std::string& rules,
             const std::string& openTime, const std::string& closeTime,
             FieldStatus status)
    : field_id_(fieldId), owner_user_id_(ownerUserId), name_(name), size_(size),
      base_hourly_rate_vnd_(baseHourlyRateVnd), rules_(rules),
      open_time_(openTime), close_time_(closeTime), status_(status) {}

const std::string& Field::getFieldId() const { return field_id_; }

const std::string& Field::getOwnerUserId() const { return owner_user_id_; }

const std::string& Field::getName() const { return name_; }

FieldSize Field::getSize() const { return size_; }

long long Field::getBaseHourlyRateVnd() const {
    return base_hourly_rate_vnd_;
}

const std::string& Field::getRules() const { return rules_; }

const std::string& Field::getOpenTime() const { return open_time_; }

const std::string& Field::getCloseTime() const { return close_time_; }

FieldStatus Field::getStatus() const { return status_; }

void Field::setName(const std::string& name) { name_ = name; }

void Field::setBaseHourlyRateVnd(long long baseHourlyRateVnd) {
    base_hourly_rate_vnd_ = baseHourlyRateVnd;
}

void Field::setRules(const std::string& rules) { rules_ = rules; }

void Field::setOperatingHours(const std::string& openTime,
                              const std::string& closeTime) {
    open_time_ = openTime;
    close_time_ = closeTime;
}

void Field::setStatus(FieldStatus status) { status_ = status; }
