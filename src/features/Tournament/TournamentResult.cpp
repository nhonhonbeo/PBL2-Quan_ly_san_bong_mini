#include "Tournament/TournamentResult.h"
#include <cstdlib>

TournamentResult::TournamentResult() : home_score_(0), away_score_(0) {}

TournamentResult::TournamentResult(const std::string& id, int home, int away)
    : match_id_(id), home_score_(home), away_score_(away) {}

const std::string& TournamentResult::getMatchId() const { return match_id_; }
int TournamentResult::getHomeScore() const { return home_score_; }
int TournamentResult::getAwayScore() const { return away_score_; }

int TournamentResult::getGoalDifference() const {
    return std::abs(home_score_ - away_score_);
}

bool TournamentResult::isDraw() const {
    return home_score_ == away_score_;
}

bool TournamentResult::isHomeWin() const {
    return home_score_ > away_score_;
}

bool TournamentResult::isAwayWin() const {
    return away_score_ > home_score_;
}
