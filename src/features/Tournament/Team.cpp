#include "Tournament/Team.h"

Team::Team() : member_count_(7) {}

Team::Team(const std::string& id, const std::string& tournamentId, const std::string& name,
           const std::string& contactPhone, int memberCount)
    : team_id_(id), tournament_id_(tournamentId), name_(name),
      contact_phone_(contactPhone), member_count_(memberCount) {}

const std::string& Team::getTeamId() const { return team_id_; }
const std::string& Team::getTournamentId() const { return tournament_id_; }
const std::string& Team::getName() const { return name_; }
const std::string& Team::getContactPhone() const { return contact_phone_; }
int Team::getMemberCount() const { return member_count_; }

void Team::setName(const std::string& name) { name_ = name; }
void Team::setContactPhone(const std::string& phone) { contact_phone_ = phone; }
void Team::setMemberCount(int count) { member_count_ = count; }

bool Team::isValid() const {
    return !team_id_.empty() && !tournament_id_.empty() && !name_.empty() && member_count_ >= 5;
}
