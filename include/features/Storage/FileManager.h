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

enum class FileReadStatus {
    Loaded,
    NotFound,
    Error
};

// LÝ DO CHỌN:
// 1. Dùng LinkedList<std::string> qua tham chiếu để tái sử dụng cấu trúc tự cài đặt
//    trong Core cho toàn bộ các Repository mà không sao chép vùng nhớ động.
// 2. Bọc toàn bộ thao tác I/O đọc/ghi tập tin theo định dạng text phân tách,
//    đảm bảo dữ liệu được lưu trữ bền vững giữa các phiên làm việc.
class FileManager {
public:
    virtual ~FileManager() = default;

    // LÝ DO CHỌN: Chuẩn hóa trường text ngay tại ranh giới lưu trữ để một giá trị
    // người dùng không thể phá vỡ định dạng bản ghi phân tách bằng ký tự '|'.
    static std::string join(const std::string& value);

    // --- API đọc/ghi dòng tổng quát (Dùng chung cho các Repository) ---
    virtual bool readAllLines(const std::string& path, LinkedList<std::string>& out,
                              std::string& error) const;
    // LÝ DO CHỌN: Trạng thái riêng giúp phân biệt file chưa có với file rỗng,
    // đồng thời vẫn giữ hành vi bool cũ cho các Repository hiện tại.
    FileReadStatus readAllLinesWithStatus(const std::string& path,
                                          LinkedList<std::string>& out,
                                          std::string& error) const;
    virtual bool writeAllLines(const std::string& path,
                               const LinkedList<std::string>& lines,
                               std::string& error) const;
    bool writeAllLinesWithBackup(const std::string& path,
                                 const LinkedList<std::string>& lines,
                                 const std::string& backupPath,
                                 std::string& error) const;
    virtual bool appendLine(const std::string& path, const std::string& line,
                            std::string& error) const;
    bool backupFile(const std::string& sourcePath,
                    const std::string& backupPath,
                    std::string& error) const;
    // LÝ DO CHỌN: Khôi phục nguyên tử nội dung dòng; kiểm tra schema nghiệp vụ
    // vẫn thuộc về Repository vì FileManager không biết kiểu bản ghi cụ thể.
    bool restoreFile(const std::string& backupPath,
                     const std::string& targetPath,
                     std::string& error) const;

    // --- API hỗ trợ trực tiếp module Account ---
    static bool docDanhSachUser(
        std::vector<std::shared_ptr<User>>& danhSach,
        const std::string& filePath = "data/accounts.txt"
    );
    static bool docDanhSachUser(
        std::vector<std::shared_ptr<User>>& danhSach,
        const std::string& filePath,
        std::string& error
    );

    static bool ghiDanhSachUser(
        const std::vector<std::shared_ptr<User>>& danhSach,
        const std::string& filePath = "data/accounts.txt"
    );
    static bool ghiDanhSachUser(
        const std::vector<std::shared_ptr<User>>& danhSach,
        const std::string& filePath,
        std::string& error
    );
};

// Tạo alias trong namespace Storage để tương thích 100% với code gọi Storage::FileManager::...
namespace Storage {
    using FileManager = ::FileManager;
}  // namespace Storage

#endif // FILE_MANAGER_H
