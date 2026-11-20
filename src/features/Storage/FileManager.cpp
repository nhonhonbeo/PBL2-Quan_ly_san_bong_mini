#include "Storage/FileManager.h"

#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <system_error>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

std::vector<std::string> tachChuoi(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string::size_type start = 0;
    while (true) {
        const std::string::size_type separator = str.find(delimiter, start);
        if (separator == std::string::npos) {
            tokens.push_back(str.substr(start));
            break;
        }
        tokens.push_back(str.substr(start, separator - start));
        start = separator + 1;
    }
    return tokens;
}

bool parseSoNguyenKhongAm(const std::string& value, int& result) {
    if (value.empty()) return false;
    for (const char digit : value) {
        if (digit < '0' || digit > '9') return false;
    }
    try {
        size_t read = 0;
        const long long parsed = std::stoll(value, &read);
        if (read != value.size() || parsed < 0 ||
            parsed > std::numeric_limits<int>::max()) {
            return false;
        }
        result = static_cast<int>(parsed);
        return true;
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }
}

std::filesystem::path taoDuongDanTam(const std::filesystem::path& path) {
    static std::atomic<unsigned long long> sequence{0};
    const auto ticks = std::chrono::steady_clock::now()
                           .time_since_epoch()
                           .count();
    return std::filesystem::path(
        path.string() + ".tmp." + std::to_string(ticks) + "." +
        std::to_string(sequence.fetch_add(1, std::memory_order_relaxed)));
}

bool taoThuMucCha(const std::filesystem::path& path, std::string& error) {
    const std::filesystem::path parent = path.parent_path();
    if (parent.empty()) return true;

    std::error_code ec;
    std::filesystem::create_directories(parent, ec);
    if (ec) {
        error = "Không thể tạo thư mục chứa file " + path.string() +
                ": " + ec.message();
        return false;
    }
    return true;
}

bool validateLines(const LinkedList<std::string>& lines, std::string& error) {
    size_t lineNumber = 0;
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        ++lineNumber;
        if (node->data.find('\n') != std::string::npos ||
            node->data.find('\r') != std::string::npos) {
            error = "Bản ghi dòng " + std::to_string(lineNumber) +
                    " chứa ký tự xuống dòng.";
            return false;
        }
    }
    return true;
}

bool ghiFileTam(const std::filesystem::path& destination,
                const LinkedList<std::string>& lines,
                std::filesystem::path& temporary,
                std::string& error) {
    temporary = taoDuongDanTam(destination);
    std::ofstream output(temporary, std::ios::out | std::ios::binary |
                                        std::ios::trunc);
    if (!output.is_open()) {
        error = "Không thể mở file tạm để ghi: " + temporary.string();
        return false;
    }

    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        if (node->data.size() >
            static_cast<size_t>(std::numeric_limits<std::streamsize>::max())) {
            error = "Bản ghi quá lớn để ghi vào file.";
            output.close();
            std::error_code ignored;
            std::filesystem::remove(temporary, ignored);
            return false;
        }
        output.write(node->data.data(),
                     static_cast<std::streamsize>(node->data.size()));
        output.put('\n');
        if (!output) {
            error = "Lỗi khi ghi file tạm: " + temporary.string();
            output.close();
            std::error_code ignored;
            std::filesystem::remove(temporary, ignored);
            return false;
        }
    }

    output.flush();
    if (!output) {
        error = "Lỗi khi flush file tạm: " + temporary.string();
        output.close();
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        return false;
    }
    output.close();
    if (output.fail()) {
        error = "Lỗi khi đóng file tạm: " + temporary.string();
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        return false;
    }
    return true;
}

