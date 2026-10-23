#include "Service/TournamentService.h"
#include "Repository/TournamentRepository.h"
#include "Tournament/Tournament.h"
#include <cstdio>
TournamentService::TournamentService(TournamentRepository& repository) : tournament_repository_(repository) {}
bool TournamentService::requestTournament(const Tournament& tournament, std::string& error) { return tournament_repository_.save(tournament, error); }
bool TournamentService::confirmTournament(Tournament& tournament, const std::string& bookingId, std::string& error) { tournament.confirm(bookingId); return tournament_repository_.update(tournament, error); }
Queue<TournamentMatch> TournamentService::buildMatchScheduleQueue(const std::string& tournamentId, const std::string& date, const std::string& firstStart, int teamCount) const { Queue<TournamentMatch> result; for (int i = 0; i + 1 < teamCount; i += 2) { char id[32]; std::snprintf(id, sizeof(id), "%s-M%02d", tournamentId.c_str(), i / 2 + 1); result.enqueue(TournamentMatch(id, tournamentId, "T" + std::to_string(i + 1), "T" + std::to_string(i + 2), 1, date, firstStart)); } return result; }
