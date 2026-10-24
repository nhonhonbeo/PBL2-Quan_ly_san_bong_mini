#ifndef FIELD_REPOSITORY_H
#define FIELD_REPOSITORY_H

#include <string>

#include "Core/LinkedList.h"
#include "Repository/VenueRepository.h"

class Field;
class FileManager;

// LÝ DO CHỌN: Repository chỉ lưu/truy xuất Field; validation và quyền sở hữu
// thuộc FieldService, còn lịch đặt thuộc BookingService.
class FieldRepository {
private:
    FileManager& file_manager_;

public:
    explicit FieldRepository(FileManager& fileManager);

    bool findById(const std::string& fieldId, Field& out,
                  std::string& error) const;
    bool loadAll(LinkedList<VenueFieldRecord>& fields,
                 std::string& error) const;
    bool save(const Field& field, std::string& error);
    bool update(const Field& field, std::string& error);
    bool remove(const std::string& fieldId, std::string& error);
};

#endif
