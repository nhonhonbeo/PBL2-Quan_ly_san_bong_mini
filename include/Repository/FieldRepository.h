#ifndef FIELD_REPOSITORY_H
#define FIELD_REPOSITORY_H

#include <string>

#include "Core/LinkedList.h"
#include "Repository/VenueRepository.h"

class Field;

// LÝ DO CHỌN: FieldRepository hoạt động như một persistence adapter chuẩn hóa giữa
// domain model Field và bảng lưu trữ VenueFieldRecord, nhận VenueRepository qua DI.
class FieldRepository {
private:
    VenueRepository& venue_repository_;

public:
    explicit FieldRepository(VenueRepository& venueRepository);

    bool findById(const std::string& fieldId, Field& out,
                  std::string& error) const;
    bool loadAll(LinkedList<VenueFieldRecord>& fields,
                 std::string& error) const;
    bool save(const Field& field, std::string& error);
    bool update(const Field& field, std::string& error);
    bool remove(const std::string& fieldId, std::string& error);
};

#endif
