#pragma once
#include <string>

// LÝ DO CHỌN: TournamentMatch đại diện cho một trận đấu giữa hai đội bóng trong khuôn khổ giải.
class TournamentMatch {
private:
    std::string match_id_;
    std::string tournament_id_;
    std::string home_team_id_;
    std::string away_team_id_;
    int round_;
    std::string date_;
    std::string start_time_;
    int home_score_;
    int away_score_;
    bool played_;

public:
    TournamentMatch();
    TournamentMatch(const std::string& id, const std::string& tournamentId,
                    const std::string& homeTeamId, const std::string& awayTeamId,
                    int round, const std::string& date, const std::string& startTime,
                    int homeScore = 0, int awayScore = 0, bool played = false);

    const std::string& getMatchId() const;
    const std::string& getTournamentId() const;
    const std::string& getHomeTeamId() const;
    const std::string& getAwayTeamId() const;
    int getRound() const;
    const std::string& getDate() const;
    const std::string& getStartTime() const;
    int getHomeScore() const;
    int getAwayScore() const;
    bool isPlayed() const;

    void setResult(int homeScore, int awayScore);
    std::string getWinnerTeamId() const;
    bool isDraw() const;
};
