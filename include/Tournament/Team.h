#pragma once
#include <string>
class Team {
    std::string team_id_, tournament_id_, name_;
public:
    Team() = default;
    Team(const std::string& id, const std::string& tournamentId, const std::string& name) : team_id_(id), tournament_id_(tournamentId), name_(name) {}
    const std::string& getTeamId() const { return team_id_; }
    const std::string& getTournamentId() const { return tournament_id_; }
    const std::string& getName() const { return name_; }
};
