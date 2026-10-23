#include "Service/CustomerService.h"

namespace {

bool reportTodo(std::string& error, const char* responsibility) {
    error = std::string("TODO CustomerService: ") + responsibility +
            ". Sinh viên cần cài đặt quy tắc nghiệp vụ Customer.";
    return false;
}

}  // namespace

CustomerService::CustomerService(CustomerRepository& customerRepository)
    : customer_repository_(customerRepository) {}

bool CustomerService::isBlacklisted(const std::string& userId,
                                    std::string& reason) const {
    (void)userId;
    // TODO: Khi hoàn thiện phải phân biệt được khách không bị chặn với lỗi truy
    // xuất. Chữ ký hiện tại chưa đủ thông tin để BookingService dùng an toàn.
    reason = "TODO CustomerService: chưa cài đặt kiểm tra blacklist.";
    return false;
}

bool CustomerService::recordSuccessfulBooking(const std::string& userId,
                                              std::string& error) {
    (void)userId;
    // TODO: Chỉ gọi sau khi BookingRepository lưu thành công; cập nhật thống kê
    // CustomerProfile rồi lưu lại, hoặc trả false kèm lỗi.
    return reportTodo(error, "ghi nhận lượt đặt sân");
}

bool CustomerService::recordCancellation(const std::string& userId,
                                         std::string& error) {
    (void)userId;
    // TODO: Chỉ gọi sau khi hủy booking thành công; cập nhật bộ đếm và áp dụng
    // ngưỡng blacklist sau khi quy tắc này được xác nhận.
    return reportTodo(error, "ghi nhận lượt hủy sân");
}

bool CustomerService::addReview(const Review& review, std::string& error) {
    (void)review;
    // TODO: Kiểm tra booking, khách, sân, rating và giới hạn review trước khi
    // gọi CustomerRepository::saveReview.
    return reportTodo(error, "thêm đánh giá");
}

bool CustomerService::findAvailableDiscount(const std::string& userId,
                                            Discount& out,
                                            std::string& error) const {
    (void)userId;
    (void)out;
    // TODO: Tìm Discount AVAILABLE theo chính sách được duyệt; không tự tạo
    // Discount để giả lập kết quả thành công.
    return reportTodo(error, "tìm ưu đãi khả dụng");
}
