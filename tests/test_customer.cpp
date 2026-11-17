#include <cassert>
#include <iostream>
#include <string>

#include "Account/Customer.h"
#include "Customer/Blacklist.h"
#include "Customer/Discount.h"
#include "Customer/Review.h"
#include "Repository/BookingRepository.h"
#include "Repository/CustomerRepository.h"
#include "Service/BookingService.h"
#include "Service/CustomerService.h"
#include "Storage/FileManager.h"

// Giả lập FileManager gặp lỗi đọc tệp (I/O error, permission denied, file hỏng)
class FailingFileManager : public FileManager {
public:
    bool readAllLines(const std::string& path, LinkedList<std::string>& out,
                      std::string& error) const override {
        (void)path;
        out.clear();
        error = "Loi doc tap tin he thong: I/O failure";
        return false;
    }
};

// Giả lập CustomerRepository gặp lỗi kết nối hoặc truy vấn cơ sở dữ liệu
class FailingCustomerRepository : public CustomerRepository {
public:
    explicit FailingCustomerRepository(FileManager& fm) : CustomerRepository(fm) {}

    bool findActiveBlacklist(const std::string& customerId, Blacklist& out,
                             std::string& error) const override {
        (void)customerId;
        (void)out;
        error = "Loi ket noi co so du lieu khach hang";
        return false;
    }
};