bool saoCheFileTam(const std::filesystem::path& sourcePath,
                   const std::filesystem::path& destinationPath,
                   std::filesystem::path& temporary,
                   std::string& error) {
    temporary = taoDuongDanTam(destinationPath);
    std::ifstream input(sourcePath, std::ios::in | std::ios::binary);
    if (!input.is_open()) {
        error = "Không thể mở file nguồn để sao lưu: " + sourcePath.string();
        return false;
    }

    std::ofstream output(temporary, std::ios::out | std::ios::binary |
                                        std::ios::trunc);
    if (!output.is_open()) {
        error = "Không thể tạo file backup tạm: " + temporary.string();
        return false;
    }

    char buffer[8192];
    while (input.read(buffer, sizeof(buffer)) || input.gcount() > 0) {
        output.write(buffer, input.gcount());
        if (!output) {
            error = "Lỗi khi sao chép dữ liệu vào file backup tạm.";
            output.close();
            std::error_code ignored;
            std::filesystem::remove(temporary, ignored);
            return false;
        }
    }
    if (input.bad()) {
        error = "Lỗi khi đọc file nguồn để sao lưu: " + sourcePath.string();
        output.close();
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        return false;
    }

    output.flush();
    if (!output) {
        error = "Lỗi khi flush file backup tạm.";
        output.close();
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        return false;
    }
    output.close();
    if (output.fail()) {
        error = "Lỗi khi đóng file backup tạm.";
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        return false;
    }
    return true;
}

// LÝ DO CHỌN: Thay thế tại cùng thư mục để thao tác rename không đi qua volume;
// Windows và POSIX dùng primitive riêng vì khác nhau khi đích đã tồn tại.
bool thayTheFile(const std::filesystem::path& temporary,
                 const std::filesystem::path& destination,
                 std::string& error) {
#ifdef _WIN32
    if (MoveFileExW(temporary.c_str(), destination.c_str(),
                    MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == 0) {
        error = "Không thể thay thế file " + destination.string() + ": " +
                std::error_code(static_cast<int>(GetLastError()),
                                std::system_category()).message();
        return false;
    }
#else
    if (std::rename(temporary.c_str(), destination.c_str()) != 0) {
        error = "Không thể thay thế file " + destination.string() + ": " +
                std::error_code(errno, std::generic_category()).message();
        return false;
    }
#endif
    return true;
}

void xoaFileTam(const std::filesystem::path& path) {
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}

}  // namespace

std::string FileManager::join(const std::string& value) {
    std::string sanitized = value;
    for (char& character : sanitized) {
        if (character == '|') character = '-';
        else if (character == '\n' || character == '\r') character = ' ';
    }
    return sanitized;
}

bool FileManager::readAllLines(const std::string& path,
                               LinkedList<std::string>& out,
                               std::string& error) const {
    return readAllLinesWithStatus(path, out, error) != FileReadStatus::Error;
}

FileReadStatus FileManager::readAllLinesWithStatus(
    const std::string& path,
    LinkedList<std::string>& out,
    std::string& error
) const {
    error.clear();
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        if (ec) {
            error = "Không thể kiểm tra file " + path + ": " + ec.message();
            return FileReadStatus::Error;
        }
        out.clear();
        return FileReadStatus::NotFound;
    }

    std::ifstream input(path);
    if (!input.is_open()) {
        error = "Không thể mở file để đọc: " + path;
        return FileReadStatus::Error;
    }

    LinkedList<std::string> loadedLines;
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        loadedLines.push_back(line);
    }
    if (input.bad() || !input.eof()) {
        error = "Lỗi khi đọc file: " + path;
        return FileReadStatus::Error;
    }

    out = std::move(loadedLines);
    return FileReadStatus::Loaded;
}

bool FileManager::writeAllLines(const std::string& path,
                                const LinkedList<std::string>& lines,
                                std::string& error) const {
    error.clear();
    if (!validateLines(lines, error)) return false;

    const std::filesystem::path destination(path);
    if (!taoThuMucCha(destination, error)) return false;

    std::filesystem::path temporary;
    if (!ghiFileTam(destination, lines, temporary, error)) return false;
    if (!thayTheFile(temporary, destination, error)) {
        xoaFileTam(temporary);
        return false;
    }
    return true;
}

bool FileManager::writeAllLinesWithBackup(
    const std::string& path,
    const LinkedList<std::string>& lines,
    const std::string& backupPath,
    std::string& error
) const {
    error.clear();
    const std::filesystem::path destination(path);
    const std::filesystem::path backup(backupPath);
    if (destination.lexically_normal() == backup.lexically_normal()) {
        error = "File dữ liệu và file backup không được trùng nhau.";
        return false;
    }
    if (!validateLines(lines, error) || !taoThuMucCha(destination, error)) {
        return false;
    }

    std::filesystem::path temporary;
    if (!ghiFileTam(destination, lines, temporary, error)) return false;

    std::error_code ec;
    const bool destinationExists = std::filesystem::exists(destination, ec);
    if (ec) {
        error = "Không thể kiểm tra file dữ liệu " + path + ": " + ec.message();
        xoaFileTam(temporary);
        return false;
    }
    if (destinationExists && !backupFile(path, backupPath, error)) {
        xoaFileTam(temporary);
        return false;
    }

    if (!thayTheFile(temporary, destination, error)) {
        xoaFileTam(temporary);
        return false;
    }
    return true;
}

