#pragma once
#include <string>
#include "Core/LinkedList.h"
#include "Repository/VenueRepository.h"
class Booking; class FileManager;
// LÝ DO CHỌN: Repository cô lập tuần tự hóa Booking khỏi quy tắc kiểm tra trùng lịch.
class BookingRepository {
    FileManager& file_manager_;
public:
    explicit BookingRepository(FileManager& fileManager);
    virtual ~BookingRepository() = default;
    virtual bool save(const Booking& booking, std::string& error);
    virtual bool update(const Booking& booking, std::string& error);
    virtual bool findById(const std::string& bookingId, Booking& out, std::string& error) const;
    virtual bool loadAll(LinkedList<VenueBookingRecord>& bookings,
                 std::string& error) const;
    virtual bool saveAll(const LinkedList<VenueBookingRecord>& bookings,
                 std::string& error) const;
};
