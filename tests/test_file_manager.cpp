#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "Account/Customer.h"
#include "Repository/CustomerRepository.h"
#include "Repository/TournamentRepository.h"
#include "Storage/FileManager.h"
#include "Tournament/Tournament.h"

namespace {

class FailingReadFileManager : public FileManager {
public:
    mutable int writeCalls = 0;

    bool readAllLines(const std::string& path, LinkedList<std::string>& out,
                      std::string& error) const override {
        (void)path;
        out.clear();
        error = "Simulated read failure";
        return false;
    }

    bool writeAllLines(const std::string& path,
                       const LinkedList<std::string>& lines,
                       std::string& error) const override {
        (void)path;
        (void)lines;
        ++writeCalls;
        error.clear();
        return true;
    }
};

class CurrentDirectoryGuard {
private:
    std::filesystem::path original_;
    std::filesystem::path temporary_;

public:
    CurrentDirectoryGuard()
        : original_(std::filesystem::current_path()),
          temporary_(std::filesystem::temp_directory_path() /
                     ("pbl2-file-manager-" +
                      std::to_string(std::chrono::steady_clock::now()
                                         .time_since_epoch()
                                         .count()))) {
        std::filesystem::create_directories(temporary_);
        std::filesystem::current_path(temporary_);
    }

    ~CurrentDirectoryGuard() {
        std::filesystem::current_path(original_);
        std::error_code error;
        std::filesystem::remove_all(temporary_, error);
    }
};

std::string readFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(input),
                       std::istreambuf_iterator<char>());
}

void writeFile(const std::filesystem::path& path, const std::string& value) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << value;
    assert(output.good());
}

}  // namespace

