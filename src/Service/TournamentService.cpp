#include "Service/TournamentService.h"

#include <cstdio>
#include <string>

#include "Core/DateTime.h"
#include "Repository/BookingRepository.h"
#include "Repository/TournamentRepository.h"

namespace {

bool tachKhung(const std::string& text, std::string& start, std::string& end) {
    const std::size_t sep = text.find(" - ");
    if (sep == std::string::npos) return false;
    start = text.substr(0, sep);
    end = text.substr(sep + 3);
    return true;
}

bool thoiGianTrungNhau(const std::string& s1, const std::string& e1,
                       const std::string& s2, const std::string& e2) {
    if (s1.empty() || e1.empty() || s2.empty() || e2.empty()) return true;
    return (s1 < e2 && e1 > s2);
}

// LÝ DO CHỌN: Sinh mã theo max sequence để không bao giờ bị trùng lặp ID khi có bản ghi bị xóa.
int timMaxSeq(const LinkedList<std::string>& ids, const std::string& prefix) {
    int maxSeq = 0;
    for (auto* n = ids.get_head(); n != nullptr; n = n->next) {
        const std::string& id = n->data;
        if (id.rfind(prefix, 0) == 0 && id.length() > prefix.length()) {
            try {
                int seq = std::stoi(id.substr(prefix.length()));
                if (seq > maxSeq) maxSeq = seq;
            } catch (...) {}
        }
    }
    return maxSeq;
}

}  // namespace

TournamentService::TournamentService(TournamentRepository& tournamentRepository,
                                     BookingRepository& bookingRepository)
    : tournament_repository_(tournamentRepository),
      booking_repository_(bookingRepository) {}

bool TournamentService::requestTournament(const Tournament& tournament,
                                          std::string& error) {
    return tournament_repository_.save(tournament, error);
}

bool TournamentService::confirmTournament(Tournament& tournament,
                                          const std::string& bookingId,
                                          std::string& error) {
    tournament.confirm(bookingId);
    return tournament_repository_.update(tournament, error);
}

Queue<TournamentMatch> TournamentService::buildMatchScheduleQueue(
    const std::string& tournamentId,
    const std::string& date,
    const std::string& firstStart,
    int teamCount
) const {
    Queue<TournamentMatch> queue;
    if (teamCount < 2) return queue;
    int hour = 8, minute = 0;
    if (firstStart.length() == 5 && firstStart[2] == ':') {
        try {
            hour = std::stoi(firstStart.substr(0, 2));
            minute = std::stoi(firstStart.substr(3, 2));
        } catch (...) {}
    }
    int matchIndex = 1;
    for (int i = 1; i <= teamCount; i += 2) {
        if (i + 1 > teamCount) break;
        char homeId[32], awayId[32], matchId[32], timeBuf[16];
        std::snprintf(homeId, sizeof(homeId), "TM-%02d", i);
        std::snprintf(awayId, sizeof(awayId), "TM-%02d", i + 1);
        std::snprintf(matchId, sizeof(matchId), "MTH-%s-%02d",
                      tournamentId.c_str(), matchIndex);
        std::snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour, minute);
        queue.enqueue(TournamentMatch(
            matchId, tournamentId, homeId, awayId, 1, date, timeBuf, 0, 0, false
        ));
        minute += 90;
        hour += minute / 60;
        minute %= 60;
        ++matchIndex;
    }
    return queue;
}

bool TournamentService::loadAllTournaments(LinkedList<Tournament>& tournaments,
                                          std::string& error) const {
    return tournament_repository_.loadAll(tournaments, error);
}

bool TournamentService::getTournamentById(const std::string& tournamentId,
                                         Tournament& out,
                                         std::string& error) const {
    return tournament_repository_.findById(tournamentId, out, error);
}

