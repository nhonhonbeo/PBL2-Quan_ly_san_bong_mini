#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include <string>

enum class TournamentStatus {
    REQUESTED,
    CONFIRMED,
    CANCELLED
};

// LÝ DO CHỌN: Tournament chỉ giữ yêu cầu giải tối thiểu; Booking là nguồn xác
// nhận giữ sân để Tournament không trở thành hệ thống lịch độc lập.
class Tournament {
private:
    std::string tournament_id_;
    std::string requester_user_id_;
    std::string name_;
    std::string requested_field_id_;
    std::string confirmed_booking_id_;
    std::string start_date_;
    std::string end_date_;
    TournamentStatus status_;
    std::string note_;

public:
    Tournament(const std::string& tournamentId,
               const std::string& requesterUserId, const std::string& name,
               const std::string& requestedFieldId,
               const std::string& startDate, const std::string& endDate,
               const std::string& note,
               TournamentStatus status = TournamentStatus::REQUESTED,
               const std::string& confirmedBookingId = "");

    const std::string& getTournamentId() const;
    const std::string& getRequesterUserId() const;
    const std::string& getName() const;
    const std::string& getRequestedFieldId() const;
    const std::string& getConfirmedBookingId() const;
    const std::string& getStartDate() const;
    const std::string& getEndDate() const;
    TournamentStatus getStatus() const;
    const std::string& getNote() const;

    // TournamentService tương lai chỉ gọi sau khi BookingService xác nhận sân.
    void confirm(const std::string& bookingId);
    void cancel();
};

#endif
