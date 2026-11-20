#include "Repository/TournamentRepository.h"

#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "Storage/FileManager.h"

namespace {

void tachTruong(const std::string& line, LinkedList<std::string>& tokens) {
    tokens.clear();
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, '|')) {
        tokens.push_back(token);
    }
    if (!line.empty() && line.back() == '|') tokens.push_back("");
}

const std::string* layTruong(const LinkedList<std::string>& tokens, size_t index) {
    size_t i = 0;
    for (auto* node = tokens.get_head(); node != nullptr; node = node->next) {
        if (i == index) return &node->data;
        ++i;
    }
    return nullptr;
}

bool tachSoKhongAm(const std::string& value, int& result) {
    if (value.empty()) return false;
    for (const char digit : value) {
        if (digit < '0' || digit > '9') return false;
    }
    try {
        size_t read = 0;
        const long long parsed = std::stoll(value, &read);
        if (read != value.size() ||
            parsed > std::numeric_limits<int>::max()) {
            return false;
        }
        result = static_cast<int>(parsed);
        return true;
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }
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

    LinkedList<Tournament> loaded;
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        if (parts.size() < 7 || parts.size() > 11) {
            error = "Bản ghi giải đấu trong data/tournaments.txt sai số trường.";
            return false;
        }

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
        if (id == nullptr || name == nullptr || reqName == nullptr ||
            reqPhone == nullptr || startDate == nullptr || endDate == nullptr ||
            fieldId == nullptr) {
            error = "Bản ghi giải đấu trong data/tournaments.txt thiếu trường.";
            return false;
        }
        if (statusStr != nullptr) {
            int code = 0;
            if (!tachSoKhongAm(*statusStr, code) || code > 4) {
                error = "Trạng thái giải đấu trong data/tournaments.txt không hợp lệ.";
                return false;
            }
            st = static_cast<TournamentStatus>(code);
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
        loaded.push_back(t);
    }
    tournaments = std::move(loaded);
    return true;
}

bool TournamentRepository::saveAll(const LinkedList<Tournament>& tournaments,
                                   std::string& error) const {
    // LÃ DO CHá»ŒN: LinkedList ná»‘i vÃ  xuáº¥t tá»‡p dÃ²ng giáº£i Ä‘áº¥u ra táº­p tin data/tournaments.txt.
    LinkedList<std::string> lines;
    for (auto* node = tournaments.get_head(); node != nullptr; node = node->next) {
        const Tournament& t = node->data;
        const int status = static_cast<int>(t.getStatus());
        if (status < 0 || status > 4) {
            error = "Không thể lưu giải đấu có trạng thái không hợp lệ.";
            return false;
        }
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
            std::to_string(status);
        lines.push_back(line);
    }
    return file_manager_.writeAllLines("data/tournaments.txt", lines, error);
}

bool TournamentRepository::save(const Tournament& tournament, std::string& error) {
    LinkedList<Tournament> list;
    if (!loadAll(list, error)) return false;
    if (tournament.getTournamentId().empty()) { error = "Tournament ID is required"; return false; }
    for (auto* n = list.get_head(); n; n = n->next) {
        if (n->data.getTournamentId() == tournament.getTournamentId()) { error = "Duplicate tournament ID"; return false; }
    }
    list.push_back(tournament);
    return saveAll(list, error);
}

bool TournamentRepository::update(const Tournament& tournament, std::string& error) {
    LinkedList<Tournament> list;
    if (!loadAll(list, error)) return false;
    const bool found = list.replaceFirstIf(
        [&tournament](const Tournament& existing) {
            return existing.getTournamentId() == tournament.getTournamentId();
        },
        tournament);
    if (!found) {
        error = "Tournament not found"; return false;
    }
    return saveAll(list, error);
}

bool TournamentRepository::remove(const std::string& tournamentId, std::string& error) {
    LinkedList<Tournament> list;
    if (!loadAll(list, error)) return false;
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
