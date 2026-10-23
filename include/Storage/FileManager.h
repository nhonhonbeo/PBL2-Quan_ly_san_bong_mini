#pragma once
#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <memory>
#include <string>
#include <vector>

#include "Core/LinkedList.h"
#include "Account/Customer.h"
#include "Account/Owner.h"
#include "Account/User.h"

// LÝ DO CHỌN:
// 1. Dùng LinkedList<std::string> qua tham chiếu để tái sử dụng cấu trúc tự cài đặt
//    trong Core cho toàn bộ các Repository mà không sao chép vùng nhớ động.
// 2. Bọc toàn bộ thao tác I/O đọc/ghi tập tin theo định dạng text phân tách,
//    đảm bảo dữ liệu được lưu trữ bền vững giữa các phiên làm việc.
class FileManager {
public:
    // LÝ DO CHỌN: Chuẩn hóa trường text ngay tại ranh giới lưu trữ để một giá trị
    // người dùng không thể phá vỡ định dạng bản ghi phân tách bằng ký tự '|'.
    static std::string join(const std::string& value);
    // --- API đọc/ghi dòng tổng quát (Dùng chung cho các Repository) ---
    bool readAllLines(const std::string& path, LinkedList<std::string>& out,
                      std::string& error) const;
    bool writeAllLines(const std::string& path,
                       const LinkedList<std::string>& lines,
                       std::string& error) const;
    bool appendLine(const std::string& path, const std::string& line,
                    std::string& error) const;

    // --- API hỗ trợ trực tiếp module Account ---
    static bool docDanhSachUser(
        std::vector<std::shared_ptr<User>>& danhSach,
        const std::string& filePath = "data/accounts.txt"
    );

    static bool ghiDanhSachUser(
        const std::vector<std::shared_ptr<User>>& danhSach,
        const std::string& filePath = "data/accounts.txt"
    );
};

// Tạo alias trong namespace Storage để tương thích 100% với code gọi Storage::FileManager::...
namespace Storage {
    using FileManager = ::FileManager;
}  // namespace Storage

#endif // FILE_MANAGER_H
