#ifndef FIELD_SERVICE_H
#define FIELD_SERVICE_H

#include <string>

#include "Core/LinkedList.h"
#include "Repository/VenueRepository.h"

class Field;
class FieldRepository;
class TimeSlot;
class VenueRepository;
enum class FieldStatus : int;

// LÝ DO CHỌN: FieldService kiểm tra cấu hình sân và giờ hoạt động; kiểm tra
// trống/đã đặt theo ngày thuộc BookingService để không nhân đôi lịch.
class FieldService {
private:
    FieldRepository& field_repository_;

public:
    explicit FieldService(FieldRepository& fieldRepository);

    bool createField(const std::string& requesterOwnerId,
                     const VenueFieldRecord& field, std::string& error);
    bool updateField(const std::string& requesterOwnerId,
                     const VenueFieldRecord& field, std::string& error);
    bool setFieldStatus(const std::string& requesterOwnerId,
                        const std::string& fieldId, FieldStatus status,
                        std::string& error);
    bool getFields(const std::string& ownerId, int fieldSize,
                   bool includeUnavailable,
                   LinkedList<VenueFieldRecord>& fields,
                   std::string& error) const;
    bool setPrice(const std::string& requesterOwnerId, int fieldSize,
                  int durationMinutes, const std::string& startTime,
                  long long priceVnd, std::string& error) const;
    bool validateOperatingSlot(const Field& field, const TimeSlot& slot,
                               std::string& error) const;
};

#endif