int main() {
    std::cout << "=== RUNNING CUSTOMER SERVICE & REPOSITORY TESTS ===\n";
    FileManager fileManager;
    CustomerRepository customerRepo(fileManager);
    CustomerService customerService(customerRepo);

    // 1. Test CustomerRepository: findCustomerByUserId
    Customer cust;
    std::string err;
    bool found = customerRepo.findCustomerByUserId("US02", cust, err);
    assert(found);
    assert(cust.get_user_id() == "US02");
    assert(cust.get_phone_number() == "0909876543");

    // 2. Test CustomerService: isBlacklisted
    // 2.1. Khách hàng không bị blacklist
    bool isBlocked = true;
    std::string reason = "initial";
    std::string checkErr;
    assert(customerService.isBlacklisted("US02", isBlocked, reason, checkErr));
    assert(!isBlocked);
    assert(reason.empty());
    assert(checkErr.empty());

    // 2.2. Khách hàng đang bị blacklist (Phone 0944112233 có trong data/blacklist.txt với status 1)
    isBlocked = false;
    reason.clear();
    assert(customerService.isBlacklisted("0944112233", isBlocked, reason, checkErr));
    assert(isBlocked);
    assert(!reason.empty());
    assert(checkErr.empty());

    // 2.3. Blacklist đã hết hiệu lực (active = false)
    const std::string expiredPhone = "0988000111";
    Blacklist expiredBl(expiredPhone, "Het han khoa", "01/01/2026", false);
    assert(customerRepo.saveBlacklist(expiredBl, err));
    isBlocked = true;
    reason = "initial";
    assert(customerService.isBlacklisted(expiredPhone, isBlocked, reason, checkErr));
    assert(!isBlocked);
    assert(reason.empty());
    assert(checkErr.empty());

    // 2.4. User ID không tồn tại
    isBlocked = true;
    reason = "initial";
    assert(customerService.isBlacklisted("US_NONEXISTENT_9999", isBlocked, reason, checkErr));
    assert(!isBlocked);
    assert(reason.empty());
    assert(checkErr.empty());

    // 2.5. File dữ liệu bị lỗi (FailingFileManager)
    FailingFileManager failingFileManager;
    CustomerRepository failingFileRepo(failingFileManager);
    CustomerService failingFileCustomerService(failingFileRepo);
    isBlocked = true;
    reason = "initial";
    std::string fileErr;
    bool queryOk = failingFileCustomerService.isBlacklisted("US02", isBlocked, reason, fileErr);
    assert(!queryOk); // Truy vấn thất bại
    assert(!isBlocked); // Không biến lỗi truy xuất thành kết quả nghiệp vụ bị khóa
    assert(fileErr == "Loi doc tap tin he thong: I/O failure"); // Error truyền lên chính xác

    // 2.6. Repository trả về lỗi (FailingCustomerRepository)
    FailingCustomerRepository failingRepo(fileManager);
    CustomerService failingRepoCustomerService(failingRepo);
    isBlocked = true;
    reason = "initial";
    std::string repoErr;
    queryOk = failingRepoCustomerService.isBlacklisted("US02", isBlocked, reason, repoErr);
    assert(!queryOk); // Truy vấn thất bại
    assert(!isBlocked);
    assert(repoErr == "Loi ket noi co so du lieu khach hang"); // Error truyền lên chính xác

    // 2.7. Đảm bảo BookingService KHÔNG cho phép đặt sân khi không thể xác minh blacklist
    BookingRepository bookingRepo(fileManager);
    BookingService bookingServiceWithFailingCustomer(bookingRepo, failingFileCustomerService);
    VenueBookingRequest req;
    req.customerId = "US02";
    req.customerName = "Tran Van Thue";
    req.customerPhone = "0909876543";
    req.fieldId = "S5-01";
    req.date = "2026-12-25";
    req.startTime = "08:00";
    req.durationMinutes = 60;
    std::string bookingId, bookingErr;
    bool booked = bookingServiceWithFailingCustomer.createBooking(req, bookingId, bookingErr);
    assert(!booked); // Phải từ chối đặt sân khi không thể kiểm tra blacklist
    assert(!bookingErr.empty());
    assert(bookingErr.find("Loi doc tap tin he thong: I/O failure") != std::string::npos);

    // 2.8. Đảm bảo BookingService từ chối đặt sân khi khách hàng bị blacklist
    BookingService normalBookingService(bookingRepo, customerService);
    req.customerId = "0944112233"; // khách bị blacklist
    req.customerPhone = "0944112233";
    booked = normalBookingService.createBooking(req, bookingId, bookingErr);
    assert(!booked);
    assert(bookingErr.find("khoa do vi pham") != std::string::npos);

    // 3. Test CustomerService: recordCancellation and recordSuccessfulBooking
    int initialCancel = cust.getCancelCount();
    int initialBooking = cust.getBookingCount();
    assert(customerService.recordCancellation("US02", err));
    assert(customerRepo.findCustomerByUserId("US02", cust, err));
    assert(cust.getCancelCount() == initialCancel + 1);

    assert(customerService.recordSuccessfulBooking("US02", err));
    assert(customerRepo.findCustomerByUserId("US02", cust, err));
    assert(cust.getBookingCount() == initialBooking + 1);

    // Revert statistics for US02 back to initial to keep data clean
    cust.setStatistics(initialBooking, initialCancel);
    customerRepo.updateCustomer(cust, err);

    // Dọn dẹp bản ghi blacklist hết hạn dùng trong kiểm thử
    LinkedList<std::string> bLines;
    fileManager.readAllLines("data/blacklist.txt", bLines, err);
    bLines.removeIf([&](const std::string& line) {
        return line.rfind(expiredPhone + "|", 0) == 0;
    });
    fileManager.writeAllLines("data/blacklist.txt", bLines, err);

    // 4. Test CustomerService: addReview validation
    Review badRating("RV99", "US02", "S5-01", "BK-0001", 6, "2026-11-06", "Good field");
    assert(!customerService.addReview(badRating, err));
    assert(!err.empty());

    Review emptyComment("RV99", "US02", "S5-01", "BK-0001", 5, "2026-11-06", "");
    assert(!customerService.addReview(emptyComment, err));
    assert(!err.empty());

    std::cout << "ALL CUSTOMER REPOSITORY & SERVICE TESTS PASSED!\n";
    return 0;
}