#pragma once
#include "Core/Queue.h"
#include "Tournament/TournamentMatch.h"
#include <string>
class Tournament; class TournamentRepository;
class TournamentService {
    TournamentRepository& tournament_repository_;
public:
    explicit TournamentService(TournamentRepository& tournamentRepository);
    bool requestTournament(const Tournament& tournament, std::string& error);
    bool confirmTournament(Tournament& tournament, const std::string& bookingId, std::string& error);
    Queue<TournamentMatch> buildMatchScheduleQueue(const std::string& tournamentId, const std::string& date, const std::string& firstStart, int teamCount) const;
};
