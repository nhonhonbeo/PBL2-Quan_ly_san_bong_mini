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
    // LÝ DO CHỌN: LinkedList nạp danh sách giải đấu từ tập tin data/tournaments.txt tuần tự.
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
    // LÝ DO CHỌN: LinkedList nối và xuất tệp dòng giải đấu ra tập tin data/tournaments.txt.
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
