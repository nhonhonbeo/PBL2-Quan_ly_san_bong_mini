#include "Repository/FieldRepository.h"

#include "Field/Field.h"
#include "Storage/FileManager.h"

FieldRepository::FieldRepository(FileManager& fileManager, VenueRepository* venueRepo)
    : file_manager_(fileManager),
      venue_repository_(venueRepo ? venueRepo : &default_venue_repo_),
      default_venue_repo_() {}

bool FieldRepository::loadAll(LinkedList<VenueFieldRecord>& fields,
                              std::string& error) const {
    return venue_repository_->loadFields(fields, error);
}

bool FieldRepository::findById(const std::string& fieldId, Field& out,
                               std::string& error) const {
    LinkedList<VenueFieldRecord> fields;
    if (!loadAll(fields, error)) return false;
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        if (node->data.fieldId != fieldId) continue;
        out = Field::fromRecord(node->data);
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
    fields.push_back(field.toRecord());
    return venue_repository_->saveFields(fields, error);
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
        node->data = field.toRecord();
        return venue_repository_->saveFields(fields, error);
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
    return venue_repository_->saveFields(fields, error);
}
