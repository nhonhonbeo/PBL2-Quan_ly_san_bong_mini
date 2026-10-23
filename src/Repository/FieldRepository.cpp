#include "Repository/FieldRepository.h"

namespace {

bool reportTodo(std::string& error, const char* responsibility) {
    error = std::string("TODO FieldRepository: ") + responsibility +
            ". Sinh viên cần cài đặt ánh xạ Field với fields.txt.";
    return false;
}

}  // namespace

FieldRepository::FieldRepository(FileManager& fileManager)
    : file_manager_(fileManager) {}

bool FieldRepository::findById(const std::string& fieldId, Field& out,
                               std::string& error) const {
    (void)fieldId;
    (void)out;
    // TODO: Tìm fieldId trong fields.txt; trả true và gán out khi tìm thấy,
    // hoặc trả false kèm lỗi nếu không tìm thấy/không thể đọc file.
    return reportTodo(error, "tìm sân theo mã");
}

bool FieldRepository::save(const Field& field, std::string& error) {
    (void)field;
    // TODO: Serialize Field vào fields.txt; không kiểm tra Owner, giờ hoạt động
    // hoặc trùng lịch ở tầng Repository.
    return reportTodo(error, "lưu sân");
}

bool FieldRepository::update(const Field& field, std::string& error) {
    (void)field;
    // TODO: Cập nhật record Field đã tồn tại trong fields.txt.
    return reportTodo(error, "cập nhật sân");
}
