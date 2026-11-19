#pragma once

#include <cstdio>
#include <string>
#include <utility>

// Sinh mã định danh dạng <tiền tố><số thứ tự>, ví dụ "RV07", "GD-12", "TM-03".
//
// LÝ DO CHỌN: gom logic "quét số thứ tự lớn nhất rồi + 1" về một chỗ (trước đây bị
// sao chép ở AccountService, CustomerService, TournamentService, FieldService).
// Dùng max-sequence thay vì đếm size() + 1 để không trùng ID khi có bản ghi bị xóa.
// Lớp không phụ thuộc kiểu container: nơi gọi tự duyệt danh sách và gọi observe().
class IdGenerator {
public:
    // width: số chữ số tối thiểu (đệm số 0 bên trái), ví dụ width = 2 -> "01".
    explicit IdGenerator(std::string prefix, int width = 2)
        : prefix_(std::move(prefix)), width_(width) {}

    // Ghi nhận một ID đã tồn tại. ID sai tiền tố hoặc phần số không hợp lệ bị bỏ qua.
    void observe(const std::string& id) {
        if (id.size() <= prefix_.size() || id.compare(0, prefix_.size(), prefix_) != 0) {
            return;
        }
        const std::string tail = id.substr(prefix_.size());
        for (char c : tail) {
            if (c < '0' || c > '9') return;
        }
        try {
            const int seq = std::stoi(tail);
            if (seq > max_seq_) max_seq_ = seq;
        } catch (...) {
            // Phần số quá lớn: bỏ qua giống hành vi cũ (catch(...) {}).
        }
    }

    int maxSequence() const { return max_seq_; }

    // Sinh ID kế tiếp rồi tự tăng bộ đếm, nên gọi liên tiếp sẽ không trùng nhau.
    std::string next() {
        ++max_seq_;
        return format(prefix_, max_seq_, width_);
    }

    static std::string format(const std::string& prefix, int seq, int width = 2) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%0*d", width, seq);
        return prefix + buf;
    }

private:
    std::string prefix_;
    int width_;
    int max_seq_ = 0;
};
