#include <cassert>
#include <iostream>
#include <string>

#include "Core/DateTime.h"
#include "Core/Queue.h"
#include "Repository/BookingRepository.h"
#include "Repository/TournamentRepository.h"
#include "Repository/VenueRepository.h"
#include "Service/TournamentService.h"
#include "Storage/FileManager.h"
#include "Tournament/Tournament.h"

int main() {
    FileManager fileManager;
    TournamentRepository tournamentRepo(fileManager);
    BookingRepository bookingRepo(fileManager);
    TournamentService tournamentService(tournamentRepo, bookingRepo);

    // 1. Test loadAllTournaments
    LinkedList<Tournament> list;
    std::string err;
    assert(tournamentService.loadAllTournaments(list, err));
    assert(list.size() > 0);

    // 2. Test buildMatchScheduleQueue (uses self-implemented Queue)
    Queue<TournamentMatch> schedule = tournamentService.buildMatchScheduleQueue(
        "GD-TEST", "2026-11-20", "18:00", 4
    );
    assert(schedule.size() == 2);
    assert(!schedule.isEmpty());

    // 3. Test checkOverlap with existing tournament on field S7-01
    // GD-01 is 15/10/2026 - 20/10/2026, status = 2 (DangDienRa), slot: 17:30 - 21:00
    Tournament overlapTournament(
        "GD-99", "0999888777", "Giai Test Trung", "S7-01",
        "16/10/2026", "18/10/2026", "Test note",
        TournamentStatus::REQUESTED, "", "Test User", "18:00 - 20:00"
    );
    assert(!tournamentService.checkOverlap(overlapTournament, "GD-99", err));
    assert(!err.empty());

    // 4. Test non-overlapping tournament on a future date
    Tournament validTournament(
        "GD-VALID", "0999888777", "Giai Hop Le", "S7-01",
        "25/12/2026", "28/12/2026", "Test note",
        TournamentStatus::REQUESTED, "", "Test User", "18:00 - 20:00"
    );
    assert(tournamentService.checkOverlap(validTournament, "GD-VALID", err));

    std::cout << "ALL TOURNAMENT REPOSITORY & SERVICE TESTS PASSED!" << std::endl;
    return 0;
}
