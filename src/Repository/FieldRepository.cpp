#include "Repository/FieldRepository.h"

#include "Field/Field.h"
#include "Storage/FileManager.h"

namespace {

FieldStatus statusToEnum(int status) {
    if (status == 1) return FieldStatus::INACTIVE;
    if (status == 2) return FieldStatus::MAINTENANCE;
    return FieldStatus::ACTIVE;
}

int enumToStatus(FieldStatus status) {
    if (status == FieldStatus::INACTIVE) return 1;
    if (status == FieldStatus::MAINTENANCE) return 2;
    return 0;
}

VenueFieldRecord toRecord(const Field& field) {
    VenueFieldRecord record;
    record.fieldId = field.getFieldId();
    record.ownerId = field.getOwnerUserId();
    record.name = field.getName();
    record.size = static_cast<int>(field.getSize());
    record.status = enumToStatus(field.getStatus());
    record.rules = field.getRules();
    return record;
}

}  // Kết thúc không gian tên ẩn danh

FieldRepository::FieldRepository(FileManager& fileManager)
    : file_manager_(fileManager) {}

bool FieldRepository::loadAll(LinkedList<VenueFieldRecord>& fields,
                              std::string& error) const {
    VenueRepository repository;
    return repository.loadFields(fields, error);
}

bool FieldRepository::findById(const std::string& fieldId, Field& out,
                               std::string& error) const {
    LinkedList<VenueFieldRecord> fields;
    if (!loadAll(fields, error)) return false;
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        const VenueFieldRecord& record = node->data;
        if (record.fieldId != fieldId) continue;
        const FieldSize size = record.size == 11
            ? FieldSize::ELEVEN
            : (record.size == 7 ? FieldSize::SEVEN : FieldSize::FIVE);
        out = Field(
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
        return true;
    }
    error = "Không tìm thấy sân.";
    return false;
}

bool FieldRepository::save(const Field& field, std::string& error) {
    LinkedList<VenueFieldRecord> fields;
    if (!loadAll(fields, error)) return false;
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        if (node->data.fieldId == field.getFieldId()) {
            error = "Mã sân đã tồn tại.";
            return false;
        }
    }
    fields.push_back(toRecord(field));
    VenueRepository repository;
    return repository.saveFields(fields, error);
}

bool FieldRepository::update(const Field& field, std::string& error) {
    LinkedList<VenueFieldRecord> fields;
    if (!loadAll(fields, error)) return false;
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        if (node->data.fieldId != field.getFieldId()) continue;
        if (node->data.ownerId != field.getOwnerUserId()) {
            error = "Không thể thay đổi chủ sở hữu của sân.";
            return false;
        }
        node->data = toRecord(field);
        VenueRepository repository;
        return repository.saveFields(fields, error);
    }
    error = "Không tìm thấy sân cần cập nhật.";
    return false;
}

bool FieldRepository::remove(const std::string& fieldId, std::string& error) {
    LinkedList<VenueFieldRecord> fields;
    if (!loadAll(fields, error)) return false;
    if (!fields.removeIf([&fieldId](const VenueFieldRecord& field) {
            return field.fieldId == fieldId;
        })) {
        error = "Không tìm thấy sân cần xóa.";
        return false;
    }
    VenueRepository repository;
    return repository.saveFields(fields, error);
}
