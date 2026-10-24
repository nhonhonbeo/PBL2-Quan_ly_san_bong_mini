#pragma once

#include <string>

#include "Core/LinkedList.h"
#include "Repository/VenueRepository.h"

struct VenueSlotRecord {
    std::string startTime;
    std::string endTime;
    std::string bookingId;
    long long priceVnd = 0;
    bool available = false;
    bool past = false;
};

struct VenueBookingRequest {
    std::string bookingId;
    std::string customerId;
    std::string customerName;
    std::string customerPhone;
    std::string fieldId;
    std::string date;
    std::string startTime;
    int durationMinutes = 60;
};

// LÝ DO CHỌN: VenueService gom các quy tắc tra cứu, phân quyền và đặt ca ở
// tầng nghiệp vụ; Facade chỉ chuyển tiếp yêu cầu từ GUI.
class VenueService {
public:
    explicit VenueService(const VenueRepository& repository);

    bool getFacilities(LinkedList<VenueFacilityRecord>& facilities,
                       std::string& error) const;
    bool getLowestPrice(const std::string& ownerId, long long& priceVnd,
                        std::string& error) const;
    bool getFields(const std::string& ownerId, int fieldSize,
                   bool includeUnavailable,
                   LinkedList<VenueFieldRecord>& fields,
                   std::string& error) const;
    bool getSlots(const std::string& fieldId, const std::string& date,
                  int durationMinutes, LinkedList<VenueSlotRecord>& slots,
                  std::string& error, bool allowPastDate = false) const;
    bool setFieldPrice(const std::string& requesterOwnerId, int fieldSize,
                       int durationMinutes, const std::string& startTime,
                       long long priceVnd, std::string& error,
                       bool allStarts = false) const;
    bool updateFacilityHours(const std::string& requesterOwnerId,
                             const std::string& openTime,
                             const std::string& closeTime,
                             bool allows60Minutes,
                             bool allows90Minutes,
                             std::string& error) const;
    bool createField(const std::string& requesterOwnerId,
                     const VenueFieldRecord& field,
                     std::string& error) const;
    bool updateField(const std::string& requesterOwnerId,
                     const VenueFieldRecord& field,
                     std::string& error) const;
    bool deleteField(const std::string& requesterOwnerId,
                     const std::string& fieldId,
                     std::string& error) const;
    bool createBooking(const VenueBookingRequest& request,
                       std::string& bookingId,
                       std::string& error) const;

private:
    const VenueRepository& repository_;
};
