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
    // LÃ DO CHá»ŒN: Queue (hÃ ng Ä‘á»£i FIFO) quáº£n lÃ½ thá»© tá»± cÃ¡c tráº­n Ä‘áº¥u trong lá»‹ch giáº£i.
    Queue<TournamentMatch> result;
    for (int i = 0; i + 1 < teamCount; i += 2) {
        char id[32];
        std::snprintf(id, sizeof(id), "%s-M%02d", tournamentId.c_str(), i / 2 + 1);
        result.enqueue(TournamentMatch(
            id, tournamentId,
            "T" + std::to_string(i + 1),
            "T" + std::to_string(i + 2),
            1, date, firstStart
        ));
    }
    return result;
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
    const std::string endDateIso = DateTime::toIsoDate(tournament.getEndDate());

    if (startDateIso.empty() || DateTime::isPastDate(startDateIso)) {
        error = u8"NgÃ y báº¯t Ä‘áº§u giáº£i khÃ´ng há»£p lá»‡ hoáº·c Ä‘Ã£ qua.";
        return false;
    }
    if (endDateIso.empty() || DateTime::compareDate(endDateIso, startDateIso) < 0) {
        error = u8"NgÃ y káº¿t thÃºc pháº£i báº±ng hoáº·c sau ngÃ y báº¯t Ä‘áº§u.";
        return false;
    }

    std::string tStartSlot, tEndSlot;
    tachKhung(tournament.getTimeSlot(), tStartSlot, tEndSlot);

    // 1. Kiá»ƒm tra trÃ¹ng lá»‹ch vá»›i cÃ¡c giáº£i Ä‘áº¥u Ä‘Ã£ duyá»‡t / Ä‘ang diá»…n ra khÃ¡c
    LinkedList<Tournament> allTournaments;
    std::string repoError;
    if (tournament_repository_.loadAll(allTournaments, repoError)) {
        for (auto* node = allTournaments.get_head(); node != nullptr; node = node->next) {
            const Tournament& t = node->data;
            if (t.getTournamentId() == excludeId) continue;
            if (t.getRequestedFieldId() != tournament.getRequestedFieldId()) continue;
            if (t.getStatus() != TournamentStatus::CONFIRMED &&
                t.getStatus() != TournamentStatus::IN_PROGRESS) {
                continue;
            }

            const std::string oStartIso = DateTime::toIsoDate(t.getStartDate());
            const std::string oEndIso = DateTime::toIsoDate(t.getEndDate());

            if (DateTime::compareDate(startDateIso, oEndIso) <= 0 &&
                DateTime::compareDate(endDateIso, oStartIso) >= 0) {
                std::string oStartSlot, oEndSlot;
                tachKhung(t.getTimeSlot(), oStartSlot, oEndSlot);
                if (thoiGianTrungNhau(tStartSlot, tEndSlot, oStartSlot, oEndSlot)) {
                    error = u8"Lỗi nghiệp vụ: Sân " + tournament.getRequestedFieldId() +
                            u8" đã có giải đấu \"" + t.getName() + u8"\" (" +
                            t.getTournamentId() + u8") diễn ra trong khoảng thời gian này!";
                    return false;
                }
            }
        }
    }

    // 2. Kiá»ƒm tra trÃ¹ng lá»‹ch vá»›i Ä‘Æ¡n Ä‘áº·t sÃ¢n cÃ²n hiá»‡u lá»±c (bá» qua Ä‘Æ¡n Ä‘Ã£ há»§y)
    LinkedList<VenueBookingRecord> bookings;
    std::string bookError;
    if (booking_repository_.loadAll(bookings, bookError)) {
        for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
            const VenueBookingRecord& b = node->data;
            if (b.fieldId != tournament.getRequestedFieldId()) continue;
            if (b.status == "CANCELLED" || b.status == u8"ÄÃ£ há»§y") continue;

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
    target.reject(reason.empty() ? u8"TrÃ¹ng lá»‹ch sÃ¢n hoáº·c quy mÃ´ khÃ´ng phÃ¹ há»£p." : reason);
    return tournament_repository_.update(target, error);
}

bool TournamentService::taoGiaiDauThuCong(const Tournament& tournament,
                                         std::string& outId,
                                         std::string& error) {
    if (tournament.getName().empty()) {
        error = u8"TÃªn giáº£i Ä‘áº¥u khÃ´ng Ä‘Æ°á»£c Ä‘á»ƒ trá»‘ng!";
        return false;
    }
    if (!checkOverlap(tournament, "", error)) {
        return false;
    }

    Tournament g = tournament;
    if (g.getTournamentId().empty()) {
        LinkedList<Tournament> list;
        tournament_repository_.loadAll(list, error);
        char buf[32];
        std::snprintf(buf, sizeof(buf), "GD-%02d", list.size() + 1);
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
        error = u8"NgÃ y báº¯t Ä‘áº§u giáº£i khÃ´ng há»£p lá»‡ hoáº·c Ä‘Ã£ qua.";
        return false;
    }
    if (!DateTime::isValidDate(end) || DateTime::compareDate(end, start) < 0) {
        error = u8"NgÃ y káº¿t thÃºc pháº£i báº±ng hoáº·c sau ngÃ y báº¯t Ä‘áº§u.";
        return false;
    }

    Tournament g = tournament;
    if (g.getTournamentId().empty()) {
        LinkedList<Tournament> list;
        tournament_repository_.loadAll(list, error);
        char buf[32];
        std::snprintf(buf, sizeof(buf), "GD-%02d", list.size() + 1);
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
    int count = 1;
    for (auto* n = teams.get_head(); n != nullptr; n = n->next) {
        if (n->data.getTournamentId() == tournamentId) ++count;
    }
    char buf[32];
    std::snprintf(buf, sizeof(buf), "TM-%02d", count);
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
    char buf[32];
    std::snprintf(buf, sizeof(buf), "MTH-%02d", matches.size() + 1);
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
    bool found = false;
    for (auto* n = matches.get_head(); n != nullptr; n = n->next) {
        if (n->data.getMatchId() == matchId) {
            n->data.setResult(homeScore, awayScore);
            found = true;
            break;
        }
    }
    if (!found) {
        error = u8"Không tìm thấy trận đấu: " + matchId;
        return false;
    }
    return tournament_repository_.saveMatches(matches, error);
}
