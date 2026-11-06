#include "Tournament/TournamentMatch.h"

TournamentMatch::TournamentMatch()
    : round_(1), home_score_(0), away_score_(0), played_(false) {}

TournamentMatch::TournamentMatch(const std::string& id, const std::string& t,
                                 const std::string& h, const std::string& a,
                                 int round, const std::string& date,
                                 const std::string& time,
                                 int homeScore, int awayScore, bool played)
    : match_id_(id), tournament_id_(t), home_team_id_(h), away_team_id_(a),
      round_(round), date_(date), start_time_(time),
      home_score_(homeScore), away_score_(awayScore), played_(played) {}

const std::string& TournamentMatch::getMatchId() const { return match_id_; }
const std::string& TournamentMatch::getTournamentId() const { return tournament_id_; }
const std::string& TournamentMatch::getHomeTeamId() const { return home_team_id_; }
const std::string& TournamentMatch::getAwayTeamId() const { return away_team_id_; }
int TournamentMatch::getRound() const { return round_; }
const std::string& TournamentMatch::getDate() const { return date_; }
const std::string& TournamentMatch::getStartTime() const { return start_time_; }
int TournamentMatch::getHomeScore() const { return home_score_; }
int TournamentMatch::getAwayScore() const { return away_score_; }
bool TournamentMatch::isPlayed() const { return played_; }

void TournamentMatch::setResult(int homeScore, int awayScore) {
    home_score_ = homeScore;
    away_score_ = awayScore;
    played_ = true;
}

std::string TournamentMatch::getWinnerTeamId() const {
    if (!played_) return "";
    if (home_score_ > away_score_) return home_team_id_;
    if (away_score_ > home_score_) return away_team_id_;
    return ""; // Hòa
}

bool TournamentMatch::isDraw() const {
    return played_ && (home_score_ == away_score_);
}
