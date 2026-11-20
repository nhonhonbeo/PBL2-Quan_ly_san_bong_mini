#include <cassert>
#include "Service/AvailabilityService.h"
int main() {
    using namespace AvailabilityService;
    assert(!overlap(60, 120, 120, 180));
    assert(overlap(60, 150, 120, 180));
    int start, end;
    assert(parseInterval("18:00 - 19:30", start, end));
    assert(start == 1080 && end == 1170);
    assert(!parseInterval("19:00 - 18:00", start, end));
    assert(!parseInterval("99:00 - 19:00", start, end));
    LinkedList<VenueBookingRecord> bookings;
    LinkedList<Tournament> tournaments;
    tournaments.push_back(Tournament("T1","C1","Cup","F1","2030-01-01","2030-01-03","",TournamentStatus::CONFIRMED,"","","18:00 - 20:00"));
    tournaments.push_back(Tournament("T2","C1","Pending","F1","2030-01-01","2030-01-03","",TournamentStatus::REQUESTED,"","","16:00 - 17:00"));
    LinkedList<Interval> occupied; std::string error, id;
    assert(collectDay(bookings,tournaments,"F1","2030-01-02",occupied,error));
    assert(hasConflict(occupied,1140,1200,id) && id == "T1");
    assert(!hasConflict(occupied,960,1020,id));
    assert(!hasConflict(occupied,1200,1260,id));
    assert(collectDay(bookings,tournaments,"F1","2030-01-04",occupied,error));
    assert(occupied.isEmpty());
}
