#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "Account/Customer.h"
#include "Booking/Booking.h"
#include "Booking/BookingDetail.h"
#include "Core/LinkedList.h"
#include "Repository/BookingRepository.h"
#include "Repository/CustomerRepository.h"
#include "Repository/VenueRepository.h"
#include "Service/BookingService.h"
#include "Service/CustomerService.h"
#include "Service/VenueService.h"
#include "Storage/FileManager.h"

// RAII Guard de bao dam toan ven du lieu file goc truoc va sau khi chay test
class StorageBackupGuard {
private:
    std::string originalBookings_;
    std::string originalReplace_;
    std::string originalBackup_;

    static std::string readFile(const std::string& path) {
        std::ifstream f(path, std::ios::binary);
        if (!f.is_open()) return "";
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    static void writeFile(const std::string& path, const std::string& content) {
        std::ofstream f(path, std::ios::binary | std::ios::trunc);
        f << content;
    }

public:
    StorageBackupGuard() {
        originalBookings_ = readFile("data/bookings.txt");
        originalReplace_ = readFile("data/bookings.txt.replace");
        originalBackup_ = readFile("data/bookings.txt.bak");
    }

    ~StorageBackupGuard() {
        restore();
    }

    void restore() {
        writeFile("data/bookings.txt", originalBookings_);
        writeFile("data/bookings.txt.replace", originalReplace_);
        writeFile("data/bookings.txt.bak", originalBackup_);
    }

    std::string currentBookings() const { return readFile("data/bookings.txt"); }
};

void runConsistencyTests() {
    StorageBackupGuard guard;

    FileManager fileManager;
    CustomerRepository customerRepo(fileManager);
    CustomerService customerService(customerRepo);
    VenueRepository venueRepository(fileManager);
    VenueService venueService(venueRepository);
    BookingRepository bookingRepo(venueRepository);
    BookingService bookingService(bookingRepo, customerService, venueService);

    const std::string testField = "S5-04";
    const std::string testCustId = "US02";
    const std::string testCustName = "Trần Văn Thuê";
    const std::string testCustPhone = "0909876543";

    // =========================================================================
    // TEST 1: Tạo một booking thành công
    // =========================================================================
    std::cout << "[TEST 1] Tao mot booking thanh cong...\n";
    {
        guard.restore();
        VenueBookingRequest req;
        req.customerId = testCustId;
        req.customerName = testCustName;
        req.customerPhone = testCustPhone;
        req.fieldId = testField;
        req.date = "2026-11-20";
        req.startTime = "08:00";
        req.durationMinutes = 60;

        std::string bookingId, err;
        bool ok = bookingService.createBooking(req, bookingId, err);
        assert(ok && "createBooking that bai!");
        assert(!bookingId.empty() && "bookingId khong duoc rong!");

        // Kiem tra don trong Bookings
        LinkedList<VenueBookingRecord> records;
        assert(bookingRepo.loadAll(records, err));
        bool foundBooking = false;
        for (auto* n = records.get_head(); n != nullptr; n = n->next) {
            if (n->data.bookingId == bookingId) {
                foundBooking = true;
                assert(n->data.status == "CONFIRMED");
                assert(n->data.priceVnd == 250000);
                break;
            }
        }
        assert(foundBooking && "Booking khong duoc tim thay trong kho luu tru!");
        std::cout << "  -> Dat don thanh cong: " << bookingId << "\n";
    }

    // =========================================================================
    // TEST 2: Tao nhieu booking thanh cong trong mot batch
    // =========================================================================
    std::cout << "[TEST 2] Tao nhieu booking thanh cong (batch consecutive)...\n";
    {
        guard.restore();
        LinkedList<VenueBookingRequest> batchReqs;
        VenueBookingRequest r1;
        r1.customerId = testCustId; r1.customerName = testCustName; r1.customerPhone = testCustPhone;
        r1.fieldId = testField; r1.date = "2026-11-21"; r1.startTime = "08:00"; r1.durationMinutes = 60;
        batchReqs.push_back(r1);

        VenueBookingRequest r2;
        r2.customerId = testCustId; r2.customerName = testCustName; r2.customerPhone = testCustPhone;
        r2.fieldId = testField; r2.date = "2026-11-21"; r2.startTime = "09:00"; r2.durationMinutes = 60;
        batchReqs.push_back(r2);

        LinkedList<std::string> bIds;
        std::string err;
        bool ok = bookingService.createBookings(batchReqs, bIds, err);
        assert(ok && "createBookings batch that bai!");
        assert(bIds.size() == 2 && "So luong bookingIds tao ra phai bang 2!");

        // Kiểm tra cả hai booking được lưu với giá chuẩn.
        for (auto* n = bIds.get_head(); n != nullptr; n = n->next) {
            Booking loadedBooking;
            assert(bookingRepo.findById(n->data, loadedBooking, err));
            assert(loadedBooking.calculateTotalVnd() == 250000);
        }
        std::cout << "  -> Batch thanh cong 2 ca lien tiep.\n";
    }

    // =========================================================================
    // TEST 3: Rollback batch khi ca thứ hai trùng lịch
    // =========================================================================
    std::cout << "[TEST 3] Rollback batch khi ca thứ hai trùng lịch...\n";
    {
        guard.restore();
        // Tao truoc 1 don tai 09:00 ngay 2026-11-22
        VenueBookingRequest preReq;
        preReq.customerId = testCustId; preReq.customerName = testCustName; preReq.customerPhone = testCustPhone;
        preReq.fieldId = testField; preReq.date = "2026-11-22"; preReq.startTime = "09:00"; preReq.durationMinutes = 60;
        std::string existingId, err;
        assert(bookingService.createBooking(preReq, existingId, err));

        const std::string bookingsSnapshot = guard.currentBookings();

        // Gui yeu cau batch gom 2 ca: 08:00 (trong) va 09:00 (da bi chiem)
        LinkedList<VenueBookingRequest> batchReqs;
        VenueBookingRequest r1 = preReq; r1.startTime = "08:00";
        VenueBookingRequest r2 = preReq; r2.startTime = "09:00";
        batchReqs.push_back(r1);
        batchReqs.push_back(r2);

        LinkedList<std::string> bIds;
        bool ok = bookingService.createBookings(batchReqs, bIds, err);
        assert(!ok && "Yeu cau trung lich phai bi tu choi!");
        assert(bIds.isEmpty() && "Danh sach bIds phai rong sau khi tu choi!");

        // Dam bao du lieu nguyen ven: ca 08:00 KHONG bi ghi do dang
        assert(guard.currentBookings() == bookingsSnapshot);
        std::cout << "  -> Batch được rollback nguyên vẹn khi ca thứ hai trùng lịch.\n";
    }

    std::cout << "[TEST 3B] Batch tu choi khi thong tin khach khong dong nhat...\n";
    {
        guard.restore();
        const std::string bookingsBefore = guard.currentBookings();

        VenueBookingRequest firstRequest;
        firstRequest.customerId = testCustId;
        firstRequest.customerName = testCustName;
        firstRequest.customerPhone = testCustPhone;
        firstRequest.fieldId = testField;
        firstRequest.date = "2026-11-22";
        firstRequest.startTime = "10:00";
        firstRequest.durationMinutes = 60;

        VenueBookingRequest secondRequest = firstRequest;
        secondRequest.startTime = "11:00";
        secondRequest.customerPhone = "0900000000";

        LinkedList<VenueBookingRequest> batchRequests;
        batchRequests.push_back(firstRequest);
        batchRequests.push_back(secondRequest);
        LinkedList<std::string> bookingIds;
        std::string error;
        assert(!bookingService.createBookings(batchRequests, bookingIds, error));
        assert(bookingIds.isEmpty());
        assert(guard.currentBookings() == bookingsBefore);
    }

    std::cout << "\n>>> BOOKING CONSISTENCY TESTS PASSED! <<<\n";
}

int main() {
    std::cout << "=== CORE BUSINESS INTEGRATION: BOOKING AND PAYMENT CONSISTENCY ===\n";
    try {
        runConsistencyTests();
    } catch (const std::exception& ex) {
        std::cerr << "EXCEPTION: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
