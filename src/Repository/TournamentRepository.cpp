#include "Repository/TournamentRepository.h"
#include "Storage/FileManager.h"
#include "Tournament/Tournament.h"
#include "Core/LinkedList.h"
TournamentRepository::TournamentRepository(FileManager& manager) : file_manager_(manager) {}
bool TournamentRepository::save(const Tournament& tournament, std::string& error) { return file_manager_.appendLine("data/tournaments.txt", tournament.getTournamentId() + "|" + tournament.getRequesterUserId() + "|" + FileManager::join(tournament.getName()) + "|" + tournament.getRequestedFieldId() + "|" + tournament.getStartDate() + "|" + tournament.getEndDate() + "|" + std::to_string(static_cast<int>(tournament.getStatus())) + "|" + FileManager::join(tournament.getNote()) + "|" + tournament.getConfirmedBookingId(), error); }
bool TournamentRepository::update(const Tournament& tournament, std::string& error) { return save(tournament, error); }
