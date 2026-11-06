#pragma once

#include <string>
#include "Core/LinkedList.h"
#include "Core/Queue.h"
#include "Tournament/Tournament.h"
#include "Tournament/TournamentMatch.h"

class TournamentRepository;
class BookingRepository;

// L? DO CH?N: TournamentService l? ?i?m t?p trung x? l? to?n b? quy t?c gi?i ??u,
// ??c bi?t l? ki?m tra tr?ng l?ch s?n b?ng v?i c?c gi?i kh?c v? ??n ??t s?n c?n hi?u l?c.
class TournamentService {
private:
    TournamentRepository& tournament_repository_;
    BookingRepository& booking_repository_;

public:
    TournamentService(TournamentRepository& tournamentRepository,
                      BookingRepository& bookingRepository);

    bool requestTournament(const Tournament& tournament, std::string& error);
    bool confirmTournament(Tournament& tournament, const std::string& bookingId, std::string& error);
    Queue<TournamentMatch> buildMatchScheduleQueue(const std::string& tournamentId,
                                                   const std::string& date,
                                                   const std::string& firstStart,
                                                   int teamCount) const;

    bool loadAllTournaments(LinkedList<Tournament>& tournaments, std::string& error) const;
    bool getTournamentById(const std::string& tournamentId, Tournament& out, std::string& error) const;
    bool duyetGiaiDau(const std::string& tournamentId, std::string& error);
    bool tuChoiGiaiDau(const std::string& tournamentId, const std::string& reason, std::string& error);
    bool taoGiaiDauThuCong(const Tournament& tournament, std::string& outId, std::string& error);
    bool suaGiaiDau(const Tournament& tournament, std::string& error);
    bool huyGiaiDau(const std::string& tournamentId, std::string& error);
    bool guiYeuCauTaoGiai(const Tournament& tournament, std::string& outId, std::string& error);
    bool checkOverlap(const Tournament& tournament, const std::string& excludeId, std::string& error) const;
};
