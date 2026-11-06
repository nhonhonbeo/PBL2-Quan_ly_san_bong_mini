#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include <string>

enum class TournamentStatus {
    REQUESTED = 0,
    CONFIRMED = 1,
    IN_PROGRESS = 2,
    FINISHED = 3,
    CANCELLED = 4
};

// LÝ DO CHỌN: Tournament biểu diễn thực thể giải đấu gồm 11 thuộc tính,
// tương thích trực tiếp với dữ liệu lưu trữ bền vững data/tournaments.txt.
class Tournament {
private:
    std::string tournament_id_;
    std::string name_;
    std::string requester_name_;
    std::string requester_user_id_;
    std::string start_date_;
    std::string end_date_;
    std::string requested_field_id_;
    std::string time_slot_;
    std::string note_;
    std::string reject_reason_;
    TournamentStatus status_;
    std::string confirmed_booking_id_;

public:
    Tournament(const std::string& tournamentId = "",
               const std::string& requesterUserId = "",
               const std::string& name = "",
               const std::string& requestedFieldId = "",
               const std::string& startDate = "",
               const std::string& endDate = "",
               const std::string& note = "",
               TournamentStatus status = TournamentStatus::REQUESTED,
               const std::string& confirmedBookingId = "",
               const std::string& requesterName = "",
               const std::string& timeSlot = "",
               const std::string& rejectReason = "");

    const std::string& getTournamentId() const;
    const std::string& getName() const;
    const std::string& getRequesterName() const;
    const std::string& getRequesterUserId() const;
    const std::string& getStartDate() const;
    const std::string& getEndDate() const;
    const std::string& getRequestedFieldId() const;
    const std::string& getTimeSlot() const;
    const std::string& getNote() const;
    const std::string& getRejectReason() const;
    TournamentStatus getStatus() const;
    const std::string& getConfirmedBookingId() const;

    void setTournamentId(const std::string& id);
    void setName(const std::string& name);
    void setRequesterName(const std::string& name);
    void setRequesterUserId(const std::string& userId);
    void setStartDate(const std::string& startDate);
    void setEndDate(const std::string& endDate);
    void setRequestedFieldId(const std::string& fieldId);
    void setTimeSlot(const std::string& timeSlot);
    void setNote(const std::string& note);
    void setRejectReason(const std::string& reason);
    void setStatus(TournamentStatus status);

    void confirm(const std::string& bookingId = "");
    void reject(const std::string& reason);
    void cancel();
};

#endif
