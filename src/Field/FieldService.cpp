#include "Service/FieldService.h"

namespace {

bool reportTodo(std::string& error, const char* responsibility) {
    error = std::string("TODO FieldService: ") + responsibility +
            ". Sinh viên cần cài đặt quy tắc nghiệp vụ Field.";
    return false;
}

}  // namespace

FieldService::FieldService(FieldRepository& fieldRepository)
    : field_repository_(fieldRepository) {}

bool FieldService::createField(const Field& field, std::string& error) {
    (void)field;
    // TODO: Kiểm tra mã sân, Owner, loại sân, giá và giờ hoạt động rồi mới gọi
    // FieldRepository::save; không trả thành công khi chưa cài đặt.
    return reportTodo(error, "tạo sân");
}

bool FieldService::updateField(const Field& field, std::string& error) {
    (void)field;
    // TODO: Kiểm tra dữ liệu Field và quyền cập nhật trước khi gọi Repository.
    return reportTodo(error, "cập nhật sân");
}

bool FieldService::setFieldStatus(const std::string& fieldId,
                                  FieldStatus status, std::string& error) {
    (void)fieldId;
    (void)status;
    // TODO: Kiểm tra sân tồn tại và ràng buộc trạng thái trước khi lưu thay đổi.
    return reportTodo(error, "đổi trạng thái sân");
}

bool FieldService::validateOperatingSlot(const Field& field,
                                         const TimeSlot& slot,
                                         std::string& error) const {
    (void)field;
    (void)slot;
    // TODO: Dùng DateTime sau khi Core có API thật để kiểm tra định dạng HH:MM
    // và slot có nằm trong giờ mở/đóng của Field hay không.
    return reportTodo(error, "kiểm tra khung giờ hoạt động");
}