bool TournamentService::checkOverlap(const Tournament& tournament,
                                    const std::string& excludeId,
                                    std::string& error) const {
    const std::string startDateIso = DateTime::toIsoDate(tournament.getStartDate());
    const std::string endDateIso   = DateTime::toIsoDate(tournament.getEndDate());

    if (!DateTime::isValidDate(startDateIso) || !DateTime::isValidDate(endDateIso)) {
        error = u8"Định dạng ngày giải đấu không hợp lệ (cần dd/MM/yyyy).";
        return false;
    }
    if (DateTime::compareDate(endDateIso, startDateIso) < 0) {
        error = u8"Ngày kết thúc giải đấu phải sau hoặc cùng ngày bắt đầu.";
        return false;
    }

    std::string tStartSlot, tEndSlot;
    tachKhung(tournament.getTimeSlot(), tStartSlot, tEndSlot);

    // 1. Kiểm tra trùng lịch với giải đấu khác
    LinkedList<Tournament> allTournaments;
    if (tournament_repository_.loadAll(allTournaments, error)) {
        for (auto* node = allTournaments.get_head(); node != nullptr; node = node->next) {
            const Tournament& existing = node->data;
            if (existing.getTournamentId() == excludeId) continue;
            if (
                existing.getStatus() == TournamentStatus::CANCELLED) continue;
            if (existing.getRequestedFieldId() != tournament.getRequestedFieldId()) continue;

            const std::string exStartIso = DateTime::toIsoDate(existing.getStartDate());
            const std::string exEndIso   = DateTime::toIsoDate(existing.getEndDate());

            bool dateOverlap = (DateTime::compareDate(startDateIso, exEndIso) <= 0 &&
                                DateTime::compareDate(endDateIso, exStartIso) >= 0);
            if (dateOverlap) {
                std::string exStartSlot, exEndSlot;
                tachKhung(existing.getTimeSlot(), exStartSlot, exEndSlot);
                if (thoiGianTrungNhau(tStartSlot, tEndSlot, exStartSlot, exEndSlot)) {
                    error = u8"Lỗi nghiệp vụ: Trùng lịch sân với giải đấu " +
                            existing.getName() + " (" + existing.getStartDate() +
                            " - " + existing.getEndDate() + ")!";
                    return false;
                }
            }
        }
    }

    // 2. Kiểm tra trùng lịch với đơn đặt sân còn hiệu lực
    LinkedList<VenueBookingRecord> bookings;
    std::string bookError;
    if (booking_repository_.loadAll(bookings, bookError)) {
        for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
            const VenueBookingRecord& b = node->data;
            if (b.fieldId != tournament.getRequestedFieldId()) continue;
            if (b.status == "CANCELLED" || b.status == u8"Đã hủy") continue;

            const std::string bDateIso = DateTime::toIsoDate(b.date);
            if (DateTime::compareDate(bDateIso, startDateIso) >= 0 &&
                DateTime::compareDate(bDateIso, endDateIso) <= 0) {
                if (thoiGianTrungNhau(tStartSlot, tEndSlot, b.startTime, b.endTime)) {
                    error = u8"Lỗi nghiệp vụ: Trùng lịch sân " + tournament.getRequestedFieldId() +
                            u8" với đơn đặt sân " + b.bookingId + u8" (" + b.date + " " +
                            b.startTime + "-" + b.endTime + ")!";
                    return false;
                }
            }
        }
    }

    return true;
}

bool TournamentService::duyetGiaiDau(const std::string& tournamentId,
                                    std::string& error) {
    Tournament target;
    if (!tournament_repository_.findById(tournamentId, target, error)) {
        return false;
    }
    if (!checkOverlap(target, tournamentId, error)) {
        return false;
    }
    target.setStatus(TournamentStatus::CONFIRMED);
    return tournament_repository_.update(target, error);
}

bool TournamentService::tuChoiGiaiDau(const std::string& tournamentId,
                                     const std::string& reason,
                                     std::string& error) {
    Tournament target;
    if (!tournament_repository_.findById(tournamentId, target, error)) {
        return false;
    }
    target.reject(reason.empty() ? u8"Trùng lịch sân hoặc quy mô không phù hợp." : reason);
    return tournament_repository_.update(target, error);
}

bool TournamentService::taoGiaiDauThuCong(const Tournament& tournament,
                                         std::string& outId,
                                         std::string& error) {
    if (tournament.getName().empty()) {
        error = u8"Tên giải đấu không được để trống!";
        return false;
    }
    if (!checkOverlap(tournament, "", error)) {
        return false;
    }

    Tournament g = tournament;
    if (g.getTournamentId().empty()) {
        LinkedList<Tournament> list;
        tournament_repository_.loadAll(list, error);
        LinkedList<std::string> ids;
        for (auto* n = list.get_head(); n != nullptr; n = n->next) ids.push_back(n->data.getTournamentId());
        int nextSeq = timMaxSeq(ids, "GD-") + 1;
        char buf[32];
        std::snprintf(buf, sizeof(buf), "GD-%02d", nextSeq);
        g.setTournamentId(buf);
    }
    g.setStatus(TournamentStatus::CONFIRMED);
    outId = g.getTournamentId();
    return tournament_repository_.save(g, error);
}

bool TournamentService::suaGiaiDau(const Tournament& tournament,
                                  std::string& error) {
    Tournament target;
    if (!tournament_repository_.findById(tournament.getTournamentId(), target, error)) {
        return false;
    }
    if (!checkOverlap(tournament, tournament.getTournamentId(), error)) {
        return false;
    }
    return tournament_repository_.update(tournament, error);
}

bool TournamentService::huyGiaiDau(const std::string& tournamentId,
                                  std::string& error) {
    return tournament_repository_.remove(tournamentId, error);
}

