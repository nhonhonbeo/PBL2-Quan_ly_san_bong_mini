#include "Repository/ReportRepository.h"

#include "Repository/VenueRepository.h"
#include "Storage/FileManager.h"

bool ReportRepository::loadFields(
    LinkedList<ReportFieldRecord>& fields,
    std::string& error
) const {
    VenueRepository venueRepository;
    LinkedList<VenueFieldRecord> venueFields;
    LinkedList<VenuePriceRecord> prices;
    if (!venueRepository.loadFields(venueFields, error) ||
        !venueRepository.loadPrices(prices, error)) return false;
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
    FileManager fileManager;
    LinkedList<std::string> lines;
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        const ReportFieldRecord& field = node->data;
        lines.push_back(
            FileManager::join(field.fieldId) + "|" +
            FileManager::join(field.ownerUserId) + "|" +
            FileManager::join(field.name) + "|" +
            std::to_string(field.size) + "|" +
            (field.underMaintenance ? "MAINTENANCE" : "ACTIVE") + "|" +
            FileManager::join(field.rules)
        );
    }
    return fileManager.writeAllLines("data/fields.txt", lines, error);
}

bool ReportRepository::loadBookings(
    LinkedList<ReportBookingRecord>& bookings,
    std::string& error
) const {
    VenueRepository venueRepository;
    LinkedList<VenueBookingRecord> venueBookings;
    if (!venueRepository.loadBookings(venueBookings, error)) return false;
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
    FileManager fileManager;
    LinkedList<std::string> lines;
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        const ReportBookingRecord& booking = node->data;
        lines.push_back(
            FileManager::join(booking.bookingId) + "|" +
            FileManager::join(booking.fieldId) + "|" +
            FileManager::join(booking.ownerUserId) + "|" +
            FileManager::join(booking.customerId) + "|" +
            FileManager::join(booking.customerName) + "|" +
            FileManager::join(booking.customerPhone) + "|" +
            FileManager::join(booking.date) + "|" +
            FileManager::join(booking.startTime) + "|" +
            FileManager::join(booking.endTime) + "|" +
            std::to_string(booking.durationMinutes) + "|" +
            std::to_string(booking.amountVnd) + "|" +
            FileManager::join(booking.status)
        );
    }
    return fileManager.writeAllLines("data/bookings.txt", lines, error);
}
