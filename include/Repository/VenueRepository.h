#pragma once

#include <string>

#include "Core/LinkedList.h"
#include "Tournament/Tournament.h"

class FileManager;

struct VenueFacilityRecord {
    std::string ownerId;
    std::string name;
    std::string address;
    std::string phone;
    std::string openTime;
    std::string closeTime;
    bool allows60Minutes = true;
    bool allows90Minutes = true;
};

struct VenueFieldRecord {
    std::string fieldId;
    std::string ownerId;
    std::string name;
    int size = 5;
    int status = 0;
    std::string rules;
};

struct VenuePriceRecord {
    std::string ownerId;
    int fieldSize = 5;
    int durationMinutes = 60;
    std::string startTime;
    long long priceVnd = 0;
};

struct VenueBookingRecord {
    std::string bookingId;
    std::string fieldId;
    std::string ownerId;
    std::string customerId;
    std::string customerName;
    std::string customerPhone;
    std::string date;
    std::string startTime;
    std::string endTime;
    int durationMinutes = 60;
    long long priceVnd = 0;
    std::string status;
};

// LÝ DO CHỌN: Tách dữ liệu đường dẫn ảnh khỏi GUI để Repository quản lý lưu trữ.
struct VenueFieldImageRecord {
    std::string fieldId;
    std::string path;
};

// LÝ DO CHỌN: Mỗi bản ghi có thể lưu trong LinkedList, còn các chỉ mục tra
// cứu được dựng lại từ dữ liệu bền vững để tránh lưu trùng trạng thái.
class VenueRepository {
private:
    FileManager& file_manager_;

public:
    explicit VenueRepository(FileManager& fileManager);

    bool loadFacilities(LinkedList<VenueFacilityRecord>& records,
                        std::string& error) const;
    bool loadFields(LinkedList<VenueFieldRecord>& records,
                    std::string& error) const;
    bool saveFields(const LinkedList<VenueFieldRecord>& records,
                    std::string& error) const;
    bool loadPrices(LinkedList<VenuePriceRecord>& records,
                    std::string& error) const;
    bool savePrices(const LinkedList<VenuePriceRecord>& records,
                    std::string& error) const;
    bool updateFacilityHours(const std::string& ownerId,
                             const std::string& openTime,
                             const std::string& closeTime,
                             bool allows60Minutes,
                             bool allows90Minutes,
                             std::string& error) const;
    bool loadTournaments(LinkedList<Tournament>& records, std::string& error) const;
    bool loadBookings(LinkedList<VenueBookingRecord>& records,
                      std::string& error) const;
    bool saveBookings(const LinkedList<VenueBookingRecord>& records,
                      std::string& error) const;
    bool loadFieldImages(LinkedList<VenueFieldImageRecord>& records,
                         std::string& error) const;
    bool saveFieldImages(const LinkedList<VenueFieldImageRecord>& records,
                         std::string& error) const;
    bool migrateLegacyData(std::string& error) const;
    bool seedIfNeeded(std::string& error) const;
};
