#include "Storage/FileManager.h"

namespace {

bool reportTodo(std::string& error, const char* responsibility) {
    error = std::string("TODO FileManager: ") + responsibility +
            ". Sinh viên cần cài đặt truy xuất file an toàn.";
    return false;
}

}  // namespace

bool FileManager::readAllLines(const std::string& path,
                               LinkedList<std::string>& out,
                               std::string& error) const {
    (void)path;
    (void)out;
    // TODO: Đầu vào là đường dẫn hợp lệ; kết quả mong đợi là nạp từng dòng vào
    // out hoặc trả false kèm lỗi. Không đọc file khi chưa có format đã duyệt.
    return reportTodo(error, "đọc toàn bộ dòng");
}

bool FileManager::writeAllLines(const std::string& path,
                                const LinkedList<std::string>& lines,
                                std::string& error) const {
    (void)path;
    (void)lines;
    // TODO: Đầu vào là danh sách dòng đã serialize; kết quả mong đợi là ghi an
    // toàn hoặc trả false kèm lỗi, không tạo dữ liệu giả.
    return reportTodo(error, "ghi toàn bộ dòng");
}

bool FileManager::appendLine(const std::string& path, const std::string& line,
                             std::string& error) const {
    (void)path;
    (void)line;
    // TODO: Đầu vào là một dòng do Repository serialize; kết quả mong đợi là
    // thêm dòng hoặc trả false kèm lỗi.
    return reportTodo(error, "thêm một dòng");
}
