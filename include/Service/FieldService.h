#ifndef FIELD_SERVICE_H
#define FIELD_SERVICE_H

#include <string>

class Field;
class FieldRepository;
class TimeSlot;
enum class FieldStatus : int;

// LÝ DO CHỌN: FieldService kiểm tra cấu hình sân và giờ hoạt động; kiểm tra
// trống/đã đặt theo ngày thuộc BookingService để không nhân đôi lịch.
class FieldService {
private:
    FieldRepository& field_repository_;

public:
    explicit FieldService(FieldRepository& fieldRepository);

    bool createField(const Field& field, std::string& error);
    bool updateField(const Field& field, std::string& error);
    bool setFieldStatus(const std::string& fieldId, FieldStatus status,
                        std::string& error);
    bool validateOperatingSlot(const Field& field, const TimeSlot& slot,
                               std::string& error) const;
};

#endif
