#include "Tournament/Tournament.h"

Tournament::Tournament(const std::string& tournamentId,
                       const std::string& requesterUserId,
                       const std::string& name,
                       const std::string& requestedFieldId,
                       const std::string& startDate,
                       const std::string& endDate, const std::string& note,
                       TournamentStatus status,
                       const std::string& confirmedBookingId)
    : tournament_id_(tournamentId), requester_user_id_(requesterUserId),
      name_(name), requested_field_id_(requestedFieldId),
      confirmed_booking_id_(confirmedBookingId), start_date_(startDate),
      end_date_(endDate), status_(status), note_(note) {}

const std::string& Tournament::getTournamentId() const {
    return tournament_id_;
}

const std::string& Tournament::getRequesterUserId() const {
    return requester_user_id_;
}

const std::string& Tournament::getName() const { return name_; }

const std::string& Tournament::getRequestedFieldId() const {
    return requested_field_id_;
}

const std::string& Tournament::getConfirmedBookingId() const {
    return confirmed_booking_id_;
}

const std::string& Tournament::getStartDate() const { return start_date_; }

const std::string& Tournament::getEndDate() const { return end_date_; }

TournamentStatus Tournament::getStatus() const { return status_; }

const std::string& Tournament::getNote() const { return note_; }

void Tournament::confirm(const std::string& bookingId) {
    confirmed_booking_id_ = bookingId;
    status_ = TournamentStatus::CONFIRMED;
}

void Tournament::cancel() { status_ = TournamentStatus::CANCELLED; }
