#pragma once
#include <string>

// LÝ DO CHỌN: TournamentResult ghi nhận kết quả chi tiết của trận đấu bóng đá.
class TournamentResult {
private:
    std::string match_id_;
    int home_score_;
    int away_score_;

public:
    TournamentResult();
    TournamentResult(const std::string& matchId, int homeScore, int awayScore);

    const std::string& getMatchId() const;
    int getHomeScore() const;
    int getAwayScore() const;

    int getGoalDifference() const;
    bool isDraw() const;
    bool isHomeWin() const;
    bool isAwayWin() const;
};
