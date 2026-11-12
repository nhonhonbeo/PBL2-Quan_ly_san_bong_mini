#include "Repository/TournamentRepository.h"

#include <sstream>

#include "Storage/FileManager.h"

namespace {

void tachTruong(const std::string& line, LinkedList<std::string>& tokens) {
    tokens.clear();
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, '|')) {
        tokens.push_back(token);
    }
}

const std::string* layTruong(const LinkedList<std::string>& tokens, size_t index) {
    size_t i = 0;
    for (auto* node = tokens.get_head(); node != nullptr; node = node->next) {
        if (i == index) return &node->data;
        ++i;
    }
    return nullptr;
}

}  // namespace

TournamentRepository::TournamentRepository(FileManager& manager)
    : file_manager_(manager) {}

bool TournamentRepository::loadAll(LinkedList<Tournament>& tournaments,
                                   std::string& error) const {
    // LÃ DO CHá»ŒN: LinkedList náº¡p danh sÃ¡ch giáº£i Ä‘áº¥u tá»« táº­p tin data/tournaments.txt tuáº§n tá»±.
    LinkedList<std::string> lines;
    if (!file_manager_.readAllLines("data/tournaments.txt", lines, error)) {
        return false;
    }

    tournaments.clear();
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        if (parts.size() < 7) continue;

        const std::string* id = layTruong(parts, 0);
        const std::string* name = layTruong(parts, 1);
        const std::string* reqName = layTruong(parts, 2);
        const std::string* reqPhone = layTruong(parts, 3);
        const std::string* startDate = layTruong(parts, 4);
        const std::string* endDate = layTruong(parts, 5);
        const std::string* fieldId = layTruong(parts, 6);
        const std::string* timeSlot = layTruong(parts, 7);
        const std::string* note = layTruong(parts, 8);
        const std::string* rejectReason = layTruong(parts, 9);
        const std::string* statusStr = layTruong(parts, 10);

        TournamentStatus st = TournamentStatus::REQUESTED;
        if (statusStr) {
            try {
                int code = std::stoi(*statusStr);
                if (code >= 0 && code <= 4) {
                    st = static_cast<TournamentStatus>(code);
                }
            } catch (...) {}
        }

        Tournament t(
            id ? *id : "",
            reqPhone ? *reqPhone : "",
            name ? *name : "",
            fieldId ? *fieldId : "",
            startDate ? *startDate : "",
            endDate ? *endDate : "",
            note ? *note : "",
            st,
            "",
            reqName ? *reqName : "",
            timeSlot ? *timeSlot : "",
            rejectReason ? *rejectReason : ""
        );
        tournaments.push_back(t);
    }
    return true;
}

bool TournamentRepository::saveAll(const LinkedList<Tournament>& tournaments,
                                   std::string& error) const {
    // LÃ DO CHá»ŒN: LinkedList ná»‘i vÃ  xuáº¥t tá»‡p dÃ²ng giáº£i Ä‘áº¥u ra táº­p tin data/tournaments.txt.
    LinkedList<std::string> lines;
    for (auto* node = tournaments.get_head(); node != nullptr; node = node->next) {
        const Tournament& t = node->data;
        std::string line =
            FileManager::join(t.getTournamentId()) + "|" +
            FileManager::join(t.getName()) + "|" +
            FileManager::join(t.getRequesterName()) + "|" +
            FileManager::join(t.getRequesterUserId()) + "|" +
            FileManager::join(t.getStartDate()) + "|" +
            FileManager::join(t.getEndDate()) + "|" +
            FileManager::join(t.getRequestedFieldId()) + "|" +
            FileManager::join(t.getTimeSlot()) + "|" +
            FileManager::join(t.getNote()) + "|" +
            FileManager::join(t.getRejectReason()) + "|" +
            std::to_string(static_cast<int>(t.getStatus()));
        lines.push_back(line);
    }
    return file_manager_.writeAllLines("data/tournaments.txt", lines, error);
}

bool TournamentRepository::save(const Tournament& tournament, std::string& error) {
    LinkedList<Tournament> list;
    loadAll(list, error);
    list.push_back(tournament);
    return saveAll(list, error);
}

bool TournamentRepository::update(const Tournament& tournament, std::string& error) {
    LinkedList<Tournament> list;
    loadAll(list, error);
    bool found = false;
    for (auto* node = list.get_head(); node != nullptr; node = node->next) {
        if (node->data.getTournamentId() == tournament.getTournamentId()) {
            node->data = tournament;
            found = true;
            break;
        }
    }
    if (!found) {
        list.push_back(tournament);
    }
    return saveAll(list, error);
}

bool TournamentRepository::remove(const std::string& tournamentId, std::string& error) {
    LinkedList<Tournament> list;
    loadAll(list, error);
    LinkedList<Tournament> filtered;
    for (auto* node = list.get_head(); node != nullptr; node = node->next) {
        if (node->data.getTournamentId() != tournamentId) {
            filtered.push_back(node->data);
        }
    }
    return saveAll(filtered, error);
}

bool TournamentRepository::findById(const std::string& tournamentId,
                                    Tournament& out,
                                    std::string& error) const {
    LinkedList<Tournament> list;
    if (!loadAll(list, error)) return false;
    for (auto* node = list.get_head(); node != nullptr; node = node->next) {
        if (node->data.getTournamentId() == tournamentId) {
            out = node->data;
            return true;
        }
    }
    error = "Khong tim thay giai dau: " + tournamentId;
    return false;
}

