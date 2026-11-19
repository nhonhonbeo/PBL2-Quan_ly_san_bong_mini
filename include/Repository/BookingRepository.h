#pragma once
#include <string>
#include "Core/LinkedList.h"
#include "Repository/VenueRepository.h"
class Booking; class FileManager;
// LÝ DO CHỌN: Repository cô lập tuần tự hóa Booking khỏi quy tắc kiểm tra trùng lịch.
class BookingRepository {
    VenueRepository& venue_repository_;
public:
    explicit BookingRepository(VenueRepository& venueRepository);
    virtual ~BookingRepository() = default;
    static Booking toDomain(const VenueBookingRecord& record);
    static VenueBookingRecord toRecord(const Booking& booking);
    virtual bool save(const Booking& booking, std::string& error);
    virtual bool update(const Booking& booking, std::string& error);
    virtual bool findById(const std::string& bookingId, Booking& out, std::string& error) const;
    virtual bool loadAll(LinkedList<VenueBookingRecord>& bookings,
                 std::string& error) const;
    virtual bool saveAll(const LinkedList<VenueBookingRecord>& bookings,
                 std::string& error) const;
};
