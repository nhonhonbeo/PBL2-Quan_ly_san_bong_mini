#include "Repository/ReportRepository.h"

#include "Repository/VenueRepository.h"

ReportRepository::ReportRepository(VenueRepository& venueRepository)
    : venue_repository_(venueRepository) {}

bool ReportRepository::loadFields(
    LinkedList<ReportFieldRecord>& fields,
    std::string& error
) const {
    LinkedList<VenueFieldRecord> venueFields;
    LinkedList<VenuePriceRecord> prices;
    if (!venue_repository_.loadFields(venueFields, error) ||
        !venue_repository_.loadPrices(prices, error)) return false;
    fields.clear();
    for (auto* node = venueFields.get_head(); node != nullptr;
         node = node->next) {
        const VenueFieldRecord& record = node->data;
        ReportFieldRecord field;
        field.fieldId = record.fieldId;
        field.ownerUserId = record.ownerId;
        field.name = record.name;
        field.size = record.size;
        field.underMaintenance = record.status != 0;
        field.rules = record.rules;
        for (auto* priceNode = prices.get_head(); priceNode != nullptr;
             priceNode = priceNode->next) {
            const VenuePriceRecord& price = priceNode->data;
            if (price.ownerId != record.ownerId ||
                price.fieldSize != record.size || price.durationMinutes <= 0) {
                continue;
            }
            const long long hourlyPrice =
                price.priceVnd * 60 / price.durationMinutes;
            if (field.hourlyRateVnd == 0 ||
                hourlyPrice < field.hourlyRateVnd) {
                field.hourlyRateVnd = hourlyPrice;
            }
        }
        fields.push_back(field);
    }
    return true;
}

bool ReportRepository::saveFields(
    const LinkedList<ReportFieldRecord>& fields,
    std::string& error
) const {
    LinkedList<VenueFieldRecord> records;
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        const ReportFieldRecord& field = node->data;
        records.push_back({field.fieldId, field.ownerUserId, field.name,
                           field.size, field.underMaintenance ? 2 : 0,
                           field.rules});
    }
    return venue_repository_.saveFields(records, error);
}

bool ReportRepository::loadBookings(
    LinkedList<ReportBookingRecord>& bookings,
    std::string& error
) const {
    LinkedList<VenueBookingRecord> venueBookings;
    if (!venue_repository_.loadBookings(venueBookings, error)) return false;
    bookings.clear();
    for (auto* node = venueBookings.get_head(); node != nullptr;
         node = node->next) {
        const VenueBookingRecord& record = node->data;
        ReportBookingRecord booking;
        booking.bookingId = record.bookingId;
        booking.fieldId = record.fieldId;
        booking.ownerUserId = record.ownerId;
        booking.customerId = record.customerId;
        booking.customerName = record.customerName;
        booking.customerPhone = record.customerPhone;
        booking.date = record.date;
        booking.startTime = record.startTime;
        booking.endTime = record.endTime;
        booking.durationMinutes = record.durationMinutes;
        booking.timeSlot = booking.startTime + " - " + booking.endTime;
        booking.amountVnd = record.priceVnd;
        booking.status = record.status;
        bookings.push_back(booking);
    }
    return true;
}

bool ReportRepository::saveBookings(
    const LinkedList<ReportBookingRecord>& bookings,
    std::string& error
) const {
    LinkedList<VenueBookingRecord> records;
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        const ReportBookingRecord& booking = node->data;
        records.push_back({booking.bookingId, booking.fieldId,
                           booking.ownerUserId, booking.customerId,
                           booking.customerName, booking.customerPhone,
                           booking.date, booking.startTime, booking.endTime,
                           booking.durationMinutes, booking.amountVnd,
                           booking.status});
    }
    return venue_repository_.saveBookings(records, error);
}
