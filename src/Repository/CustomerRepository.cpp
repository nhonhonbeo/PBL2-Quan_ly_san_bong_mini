#include "Repository/CustomerRepository.h"

#include "Account/Customer.h"

namespace {

bool reportTodo(std::string& error, const char* responsibility) {
    error = std::string("TODO CustomerRepository: ") + responsibility +
            ". Sinh viên cần cài đặt ánh xạ dữ liệu Customer với file.";
    return false;
}

}  // namespace

CustomerRepository::CustomerRepository(FileManager& fileManager)
    : file_manager_(fileManager) {}

bool CustomerRepository::findCustomerByUserId(const std::string& userId,
                                              Customer& out,
                                              std::string& error) const {
    (void)userId;
    (void)out;
    // TODO: Tìm userId trong customers.txt; trả true và gán out khi tìm thấy,
    // hoặc trả false kèm lỗi nếu không tìm thấy/không thể đọc file.
    return reportTodo(error, "tìm hồ sơ khách theo user_id");
}

bool CustomerRepository::saveCustomer(const Customer& customer,
                                      std::string& error) {
    (void)customer;
    // TODO: Serialize CustomerProfile vào customers.txt. Repository không xét
    // điều kiện blacklist, ưu đãi hay trạng thái Booking.
    return reportTodo(error, "lưu hồ sơ khách");
}

bool CustomerRepository::updateCustomer(const Customer& customer,
                                        std::string& error) {
    (void)customer;
    // TODO: Cập nhật record CustomerProfile đã tồn tại trong customers.txt.
    return reportTodo(error, "cập nhật hồ sơ khách");
}

bool CustomerRepository::findActiveBlacklist(const std::string& customerId,
                                             Blacklist& out,
                                             std::string& error) const {
    (void)customerId;
    (void)out;
    // TODO: Đọc blacklist.txt và trả về record active của customerId nếu có.
    return reportTodo(error, "tìm blacklist đang hiệu lực");
}

bool CustomerRepository::saveBlacklist(const Blacklist& blacklist,
                                       std::string& error) {
    (void)blacklist;
    // TODO: Serialize Blacklist vào blacklist.txt; Service quyết định khi nào
    // khách bị đưa vào blacklist.
    return reportTodo(error, "lưu blacklist");
}

bool CustomerRepository::saveDiscount(const Discount& discount,
                                      std::string& error) {
    (void)discount;
    // TODO: Serialize Discount vào discounts.txt, không tự cấp ưu đãi tại đây.
    return reportTodo(error, "lưu ưu đãi");
}

bool CustomerRepository::saveReview(const Review& review, std::string& error) {
    (void)review;
    // TODO: Serialize Review vào reviews.txt sau khi Service đã kiểm tra quyền.
    return reportTodo(error, "lưu đánh giá");
}
