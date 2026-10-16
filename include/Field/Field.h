#ifndef FIELD_H
#define FIELD_H

#include <string>

enum class FieldSize {
    FIVE = 5,
    SEVEN = 7,
    ELEVEN = 11
};

enum class FieldStatus : int {
    ACTIVE,
    INACTIVE,
    MAINTENANCE
};

// LÝ DO CHỌN: Field đại diện một sân vật lý có thể đặt độc lập; lịch đặt chỉ
// được lưu ở Booking để không trùng dữ liệu khả dụng theo ngày và giờ.
class Field {
private:
    std::string field_id_;
    std::string owner_user_id_;
    std::string name_;
    FieldSize size_;
    long long base_hourly_rate_vnd_;
    std::string rules_;
    std::string open_time_;
    std::string close_time_;
    FieldStatus status_;

public:
    Field(const std::string& fieldId, const std::string& ownerUserId,
          const std::string& name, FieldSize size, long long baseHourlyRateVnd,
          const std::string& rules, const std::string& openTime,
          const std::string& closeTime,
          FieldStatus status = FieldStatus::ACTIVE);

    const std::string& getFieldId() const;
    const std::string& getOwnerUserId() const;
    const std::string& getName() const;
    FieldSize getSize() const;
    long long getBaseHourlyRateVnd() const;
    const std::string& getRules() const;
    const std::string& getOpenTime() const;
    const std::string& getCloseTime() const;
    FieldStatus getStatus() const;

    void setName(const std::string& name);
    void setBaseHourlyRateVnd(long long baseHourlyRateVnd);
    void setRules(const std::string& rules);
    void setOperatingHours(const std::string& openTime,
                           const std::string& closeTime);
    void setStatus(FieldStatus status);
};

#endif
