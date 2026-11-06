#pragma once

#include <string>
#include "Core/LinkedList.h"
#include "Tournament/Tournament.h"
#include "Tournament/Team.h"
#include "Tournament/TournamentMatch.h"

class FileManager;

// LÝ DO CHỌN: TournamentRepository quản lý việc lưu trữ và truy xuất các giải đấu,
// đội bóng và lịch thi đấu từ data/tournaments.txt, data/teams.txt, data/matches.txt.
class TournamentRepository {
private:
    FileManager& file_manager_;

public:
    explicit TournamentRepository(FileManager& fileManager);

    bool loadAll(LinkedList<Tournament>& tournaments, std::string& error) const;
    bool saveAll(const LinkedList<Tournament>& tournaments, std::string& error) const;
    bool save(const Tournament& tournament, std::string& error);
    bool update(const Tournament& tournament, std::string& error);
    bool remove(const std::string& tournamentId, std::string& error);
    bool findById(const std::string& tournamentId, Tournament& out, std::string& error) const;

    // Quản lý Đội bóng tham gia (data/teams.txt)
    bool loadTeams(LinkedList<Team>& out, std::string& error) const;
    bool saveTeams(const LinkedList<Team>& teams, std::string& error) const;
    bool saveTeam(const Team& team, std::string& error);

    // Quản lý Trận đấu của giải (data/matches.txt)
    bool loadMatches(LinkedList<TournamentMatch>& out, std::string& error) const;
    bool saveMatches(const LinkedList<TournamentMatch>& matches, std::string& error) const;
    bool saveMatch(const TournamentMatch& match, std::string& error);
};