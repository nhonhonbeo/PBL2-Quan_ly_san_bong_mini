#pragma once
#include <string>

// LÝ DO CHỌN: Team đại diện cho một đội bóng tham gia giải đấu cụ thể.
class Team {
private:
    std::string team_id_;
    std::string tournament_id_;
    std::string name_;
    std::string contact_phone_;
    int member_count_;

public:
    Team();
    Team(const std::string& id, const std::string& tournamentId, const std::string& name,
         const std::string& contactPhone = "", int memberCount = 7);

    const std::string& getTeamId() const;
    const std::string& getTournamentId() const;
    const std::string& getName() const;
    const std::string& getContactPhone() const;
    int getMemberCount() const;

    void setName(const std::string& name);
    void setContactPhone(const std::string& phone);
    void setMemberCount(int count);

    bool isValid() const;
};
