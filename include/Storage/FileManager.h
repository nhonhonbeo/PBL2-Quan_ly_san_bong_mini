#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Account/Customer.h"
#include "Account/Owner.h"
#include "Account/User.h"

namespace Storage {

// LÝ DO CHỌN: FileManager bọc toàn bộ thao tác I/O đọc/ghi tập tin theo định dạng text phân tách,
// đảm bảo dữ liệu người dùng được lưu trữ bền vững giữa các phiên làm việc của phần mềm.
class FileManager {
public:
    static bool docDanhSachUser(
        std::vector<std::shared_ptr<User>>& danhSach,
        const std::string& filePath = "data/accounts.txt"
    );

    static bool ghiDanhSachUser(
        const std::vector<std::shared_ptr<User>>& danhSach,
        const std::string& filePath = "data/accounts.txt"
    );
};

}  // namespace Storage
