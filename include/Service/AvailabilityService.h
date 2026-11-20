#pragma once
#include <string>
#include "Core/DateTime.h"
#include "Core/LinkedList.h"
#include "Repository/VenueRepository.h"
#include "Booking/Booking.h"

// A read-only, day-specific reservation index shared by availability queries
// and booking creation. No duplication of persisted state.
namespace AvailabilityService {
struct Interval { int start; int end; std::string id; };
inline int minutes(const std::string& time) {
    if (!DateTime::isValidTime(time)) return -1;
    return std::stoi(time.substr(0,2))*60 + std::stoi(time.substr(3,2));
}
inline bool overlap(int a, int b, int c, int d) { return a < d && c < b; }
inline bool reservesTime(TournamentStatus s) {
    return s == TournamentStatus::CONFIRMED || s == TournamentStatus::IN_PROGRESS || s == TournamentStatus::FINISHED;
}
inline bool parseInterval(const std::string& text, int& start, int& end) {
    auto sep = text.find(" - ");
    if (sep == std::string::npos) return false;
    start = minutes(text.substr(0,sep)); end = minutes(text.substr(sep+3));
    return start >= 0 && end > start;
}
inline bool collectDay(const LinkedList<VenueBookingRecord>& bookings,
                       const LinkedList<Tournament>& tournaments,
                       const std::string& field, const std::string& day,
                       LinkedList<Interval>& out, std::string& error) {
    out.clear();
    for (auto* n = bookings.get_head(); n; n = n->next) {
        const auto& b = n->data;
        if (b.fieldId != field || Booking::textToStatus(b.status) == BookingStatus::CANCELLED) continue;
        auto date = DateTime::toIsoDate(b.date);
        if (date.empty()) { error = "Invalid booking date"; return false; }
        if (date != day) continue;
        int s = minutes(b.startTime), e = minutes(b.endTime);
        if (s < 0 || e <= s) { error = "Invalid booking interval"; return false; }
        out.push_back({s,e,b.bookingId});
    }
    for (auto* n = tournaments.get_head(); n; n = n->next) {
        const auto& t = n->data;
        if (t.getRequestedFieldId() != field || !reservesTime(t.getStatus())) continue;
        auto first = DateTime::toIsoDate(t.getStartDate()), last = DateTime::toIsoDate(t.getEndDate());
        if (first.empty() || last.empty() || first > last) { error = "Invalid tournament dates"; return false; }
        if (day < first || day > last) continue;
        int s, e;
        if (!parseInterval(t.getTimeSlot(),s,e)) { error = "Invalid tournament interval"; return false; }
        out.push_back({s,e,t.getTournamentId()});
    }
    return true;
}
inline bool hasConflict(const LinkedList<Interval>& reservations, int start, int end, std::string& id) {
    for (auto* n = reservations.get_head(); n; n = n->next) {
        if (overlap(start,end,n->data.start,n->data.end)) { id = n->data.id; return true; }
    }
    return false;
}
} // namespace AvailabilityService