bool TournamentService::guiYeuCauTaoGiai(const Tournament& tournament,
                                        std::string& outId,
                                        std::string& error) {
    const std::string start = DateTime::toIsoDate(tournament.getStartDate());
    const std::string end = DateTime::toIsoDate(tournament.getEndDate());
    if (!DateTime::isValidDate(start) || DateTime::isPastDate(start)) {
        error = u8"Ngày bắt đầu giải không hợp lệ hoặc đã qua.";
        return false;
    }
    if (!DateTime::isValidDate(end) || DateTime::compareDate(end, start) < 0) {
        error = u8"Ngày kết thúc phải bằng hoặc sau ngày bắt đầu.";
        return false;
    }

    Tournament g = tournament;
    if (g.getTournamentId().empty()) {
        LinkedList<Tournament> list;
        tournament_repository_.loadAll(list, error);
        LinkedList<std::string> ids;
        for (auto* n = list.get_head(); n != nullptr; n = n->next) ids.push_back(n->data.getTournamentId());
        int nextSeq = timMaxSeq(ids, "GD-") + 1;
        char buf[32];
        std::snprintf(buf, sizeof(buf), "GD-%02d", nextSeq);
        g.setTournamentId(buf);
    }
    g.setStatus(TournamentStatus::REQUESTED);
    outId = g.getTournamentId();
    return tournament_repository_.save(g, error);
}

bool TournamentService::registerTeam(const std::string& tournamentId,
                                     const std::string& name,
                                     const std::string& phone,
                                     std::string& outTeamId,
                                     std::string& error) {
    if (tournamentId.empty() || name.empty()) {
        error = u8"Thông tin giải đấu hoặc tên đội không hợp lệ.";
        return false;
    }
    LinkedList<Team> teams;
    tournament_repository_.loadTeams(teams, error);
    LinkedList<std::string> ids;
    for (auto* n = teams.get_head(); n != nullptr; n = n->next) {
        if (n->data.getTournamentId() == tournamentId) {
            ids.push_back(n->data.getTeamId());
        }
    }
    int nextSeq = timMaxSeq(ids, "TM-") + 1;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "TM-%02d", nextSeq);
    outTeamId = buf;
    Team t(outTeamId, tournamentId, name, phone, 7);
    return tournament_repository_.saveTeam(t, error);
}

bool TournamentService::getTeamsByTournament(const std::string& tournamentId,
                                            LinkedList<Team>& out,
                                            std::string& error) const {
    out.clear();
    LinkedList<Team> all;
    if (!tournament_repository_.loadTeams(all, error)) return false;
    for (auto* n = all.get_head(); n != nullptr; n = n->next) {
        if (n->data.getTournamentId() == tournamentId) {
            out.push_back(n->data);
        }
    }
    return true;
}

bool TournamentService::scheduleMatch(const std::string& tournamentId,
                                      const std::string& homeTeamId,
                                      const std::string& awayTeamId,
                                      int round, const std::string& date,
                                      const std::string& startTime,
                                      std::string& outMatchId,
                                      std::string& error) {
    if (tournamentId.empty() || homeTeamId.empty() || awayTeamId.empty()) {
        error = u8"Thông tin trận đấu không hợp lệ.";
        return false;
    }
    LinkedList<TournamentMatch> matches;
    tournament_repository_.loadMatches(matches, error);
    LinkedList<std::string> ids;
    for (auto* n = matches.get_head(); n != nullptr; n = n->next) ids.push_back(n->data.getMatchId());
    int nextSeq = timMaxSeq(ids, "MTH-") + 1;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "MTH-%02d", nextSeq);
    outMatchId = buf;
    TournamentMatch m(outMatchId, tournamentId, homeTeamId, awayTeamId,
                      round, date, startTime, 0, 0, false);
    return tournament_repository_.saveMatch(m, error);
}

bool TournamentService::getMatchesByTournament(const std::string& tournamentId,
                                              LinkedList<TournamentMatch>& out,
                                              std::string& error) const {
    out.clear();
    LinkedList<TournamentMatch> all;
    if (!tournament_repository_.loadMatches(all, error)) return false;
    for (auto* n = all.get_head(); n != nullptr; n = n->next) {
        if (n->data.getTournamentId() == tournamentId) {
            out.push_back(n->data);
        }
    }
    return true;
}

bool TournamentService::recordMatchScore(const std::string& matchId, int homeScore,
                                        int awayScore, std::string& error) {
    LinkedList<TournamentMatch> matches;
    if (!tournament_repository_.loadMatches(matches, error)) return false;
    TournamentMatch* match = matches.findIf(
        [&matchId](const TournamentMatch& current) {
            return current.getMatchId() == matchId;
        });
    if (match == nullptr) {
        error = u8"Không tìm thấy trận đấu: " + matchId;
        return false;
    }
    match->setResult(homeScore, awayScore);
    return tournament_repository_.saveMatches(matches, error);
}
