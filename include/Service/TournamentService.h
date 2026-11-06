#pragma once

#include <string>
#include "Core/LinkedList.h"
#include "Core/Queue.h"
#include "Tournament/Tournament.h"
#include "Tournament/Team.h"
#include "Tournament/TournamentMatch.h"

class TournamentRepository;
class BookingRepository;

// LÝ DO CHỌN: TournamentService là điểm tập trung xử lý toàn bộ quy tắc giải đấu,
// đội thi đấu, lịch và tỷ số trận, đặc biệt là kiểm tra trùng lịch sân bóng.
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

    // Quản lý Đội bóng & Lịch thi đấu
    bool registerTeam(const std::string& tournamentId, const std::string& name,
                      const std::string& phone, std::string& outTeamId,
                      std::string& error);
    bool getTeamsByTournament(const std::string& tournamentId,
                              LinkedList<Team>& out,
                              std::string& error) const;
    bool scheduleMatch(const std::string& tournamentId,
                       const std::string& homeTeamId,
                       const std::string& awayTeamId,
                       int round, const std::string& date,
                       const std::string& startTime,
                       std::string& outMatchId,
                       std::string& error);
    bool getMatchesByTournament(const std::string& tournamentId,
                                LinkedList<TournamentMatch>& out,
                                std::string& error) const;
    bool recordMatchScore(const std::string& matchId, int homeScore,
                          int awayScore, std::string& error);
};