#include "Booking/BookingDetail.h"
BookingDetail::BookingDetail() : amount_vnd_(0) {}
BookingDetail::BookingDetail(const std::string& id, const std::string& start, const std::string& end, long long amount) : slot_id_(id), start_time_(start), end_time_(end), amount_vnd_(amount) {}
const std::string& BookingDetail::getSlotId() const { return slot_id_; }
const std::string& BookingDetail::getStartTime() const { return start_time_; }
const std::string& BookingDetail::getEndTime() const { return end_time_; }
long long BookingDetail::getAmountVnd() const { return amount_vnd_; }