int main() {
    CurrentDirectoryGuard directory;
    FileManager manager;
    std::string error;

    const std::filesystem::path target = "data/records.txt";
    LinkedList<std::string> originalLines;
    originalLines.push_back("old|record");
    assert(manager.writeAllLines(target.string(), originalLines, error));
    const std::string originalContent = readFile(target);

    // Windows CRLF and Unix LF must produce identical logical records.
    writeFile("data/crlf.txt", "first|record\r\nsecond|record\nlast|record");
    LinkedList<std::string> normalized;
    assert(manager.readAllLines("data/crlf.txt", normalized, error));
    auto* line = normalized.get_head();
    assert(line && line->data == "first|record");
    line = line->next;
    assert(line && line->data == "second|record");
    line = line->next;
    assert(line && line->data == "last|record" && !line->next);

    LinkedList<std::string> invalidLines;
    invalidLines.push_back("first line\ninjected record");
    assert(!manager.writeAllLines(target.string(), invalidLines, error));
    assert(readFile(target) == originalContent);
    assert(!error.empty());

    LinkedList<std::string> appendedLines;
    appendedLines.push_back("second line\r\ninjected record");
    assert(!manager.appendLine(target.string(), appendedLines.front(), error));
    assert(readFile(target) == originalContent);
    assert(!error.empty());

    const std::filesystem::path blockedParent = "not-a-directory";
    writeFile(blockedParent, "blocker");
    assert(!manager.writeAllLinesWithBackup(
        target.string(), originalLines,
        (blockedParent / "backup.txt").string(), error));
    assert(readFile(target) == originalContent);
    assert(!error.empty());

    assert(manager.appendLine(target.string(), "next|record", error));
    LinkedList<std::string> appendedResult;
    assert(manager.readAllLines(target.string(), appendedResult, error));
    assert(appendedResult.size() == 2);
    assert(appendedResult.get_tail()->data == "next|record");
    assert(manager.writeAllLines(target.string(), originalLines, error));

    bool writeFailed = false;
    bool threw = false;
    try {
        writeFailed = !manager.writeAllLines(
            (blockedParent / "records.txt").string(), originalLines, error);
    } catch (...) {
        threw = true;
    }
    assert(!threw);
    assert(writeFailed);
    assert(!error.empty());

    const std::filesystem::path accountsPath = "data/accounts-invalid.txt";
    writeFile(accountsPath, "CUSTOMER|US01|user|pass|Tên|0900000000|1|0\n"
                            "UNKNOWN|malformed\n");
    std::vector<std::shared_ptr<User>> accounts;
    accounts.push_back(std::make_shared<Customer>(
        "KEEP", "keep", "pw", "Giữ nguyên", "0900000001"));
    assert(!FileManager::docDanhSachUser(accounts, accountsPath.string()));
    assert(accounts.size() == 1);
    assert(accounts[0]->get_user_id() == "KEEP");

    writeFile(accountsPath, "CUSTOMER|MISSING|only|four|fields\n");
    assert(!FileManager::docDanhSachUser(accounts, accountsPath.string(), error));
    assert(accounts.size() == 1);
    assert(accounts[0]->get_user_id() == "KEEP");
    assert(!error.empty());

    writeFile(accountsPath, "CUSTOMER|BAD-COUNT|user|pass|Tên|0900000000|x|0\n");
    assert(!FileManager::docDanhSachUser(accounts, accountsPath.string(), error));
    assert(accounts.size() == 1);
    assert(accounts[0]->get_user_id() == "KEEP");
    assert(!error.empty());

    auto invalidCustomer = std::make_shared<Customer>(
        "INVALID", "invalid", "pw", "Invalid", "0900000005");
    invalidCustomer->setStatistics(-1, 0);
    std::vector<std::shared_ptr<User>> invalidAccounts{invalidCustomer};
    const std::string accountsBeforeInvalidSave = readFile(accountsPath);
    assert(!FileManager::ghiDanhSachUser(
        invalidAccounts, accountsPath.string(), error));
    assert(readFile(accountsPath) == accountsBeforeInvalidSave);
    assert(!error.empty());

    const std::filesystem::path missingData = "data/not-created.txt";
    LinkedList<std::string> loadedLines;
    loadedLines.push_back("stale");
    assert(manager.readAllLinesWithStatus(
               missingData.string(), loadedLines, error) ==
           FileReadStatus::NotFound);
    assert(loadedLines.isEmpty());

    const std::filesystem::path emptyData = "data/empty.txt";
    writeFile(emptyData, "");
    assert(manager.readAllLinesWithStatus(
               emptyData.string(), loadedLines, error) ==
           FileReadStatus::Loaded);
    assert(loadedLines.isEmpty());

    const std::filesystem::path readDirectory = "data/read-directory";
    std::filesystem::create_directories(readDirectory);
    loadedLines.push_back("keep on read failure");
    assert(manager.readAllLinesWithStatus(
               readDirectory.string(), loadedLines, error) ==
           FileReadStatus::Error);
    assert(loadedLines.size() == 1);
    assert(loadedLines.front() == "keep on read failure");
    assert(!error.empty());

    LinkedList<std::string> unicodeLines;
    unicodeLines.push_back(u8"Tiếng Việt — Sân bóng");
    unicodeLines.push_back(FileManager::join(u8"Tên|sân\r\nmới"));
    unicodeLines.push_back("");
    const std::filesystem::path unicodePath = "data/unicode.txt";
    assert(manager.writeAllLines(unicodePath.string(), unicodeLines, error));
    LinkedList<std::string> roundTripLines;
    assert(manager.readAllLines(unicodePath.string(), roundTripLines, error));
    assert(roundTripLines.size() == 3);
    assert(roundTripLines.get_head()->data == u8"Tiếng Việt — Sân bóng");
    assert(roundTripLines.get_head()->next->data == u8"Tên-sân  mới");
    assert(roundTripLines.get_tail()->data.empty());

    const std::filesystem::path accountsRoundTripPath =
        "data/accounts-round-trip.txt";
    auto customer = std::make_shared<Customer>(
        "US-VN", "khach", u8"mật khẩu", u8"Tên|có dấu", "0900000002");
    customer->setStatistics(8, 2);
    std::vector<std::shared_ptr<User>> accountsToSave{customer};
    assert(FileManager::ghiDanhSachUser(
        accountsToSave, accountsRoundTripPath.string(), error));
    std::vector<std::shared_ptr<User>> loadedAccounts;
    assert(FileManager::docDanhSachUser(
        loadedAccounts, accountsRoundTripPath.string(), error));
    assert(loadedAccounts.size() == 1);
    auto loadedCustomer =
        std::dynamic_pointer_cast<Customer>(loadedAccounts.front());
    assert(loadedCustomer);
    assert(loadedCustomer->get_name() == u8"Tên-có dấu");
    assert(loadedCustomer->get_password() == u8"mật khẩu");
    assert(loadedCustomer->getBookingCount() == 8);
    assert(loadedCustomer->getCancelCount() == 2);

    const std::filesystem::path legacyAccountsPath =
        "data/accounts-legacy.txt";
    writeFile(legacyAccountsPath,
              "CUSTOMER|OLD|old-user|pw|Tên cũ|0900000003\n"
              "OWNER|OLD-OWNER|old-owner|pw|Chủ cũ|0900000004|Sân cũ|Địa chỉ|Quy định|06:00|23:00|Chủ cũ\n");
    assert(FileManager::docDanhSachUser(
        loadedAccounts, legacyAccountsPath.string(), error));
    assert(loadedAccounts.size() == 2);
    loadedCustomer = std::dynamic_pointer_cast<Customer>(loadedAccounts[0]);
    assert(loadedCustomer);
    assert(loadedCustomer->getBookingCount() == 0);

    const std::filesystem::path backupPath = "data/records.txt.replace";
    LinkedList<std::string> updatedLines;
    updatedLines.push_back(u8"Dữ liệu mới");
    assert(manager.writeAllLinesWithBackup(
        target.string(), updatedLines, backupPath.string(), error));
    assert(readFile(backupPath) == originalContent);
    assert(readFile(target) != originalContent);
    assert(manager.restoreFile(backupPath.string(), target.string(), error));
    assert(readFile(target) == originalContent);

    const std::string beforeMissingRestore = readFile(target);
    assert(!manager.restoreFile("data/no-backup.txt", target.string(), error));
    assert(readFile(target) == beforeMissingRestore);
    assert(!error.empty());
    assert(!manager.restoreFile(
        readDirectory.string(), target.string(), error));
    assert(readFile(target) == beforeMissingRestore);
    assert(!error.empty());

    const std::filesystem::path missingAccounts = "data/missing-accounts.txt";
    std::vector<std::shared_ptr<User>> emptyAccounts;
    assert(FileManager::docDanhSachUser(emptyAccounts, missingAccounts.string()));
    assert(emptyAccounts.empty());
    assert(!std::filesystem::exists(missingAccounts));

    const std::filesystem::path emptyAccountsPath = "data/empty-accounts.txt";
    writeFile(emptyAccountsPath, "");
    assert(FileManager::docDanhSachUser(
        emptyAccounts, emptyAccountsPath.string(), error));
    assert(emptyAccounts.empty());
    assert(readFile(emptyAccountsPath).empty());

    const std::filesystem::path tournamentsPath = "data/tournaments.txt";
    writeFile(tournamentsPath,
              "TR-OK|Valid|Owner|US01|2026-11-14|2026-11-15|F1|"
              "08:00-10:00|Note||2\n"
              "TR-OLD|Old|Owner|US01|2026-11-14|2026-11-15|F1\n");
    TournamentRepository tournamentRepository(manager);
    LinkedList<Tournament> tournaments;
    assert(tournamentRepository.loadAll(tournaments, error));
    assert(tournaments.size() == 2);
    assert(tournaments.front().getTournamentId() == "TR-OK");

    const std::string malformedTournament =
        "TR-BAD|Tên giải|Người yêu cầu|US01|2026-11-14|2026-11-15|F1|"
        "08:00-10:00|Ghi chú||invalid-status\n";
    writeFile(tournamentsPath, malformedTournament);
    tournaments.clear();
    tournaments.push_back(Tournament("KEEP", "US01", "Giải giữ nguyên"));
    assert(!tournamentRepository.loadAll(tournaments, error));
    assert(tournaments.size() == 1);
    assert(tournaments.front().getTournamentId() == "KEEP");
    assert(!tournamentRepository.save(
        Tournament("NEW", "US01", "Giải mới"), error));
    assert(readFile(tournamentsPath) == malformedTournament);

    FailingReadFileManager failingFileManager;
    CustomerRepository customerRepository(failingFileManager);
    assert(!customerRepository.recordBookingId(
        "BK-FAIL", "US01", "BOOKING", error));
    assert(error == "Simulated read failure");
    assert(failingFileManager.writeCalls == 0);
    assert(!failingFileManager.appendLine("ignored", "new", error));
    assert(error == "Simulated read failure");
    assert(failingFileManager.writeCalls == 0);

    std::cout << "FileManager regression tests passed.\n";
    return 0;
}
