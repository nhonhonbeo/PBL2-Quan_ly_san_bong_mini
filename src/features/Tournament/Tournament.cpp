#include "Tournament/Tournament.h"

Tournament::Tournament(const std::string& tournamentId,
                       const std::string& requesterUserId,
                       const std::string& name,
                       const std::string& requestedFieldId,
                       const std::string& startDate,
                       const std::string& endDate,
                       const std::string& note,
                       TournamentStatus status,
                       const std::string& confirmedBookingId,
                       const std::string& requesterName,
                       const std::string& timeSlot,
                       const std::string& rejectReason)
    : tournament_id_(tournamentId),
      name_(name),
      requester_name_(requesterName),
      requester_user_id_(requesterUserId),
      start_date_(startDate),
      end_date_(endDate),
      requested_field_id_(requestedFieldId),
      time_slot_(timeSlot),
      note_(note),
      reject_reason_(rejectReason),
      status_(status),
      confirmed_booking_id_(confirmedBookingId) {}

const std::string& Tournament::getTournamentId() const {
    return tournament_id_;
}

const std::string& Tournament::getName() const {
    return name_;
}

const std::string& Tournament::getRequesterName() const {
    return requester_name_;
}

const std::string& Tournament::getRequesterUserId() const {
    return requester_user_id_;
}

const std::string& Tournament::getStartDate() const {
    return start_date_;
}

const std::string& Tournament::getEndDate() const {
    return end_date_;
}

const std::string& Tournament::getRequestedFieldId() const {
    return requested_field_id_;
}

const std::string& Tournament::getTimeSlot() const {
    return time_slot_;
}

const std::string& Tournament::getNote() const {
    return note_;
}

const std::string& Tournament::getRejectReason() const {
    return reject_reason_;
}

TournamentStatus Tournament::getStatus() const {
    return status_;
}

const std::string& Tournament::getConfirmedBookingId() const {
    return confirmed_booking_id_;
}

void Tournament::setTournamentId(const std::string& id) {
    tournament_id_ = id;
}

void Tournament::setName(const std::string& name) {
    name_ = name;
}

void Tournament::setRequesterName(const std::string& name) {
    requester_name_ = name;
}

void Tournament::setRequesterUserId(const std::string& userId) {
    requester_user_id_ = userId;
}

void Tournament::setStartDate(const std::string& startDate) {
    start_date_ = startDate;
}

void Tournament::setEndDate(const std::string& endDate) {
    end_date_ = endDate;
}

void Tournament::setRequestedFieldId(const std::string& fieldId) {
    requested_field_id_ = fieldId;
}

void Tournament::setTimeSlot(const std::string& timeSlot) {
    time_slot_ = timeSlot;
}

void Tournament::setNote(const std::string& note) {
    note_ = note;
}

void Tournament::setRejectReason(const std::string& reason) {
    reject_reason_ = reason;
}

void Tournament::setStatus(TournamentStatus status) {
    status_ = status;
}

void Tournament::confirm(const std::string& bookingId) {
    if (!bookingId.empty()) {
        confirmed_booking_id_ = bookingId;
    }
    status_ = TournamentStatus::CONFIRMED;
}

void Tournament::reject(const std::string& reason) {
    status_ = TournamentStatus::CANCELLED;
    reject_reason_ = reason;
}

void Tournament::cancel() {
    status_ = TournamentStatus::CANCELLED;
}
