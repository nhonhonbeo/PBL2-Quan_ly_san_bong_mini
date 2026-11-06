#include "Booking/BookingDetail.h"

namespace {

// Đổi chuỗi "HH:MM" thành số phút tính từ 00:00. Trả về -1 nếu chuỗi không hợp lệ.
int parseMinutes(const std::string& text) {
    if (text.size() != 5 || text[2] != ':') {
        return -1;
    }
    for (std::size_t i : {0u, 1u, 3u, 4u}) {
        if (text[i] < '0' || text[i] > '9') {
            return -1;
        }
    }
    const int hour = (text[0] - '0') * 10 + (text[1] - '0');
    const int minute = (text[3] - '0') * 10 + (text[4] - '0');
    if (hour > 23 || minute > 59) {
        return -1;
    }
    return hour * 60 + minute;
}

// LÝ DO CHỌN: constructor cũ (4 tham số) không nhận thời lượng, nên tự suy ra từ giờ bắt đầu/kết thúc
// để mã cũ vẫn có thời lượng đúng. Giờ không hợp lệ hoặc kết thúc không sau bắt đầu thì trả về 0.
int calculateDuration(const std::string& start, const std::string& end) {
    const int from = parseMinutes(start);
    const int to = parseMinutes(end);
    if (from < 0 || to < 0 || to <= from) {
        return 0;
    }
    return to - from;
}

}  // namespace

BookingDetail::BookingDetail() : duration_minutes_(0), amount_vnd_(0) {}

BookingDetail::BookingDetail(const std::string& id, const std::string& start, const std::string& end,
                             long long amount)
    : slot_id_(id), start_time_(start), end_time_(end),
      duration_minutes_(calculateDuration(start, end)), amount_vnd_(amount) {}

BookingDetail::BookingDetail(const std::string& id, const std::string& start, const std::string& end,
                             int durationMinutes, long long amount)
    : slot_id_(id), start_time_(start), end_time_(end),
      duration_minutes_(durationMinutes), amount_vnd_(amount) {}

const std::string& BookingDetail::getSlotId() const { return slot_id_; }
const std::string& BookingDetail::getStartTime() const { return start_time_; }
const std::string& BookingDetail::getEndTime() const { return end_time_; }
int BookingDetail::getDurationMinutes() const { return duration_minutes_; }
long long BookingDetail::getAmountVnd() const { return amount_vnd_; }