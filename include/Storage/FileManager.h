#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>

#include "Core/LinkedList.h"

// LÝ DO CHỌN: Dùng LinkedList<std::string> qua tham chiếu để tái sử dụng cấu
// trúc tự cài đặt mà không sao chép LinkedList quản lý bộ nhớ động.
class FileManager {
public:
    bool readAllLines(const std::string& path, LinkedList<std::string>& out,
                      std::string& error) const;
    bool writeAllLines(const std::string& path,
                       const LinkedList<std::string>& lines,
                       std::string& error) const;
    bool appendLine(const std::string& path, const std::string& line,
                    std::string& error) const;
};

#endif