bool FileManager::appendLine(const std::string& path,
                             const std::string& line,
                             std::string& error) const {
    error.clear();
    if (line.find('\n') != std::string::npos ||
        line.find('\r') != std::string::npos) {
        error = "Bản ghi cần thêm chứa ký tự xuống dòng.";
        return false;
    }

    LinkedList<std::string> lines;
    if (!readAllLines(path, lines, error)) return false;
    lines.push_back(line);
    return writeAllLines(path, lines, error);
}

bool FileManager::backupFile(const std::string& sourcePath,
                             const std::string& backupPath,
                             std::string& error) const {
    error.clear();
    const std::filesystem::path source(sourcePath);
    const std::filesystem::path backup(backupPath);
    if (source.lexically_normal() == backup.lexically_normal()) {
        error = "File nguồn và file backup không được trùng nhau.";
        return false;
    }
    if (!taoThuMucCha(backup, error)) return false;

    std::error_code ec;
    if (!std::filesystem::exists(source, ec)) {
        error = ec ? "Không thể kiểm tra file nguồn " + sourcePath + ": " +
                         ec.message()
                   : "File nguồn cần sao lưu không tồn tại: " + sourcePath;
        return false;
    }

    std::filesystem::path temporary;
    if (!saoCheFileTam(source, backup, temporary, error)) return false;
    if (!thayTheFile(temporary, backup, error)) {
        xoaFileTam(temporary);
        return false;
    }
    return true;
}

bool FileManager::restoreFile(const std::string& backupPath,
                              const std::string& targetPath,
                              std::string& error) const {
    error.clear();
    LinkedList<std::string> lines;
    const FileReadStatus status =
        readAllLinesWithStatus(backupPath, lines, error);
    if (status != FileReadStatus::Loaded) {
        if (status == FileReadStatus::NotFound) {
            error = "File backup không tồn tại: " + backupPath;
        }
        return false;
    }
    return writeAllLines(targetPath, lines, error);
}

bool FileManager::docDanhSachUser(
    std::vector<std::shared_ptr<User>>& danhSach,
    const std::string& filePath
) {
    std::string ignoredError;
    return docDanhSachUser(danhSach, filePath, ignoredError);
}

bool FileManager::docDanhSachUser(
    std::vector<std::shared_ptr<User>>& danhSach,
    const std::string& filePath,
    std::string& error
) {
    error.clear();
    FileManager manager;
    LinkedList<std::string> lines;
    const FileReadStatus status =
        manager.readAllLinesWithStatus(filePath, lines, error);
    if (status == FileReadStatus::Error) return false;
    if (status == FileReadStatus::NotFound) {
        danhSach.clear();
        return true;
    }

    std::vector<std::shared_ptr<User>> loadedAccounts;
    size_t lineNumber = 0;
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        ++lineNumber;
        const std::string& line = node->data;
        if (line.empty() || line[0] == '#') continue;

        const std::vector<std::string> parts = tachChuoi(line, '|');
        if (parts[0] == "CUSTOMER") {
            if (parts.size() < 6 || parts.size() > 8) {
                error = "Bản ghi CUSTOMER sai số trường tại dòng " +
                        std::to_string(lineNumber) + ".";
                return false;
            }
            int bookingCount = 0;
            int cancelCount = 0;
            if ((parts.size() >= 7 &&
                 !parseSoNguyenKhongAm(parts[6], bookingCount)) ||
                (parts.size() == 8 &&
                 !parseSoNguyenKhongAm(parts[7], cancelCount))) {
                error = "Thống kê CUSTOMER không hợp lệ tại dòng " +
                        std::to_string(lineNumber) + ".";
                return false;
            }
            auto customer = std::make_shared<Customer>(
                parts[1], parts[2], parts[3], parts[4], parts[5]);
            customer->setStatistics(bookingCount, cancelCount);
            loadedAccounts.push_back(customer);
        } else if (parts[0] == "OWNER") {
            if (parts.size() < 12 || parts.size() > 14) {
                error = "Bản ghi OWNER sai số trường tại dòng " +
                        std::to_string(lineNumber) + ".";
                return false;
            }
            const bool has60 = parts.size() <= 12 || parts[12] == "1";
            const bool has90 = parts.size() <= 13 || parts[13] == "1";
            if ((parts.size() > 12 && parts[12] != "0" && parts[12] != "1") ||
                (parts.size() > 13 && parts[13] != "0" && parts[13] != "1")) {
                error = "Cấu hình khung giờ OWNER không hợp lệ tại dòng " +
                        std::to_string(lineNumber) + ".";
                return false;
            }
            auto owner = std::make_shared<Owner>(
                parts[1], parts[2], parts[3], parts[4], parts[5],
                parts[6], parts[7], parts[8], parts[9], parts[10], parts[11]);
            owner->setAllowedSlotDurations(has60, has90);
            loadedAccounts.push_back(owner);
        } else {
            error = "Loại bản ghi tài khoản không hợp lệ tại dòng " +
                    std::to_string(lineNumber) + ".";
            return false;
        }
    }

    danhSach.swap(loadedAccounts);
    return true;
}

