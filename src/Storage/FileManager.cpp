#include "Storage/FileManager.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

namespace {

// Hàm phụ trợ tách chuỗi dùng riêng cho module Account cũ
std::vector<std::string> tachChuoi(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Hàm hỗ trợ báo lỗi theo chuẩn TODO của đồ án
bool reportTodo(std::string& error, const char* responsibility) {
    error = std::string("TODO FileManager: ") + responsibility +
            ". Sinh viên cần cài đặt truy xuất file an toàn.";
    return false;
}

}  // namespace

// --- 1. Triển khai API đọc/ghi tổng quát (Dùng chung cho các Repository) ---

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

// --- 2. Triển khai API hỗ trợ trực tiếp module Account ---

bool FileManager::docDanhSachUser(
    std::vector<std::shared_ptr<User>>& danhSach,
    const std::string& filePath
) {
    danhSach.clear();

    if (!std::filesystem::exists(filePath)) {
        // LÝ DO CHỌN: Tạo dữ liệu tài khoản ban đầu nếu tập tin chưa tồn tại để người dùng có thể thử nghiệm ngay
        std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());

        auto ownerMacDinh = std::make_shared<Owner>(
            "US01",
            "owner01",
            "123456",
            u8"Nguyễn Văn Chủ",
            "0901234567",
            u8"Cụm sân Sao Vàng",
            u8"123 Nguyễn Huệ, Đà Nẵng",
            u8"Đi giày đế TF/IC, không hút thuốc",
            "06:00",
            "23:00",
            u8"Nguyễn Văn Chủ"
        );

        auto customerMacDinh = std::make_shared<Customer>(
            "US02",
            "renter01",
            "123456",
            u8"Trần Văn Thuê",
            "0909876543"
        );

        danhSach.push_back(ownerMacDinh);
        danhSach.push_back(customerMacDinh);

        ghiDanhSachUser(danhSach, filePath);
        return true;
    }

    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(inFile, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::vector<std::string> parts = tachChuoi(line, '|');
        if (parts.empty()) {
            continue;
        }

        if (parts[0] == "CUSTOMER" && parts.size() >= 6) {
            auto customer = std::make_shared<Customer>(
                parts[1], // user_id
                parts[2], // user_name
                parts[3], // password
                parts[4], // name
                parts[5]  // phone_number
            );
            danhSach.push_back(customer);
        } else if (parts[0] == "OWNER" && parts.size() >= 12) {
            auto owner = std::make_shared<Owner>(
                parts[1],  // user_id
                parts[2],  // user_name
                parts[3],  // password
                parts[4],  // name
                parts[5],  // phone_number
                parts[6],  // field_name
                parts[7],  // field_address
                parts[8],  // field_rules
                parts[9],  // open_time
                parts[10], // close_time
                parts[11]  // owner_name
            );
            danhSach.push_back(owner);
        }
    }

    inFile.close();
    return true;
}

bool FileManager::ghiDanhSachUser(
    const std::vector<std::shared_ptr<User>>& danhSach,
    const std::string& filePath
) {
    std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());

    std::ofstream outFile(filePath, std::ios::trunc);
    if (!outFile.is_open()) {
        return false;
    }

    for (const auto& user : danhSach) {
        if (!user) continue;

        if (user->get_account_type() == "Customer") {
            auto cust = std::dynamic_pointer_cast<Customer>(user);
            outFile << "CUSTOMER|"
                    << user->get_user_id() << "|"
                    << user->get_user_name() << "|"
                    << "123456" << "|" // Mật khẩu mặc định hoặc lưu kèm
                    << user->get_name() << "|"
                    << user->get_phone_number() << "|"
                    << (cust ? cust->get_booking_count() : 0) << "|"
                    << (cust ? cust->get_cancel_count() : 0) << "\n";
        } else if (user->get_account_type() == "Owner") {
            auto own = std::dynamic_pointer_cast<Owner>(user);
            outFile << "OWNER|"
                    << user->get_user_id() << "|"
                    << user->get_user_name() << "|"
                    << "123456" << "|"
                    << user->get_name() << "|"
                    << user->get_phone_number() << "|"
                    << (own ? own->get_field_name() : "") << "|"
                    << (own ? own->get_field_address() : "") << "|"
                    << (own ? own->get_field_rules() : "") << "|"
                    << (own ? own->get_open_time() : "06:00") << "|"
                    << (own ? own->get_close_time() : "23:00") << "|"
                    << (own ? own->get_owner_name() : user->get_name()) << "\n";
        }
    }

    outFile.close();
    return true;
}