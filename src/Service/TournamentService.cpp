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
    // L? DO CH?N: Queue (h?ng ??i FIFO) qu?n l? th? t? c?c tr?n ??u trong l?ch gi?i.
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
        error = u8"Ng?y b?t ??u gi?i kh?ng h?p l? ho?c ?? qua.";
        return false;
    }
    if (endDateIso.empty() || DateTime::compareDate(endDateIso, startDateIso) < 0) {
        error = u8"Ng?y k?t th?c ph?i b?ng ho?c sau ng?y b?t ??u.";
        return false;
    }

    std::string tStartSlot, tEndSlot;
    tachKhung(tournament.getTimeSlot(), tStartSlot, tEndSlot);

    // 1. Ki?m tra tr?ng l?ch v?i c?c gi?i ??u ?? duy?t / ?ang di?n ra kh?c
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
                    error = u8"L?i nghi?p v?: S?n " + tournament.getRequestedFieldId() +
                            u8" ?? c? gi?i ??u "" + t.getName() + u8"" (" +
                            t.getTournamentId() + u8") di?n ra trong kho?ng th?i gian n?y!";
                    return false;
                }
            }
        }
    }

    // 2. Ki?m tra tr?ng l?ch v?i ??n ??t s?n c?n hi?u l?c (b? qua ??n ?? h?y)
    LinkedList<VenueBookingRecord> bookings;
    std::string bookError;
    if (booking_repository_.loadAll(bookings, bookError)) {
        for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
            const VenueBookingRecord& b = node->data;
            if (b.fieldId != tournament.getRequestedFieldId()) continue;
            if (b.status == "CANCELLED" || b.status == u8"?? h?y") continue;

            const std::string bDateIso = DateTime::toIsoDate(b.date);
            if (DateTime::compareDate(bDateIso, startDateIso) >= 0 &&
                DateTime::compareDate(bDateIso, endDateIso) <= 0) {
                if (thoiGianTrungNhau(tStartSlot, tEndSlot, b.startTime, b.endTime)) {
                    error = u8"L?i nghi?p v?: Tr?ng l?ch s?n " + tournament.getRequestedFieldId() +
                            u8" v?i ??n ??t s?n " + b.bookingId + u8" (" + b.date + " " +
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
    target.reject(reason.empty() ? u8"Tr?ng l?ch s?n ho?c quy m? kh?ng ph? h?p." : reason);
    return tournament_repository_.update(target, error);
}

bool TournamentService::taoGiaiDauThuCong(const Tournament& tournament,
                                         std::string& outId,
                                         std::string& error) {
    if (tournament.getName().empty()) {
        error = u8"T?n gi?i ??u kh?ng ???c ?? tr?ng!";
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
        error = u8"Ng?y b?t ??u gi?i kh?ng h?p l? ho?c ?? qua.";
        return false;
    }
    if (!DateTime::isValidDate(end) || DateTime::compareDate(end, start) < 0) {
        error = u8"Ng?y k?t th?c ph?i b?ng ho?c sau ng?y b?t ??u.";
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
