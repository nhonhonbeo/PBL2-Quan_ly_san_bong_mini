#ifndef FIELD_SERVICE_H
#define FIELD_SERVICE_H

#include <string>

#include "Core/LinkedList.h"
#include "Repository/VenueRepository.h"
#include "Service/VenueService.h"

class Field;
class FieldRepository;
class TimeSlot;
class VenueRepository;
class VenueService;
enum class FieldStatus : int;

// LÝ DO CHỌN: FieldService quản lý các nghiệp vụ trên sân vật lý (tạo sân hàng loạt,
// cập nhật trạng thái bảo trì, xác thực quy chuẩn sân), nhận VenueService và
// VenueRepository qua Dependency Injection.
class FieldService {
private:
    FieldRepository& field_repository_;
    VenueService* venue_service_;
    VenueRepository* venue_repository_;
    VenueRepository default_venue_repo_;
    VenueService default_venue_service_;

public:
    explicit FieldService(FieldRepository& fieldRepository,
                          VenueService* venueService = nullptr,
                          VenueRepository* venueRepository = nullptr);
    void setVenueDependencies(VenueService* venueService, VenueRepository* venueRepository);

    bool createMultipleFields(const std::string& requesterOwnerId, int loaiSan,
                              int soLuong, const std::string& quyDinh,
                              LinkedList<std::string>& outMaSan, std::string& error);
    bool createField(const std::string& requesterOwnerId,
                     const VenueFieldRecord& field, std::string& error);
    bool updateField(const std::string& requesterOwnerId,
                     const VenueFieldRecord& field, std::string& error);
    bool deleteField(const std::string& requesterOwnerId,
                     const std::string& fieldId, std::string& error);
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

