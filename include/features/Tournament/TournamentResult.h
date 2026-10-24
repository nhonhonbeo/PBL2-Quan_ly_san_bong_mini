#pragma once
#include <string>
class TournamentResult {
    std::string match_id_; int home_score_, away_score_;
public:
    TournamentResult(); TournamentResult(const std::string&, int, int);
    const std::string& getMatchId() const; int getHomeScore() const; int getAwayScore() const;
};
