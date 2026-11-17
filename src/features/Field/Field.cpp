#include "Field/Field.h"
#include "Repository/VenueRepository.h"

Field::Field(const std::string& fieldId, const std::string& ownerUserId,
             const std::string& name, FieldSize size,
             long long baseHourlyRateVnd, const std::string& rules,
             const std::string& openTime, const std::string& closeTime,
             FieldStatus status)
    : field_id_(fieldId), owner_user_id_(ownerUserId), name_(name),
      size_(size), base_hourly_rate_vnd_(baseHourlyRateVnd), rules_(rules),
      open_time_(openTime), close_time_(closeTime), status_(status) {}

const std::string& Field::getFieldId() const { return field_id_; }
const std::string& Field::getOwnerUserId() const { return owner_user_id_; }
const std::string& Field::getName() const { return name_; }
FieldSize Field::getSize() const { return size_; }
long long Field::getBaseHourlyRateVnd() const { return base_hourly_rate_vnd_; }
const std::string& Field::getRules() const { return rules_; }
const std::string& Field::getOpenTime() const { return open_time_; }
const std::string& Field::getCloseTime() const { return close_time_; }
FieldStatus Field::getStatus() const { return status_; }

void Field::setName(const std::string& name) { name_ = name; }
void Field::setBaseHourlyRateVnd(long long baseHourlyRateVnd) { base_hourly_rate_vnd_ = baseHourlyRateVnd; }
void Field::setRules(const std::string& rules) { rules_ = rules; }
void Field::setOperatingHours(const std::string& openTime, const std::string& closeTime) {
    open_time_ = openTime;
    close_time_ = closeTime;
}
void Field::setStatus(FieldStatus status) { status_ = status; }

bool Field::isAvailable() const {
    return status_ == FieldStatus::ACTIVE;
}

bool Field::isUnderMaintenance() const {
    return status_ == FieldStatus::MAINTENANCE;
}

void Field::toggleMaintenance() {
    if (status_ == FieldStatus::MAINTENANCE) {
        status_ = FieldStatus::ACTIVE;
    } else {
        status_ = FieldStatus::MAINTENANCE;
    }
}

bool Field::supportsDuration(int durationMinutes) const {
    return durationMinutes == 60 || durationMinutes == 90;
}

long long Field::calculatePrice(int durationMinutes) const {
    if (durationMinutes <= 0) return 0;
    return (base_hourly_rate_vnd_ * durationMinutes) / 60;
}

FieldStatus Field::statusToEnum(int status) {
    if (status == 1) return FieldStatus::INACTIVE;
    if (status == 2) return FieldStatus::MAINTENANCE;
    return FieldStatus::ACTIVE;
}

int Field::enumToStatus(FieldStatus status) {
    if (status == FieldStatus::INACTIVE) return 1;
    if (status == FieldStatus::MAINTENANCE) return 2;
    return 0;
}

Field Field::fromRecord(const VenueFieldRecord& record) {
    const FieldSize size = record.size == 11
        ? FieldSize::ELEVEN
        : (record.size == 7 ? FieldSize::SEVEN : FieldSize::FIVE);
    return Field(
        record.fieldId,
        record.ownerId,
        record.name,
        size,
        0,
        record.rules,
        "06:00",
        "23:00",
        statusToEnum(record.status)
    );
}

VenueFieldRecord Field::toRecord() const {
    VenueFieldRecord rec;
    rec.fieldId = field_id_;
    rec.ownerId = owner_user_id_;
    rec.name = name_;
    rec.size = static_cast<int>(size_);
    rec.status = enumToStatus(status_);
    rec.rules = rules_;
    return rec;
}