bool FileManager::ghiDanhSachUser(
    const std::vector<std::shared_ptr<User>>& danhSach,
    const std::string& filePath
) {
    std::string ignoredError;
    return ghiDanhSachUser(danhSach, filePath, ignoredError);
}

bool FileManager::ghiDanhSachUser(
    const std::vector<std::shared_ptr<User>>& danhSach,
    const std::string& filePath,
    std::string& error
) {
    error.clear();
    LinkedList<std::string> lines;
    for (size_t index = 0; index < danhSach.size(); ++index) {
        const std::shared_ptr<User>& user = danhSach[index];
        if (!user) {
            error = "Tài khoản rỗng tại vị trí " + std::to_string(index) + ".";
            return false;
        }

        if (user->get_account_type() == "Customer") {
            auto customer = std::dynamic_pointer_cast<Customer>(user);
            if (!customer) {
                error = "Đối tượng Customer không hợp lệ tại vị trí " +
                        std::to_string(index) + ".";
                return false;
            }
            if (customer->getBookingCount() < 0 ||
                customer->getCancelCount() < 0) {
                error = "Thống kê Customer âm tại vị trí " +
                        std::to_string(index) + ".";
                return false;
            }
            lines.push_back(
                "CUSTOMER|" + join(user->get_user_id()) + "|" +
                join(user->get_user_name()) + "|" +
                join(user->get_password()) + "|" + join(user->get_name()) +
                "|" + join(user->get_phone_number()) + "|" +
                std::to_string(customer->getBookingCount()) + "|" +
                std::to_string(customer->getCancelCount()));
        } else if (user->get_account_type() == "Owner") {
            auto owner = std::dynamic_pointer_cast<Owner>(user);
            if (!owner) {
                error = "Đối tượng Owner không hợp lệ tại vị trí " +
                        std::to_string(index) + ".";
                return false;
            }
            lines.push_back(
                "OWNER|" + join(user->get_user_id()) + "|" +
                join(user->get_user_name()) + "|" +
                join(user->get_password()) + "|" + join(user->get_name()) +
                "|" + join(user->get_phone_number()) + "|" +
                join(owner->get_field_name()) + "|" +
                join(owner->get_field_address()) + "|" +
                join(owner->get_field_rules()) + "|" +
                join(owner->get_open_time()) + "|" +
                join(owner->get_close_time()) + "|" +
                join(owner->get_owner_name()) + "|" +
                (owner->allows60MinuteSlots() ? "1" : "0") + "|" +
                (owner->allows90MinuteSlots() ? "1" : "0"));
        } else {
            error = "Loại tài khoản không được hỗ trợ tại vị trí " +
                    std::to_string(index) + ".";
            return false;
        }
    }

    FileManager manager;
    return manager.writeAllLines(filePath, lines, error);
}