bool TournamentRepository::loadTeams(LinkedList<Team>& out, std::string& error) const {
    out.clear();
    // LÝ DO CHỌN: LinkedList nạp danh sách đội bóng tham gia giải đấu từ data/teams.txt.
    LinkedList<std::string> lines;
    if (!file_manager_.readAllLines("data/teams.txt", lines, error)) {
        return false;
    }
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        if (parts.size() < 3) continue;
        const std::string* id = layTruong(parts, 0);
        const std::string* tourId = layTruong(parts, 1);
        const std::string* name = layTruong(parts, 2);
        const std::string* phone = layTruong(parts, 3);
        const std::string* countStr = layTruong(parts, 4);

        int count = 7;
        if (countStr) {
            try { count = std::stoi(*countStr); } catch (...) {}
        }
        Team t(id ? *id : "", tourId ? *tourId : "", name ? *name : "",
               phone ? *phone : "", count);
        out.push_back(t);
    }
    return true;
}

bool TournamentRepository::saveTeams(const LinkedList<Team>& teams, std::string& error) const {
    // LÝ DO CHỌN: LinkedList nối và xuất tập danh sách đội bóng ra data/teams.txt.
    LinkedList<std::string> lines;
    for (auto* node = teams.get_head(); node != nullptr; node = node->next) {
        const Team& t = node->data;
        std::string line =
            FileManager::join(t.getTeamId()) + "|" +
            FileManager::join(t.getTournamentId()) + "|" +
            FileManager::join(t.getName()) + "|" +
            FileManager::join(t.getContactPhone()) + "|" +
            std::to_string(t.getMemberCount());
        lines.push_back(line);
    }
    return file_manager_.writeAllLines("data/teams.txt", lines, error);
}

bool TournamentRepository::saveTeam(const Team& team, std::string& error) {
    LinkedList<Team> list;
    loadTeams(list, error);
    bool found = false;
    for (auto* node = list.get_head(); node != nullptr; node = node->next) {
        if (node->data.getTeamId() == team.getTeamId()) {
            node->data = team;
            found = true;
            break;
        }
    }
    if (!found) {
        list.push_back(team);
    }
    return saveTeams(list, error);
}

bool TournamentRepository::loadMatches(LinkedList<TournamentMatch>& out, std::string& error) const {
    out.clear();
    // LÝ DO CHỌN: LinkedList nạp danh sách lịch thi đấu từ data/matches.txt tuần tự.
    LinkedList<std::string> lines;
    if (!file_manager_.readAllLines("data/matches.txt", lines, error)) {
        return false;
    }
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        if (parts.size() < 7) continue;
        const std::string* id = layTruong(parts, 0);
        const std::string* tourId = layTruong(parts, 1);
        const std::string* home = layTruong(parts, 2);
        const std::string* away = layTruong(parts, 3);
        const std::string* roundStr = layTruong(parts, 4);
        const std::string* date = layTruong(parts, 5);
        const std::string* time = layTruong(parts, 6);
        const std::string* homeScStr = layTruong(parts, 7);
        const std::string* awayScStr = layTruong(parts, 8);
        const std::string* playedStr = layTruong(parts, 9);

        int round = roundStr ? std::stoi(*roundStr) : 1;
        int homeSc = homeScStr ? std::stoi(*homeScStr) : 0;
        int awaySc = awayScStr ? std::stoi(*awayScStr) : 0;
        bool played = playedStr ? (*playedStr == "1" || *playedStr == "true") : false;

        TournamentMatch m(id ? *id : "", tourId ? *tourId : "",
                          home ? *home : "", away ? *away : "",
                          round, date ? *date : "", time ? *time : "",
                          homeSc, awaySc, played);
        out.push_back(m);
    }
    return true;
}

bool TournamentRepository::saveMatches(const LinkedList<TournamentMatch>& matches, std::string& error) const {
    // LÝ DO CHỌN: LinkedList nối và xuất tập danh sách trận đấu ra data/matches.txt.
    LinkedList<std::string> lines;
    for (auto* node = matches.get_head(); node != nullptr; node = node->next) {
        const TournamentMatch& m = node->data;
        std::string line =
            FileManager::join(m.getMatchId()) + "|" +
            FileManager::join(m.getTournamentId()) + "|" +
            FileManager::join(m.getHomeTeamId()) + "|" +
            FileManager::join(m.getAwayTeamId()) + "|" +
            std::to_string(m.getRound()) + "|" +
            FileManager::join(m.getDate()) + "|" +
            FileManager::join(m.getStartTime()) + "|" +
            std::to_string(m.getHomeScore()) + "|" +
            std::to_string(m.getAwayScore()) + "|" +
            (m.isPlayed() ? "1" : "0");
        lines.push_back(line);
    }
    return file_manager_.writeAllLines("data/matches.txt", lines, error);
}

bool TournamentRepository::saveMatch(const TournamentMatch& match, std::string& error) {
    LinkedList<TournamentMatch> list;
    loadMatches(list, error);
    bool found = false;
    for (auto* node = list.get_head(); node != nullptr; node = node->next) {
        if (node->data.getMatchId() == match.getMatchId()) {
            node->data = match;
            found = true;
            break;
        }
    }
    if (!found) {
        list.push_back(match);
    }
    return saveMatches(list, error);
}