#include "Service/FieldService.h"

#include "Core/DateTime.h"
#include "Field/Field.h"
#include "Field/TimeSlot.h"
#include "Repository/FieldRepository.h"
#include "Repository/VenueRepository.h"
#include "Service/VenueService.h"

namespace {

bool isValidSize(int size) {
    return size == 5 || size == 7 || size == 11;
}

}  // Kết thúc không gian tên ẩn danh

FieldService::FieldService(FieldRepository& fieldRepository)
    : field_repository_(fieldRepository) {}

bool FieldService::createField(const std::string& requesterOwnerId,
                               const VenueFieldRecord& field,
                               std::string& error) {
    VenueRepository repository;
    VenueService service(repository);
    return service.createField(requesterOwnerId, field, error);
}

bool FieldService::updateField(const std::string& requesterOwnerId,
                               const VenueFieldRecord& field,
                               std::string& error) {
    VenueRepository repository;
    VenueService service(repository);
    return service.updateField(requesterOwnerId, field, error);
}

bool FieldService::setFieldStatus(const std::string& requesterOwnerId,
                                  const std::string& fieldId,
                                  FieldStatus status,
                                  std::string& error) {
    LinkedList<VenueFieldRecord> fields;
    if (!field_repository_.loadAll(fields, error)) return false;
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        VenueFieldRecord& field = node->data;
        if (field.fieldId != fieldId) continue;
        if (field.ownerId != requesterOwnerId) {
            error = "Không có quyền thay đổi trạng thái sân của chủ sân khác.";
            return false;
        }
        field.status = static_cast<int>(status);
        VenueRepository repository;
        return repository.saveFields(fields, error);
    }
    error = "Không tìm thấy sân cần cập nhật trạng thái.";
    return false;
}

bool FieldService::getFields(const std::string& ownerId, int fieldSize,
                             bool includeUnavailable,
                             LinkedList<VenueFieldRecord>& fields,
                             std::string& error) const {
    if (fieldSize != 0 && !isValidSize(fieldSize)) {
        error = "Loại sân không hợp lệ.";
        return false;
    }
    VenueRepository repository;
    VenueService service(repository);
    return service.getFields(ownerId, fieldSize, includeUnavailable, fields, error);
}

bool FieldService::setPrice(const std::string& requesterOwnerId,
                            int fieldSize,
                            int durationMinutes,
                            const std::string& startTime,
                            long long priceVnd,
                            std::string& error) const {
    VenueRepository repository;
    VenueService service(repository);
    return service.setFieldPrice(
        requesterOwnerId, fieldSize, durationMinutes, startTime, priceVnd, error
    );
}

bool FieldService::validateOperatingSlot(const Field& field,
                                         const TimeSlot& slot,
                                         std::string& error) const {
    if (!DateTime::isValidTime(slot.getStartTime()) ||
        !DateTime::isValidTime(slot.getEndTime())) {
        error = "Giờ bắt đầu hoặc kết thúc không đúng định dạng HH:MM.";
        return false;
    }
    if (DateTime::compareTime(slot.getStartTime(), slot.getEndTime()) >= 0) {
        error = "Giờ kết thúc phải sau giờ bắt đầu.";
        return false;
    }
    if (DateTime::compareTime(slot.getStartTime(), field.getOpenTime()) < 0 ||
        DateTime::compareTime(slot.getEndTime(), field.getCloseTime()) > 0) {
        error = "Khung giờ nằm ngoài giờ hoạt động.";
        return false;
    }
    return true;
}
