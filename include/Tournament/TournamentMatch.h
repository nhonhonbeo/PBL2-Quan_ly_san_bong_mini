#pragma once
#include <string>
class TournamentMatch {
    std::string match_id_, tournament_id_, home_team_id_, away_team_id_, date_, start_time_;
    int round_;
public:
    TournamentMatch();
    TournamentMatch(const std::string&, const std::string&, const std::string&, const std::string&, int, const std::string&, const std::string&);
    const std::string& getMatchId() const; const std::string& getTournamentId() const;
    const std::string& getHomeTeamId() const; const std::string& getAwayTeamId() const;
    int getRound() const; const std::string& getDate() const; const std::string& getStartTime() const;
};
