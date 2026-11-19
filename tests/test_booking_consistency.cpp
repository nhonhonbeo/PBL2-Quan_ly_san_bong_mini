#include <cassert>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "Account/Customer.h"
#include "Booking/Booking.h"
#include "Booking/BookingDetail.h"
#include "Core/DateTime.h"
#include "Core/LinkedList.h"
#include "Payment/Invoice.h"
#include "Payment/Payment.h"
#include "Repository/BookingRepository.h"
#include "Repository/CustomerRepository.h"
#include "Repository/PaymentRepository.h"
#include "Repository/VenueRepository.h"
#include "Service/BookingService.h"
#include "Service/CustomerService.h"
#include "Service/PaymentService.h"
#include "Service/VenueService.h"
#include "Storage/FileManager.h"

// RAII Guard de bao dam toan ven du lieu file goc truoc va sau khi chay test
class StorageBackupGuard {
private:
    std::string originalBookings_;
    std::string originalInvoices_;
    std::string originalPayments_;
    std::string originalReplace_;

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
        originalInvoices_ = readFile("data/invoices.txt");
        originalPayments_ = readFile("data/payments.txt");
        originalReplace_ = readFile("data/bookings.txt.replace");
    }

    ~StorageBackupGuard() {
        restore();
    }

    void restore() {
        writeFile("data/bookings.txt", originalBookings_);
        writeFile("data/bookings.txt.replace", originalReplace_);
        writeFile("data/invoices.txt", originalInvoices_);
        writeFile("data/payments.txt", originalPayments_);
    }

    std::string currentBookings() const { return readFile("data/bookings.txt"); }
    std::string currentInvoices() const { return readFile("data/invoices.txt"); }
    std::string currentPayments() const { return readFile("data/payments.txt"); }

    const std::string& getOriginalBookings() const { return originalBookings_; }
    const std::string& getOriginalInvoices() const { return originalInvoices_; }
};

// Mock PaymentService de mo phong loi tao hoa don tai cac buoc cu the
class MockFailingPaymentService : public PaymentService {
public:
    int callCount = 0;
    int failOnCallIndex = -1; // 1-based index that bai, -1 la khong bao gio fail
    bool alwaysFail = false;
    LinkedList<std::string> rollbackCalls;

    explicit MockFailingPaymentService(PaymentRepository& repo)
        : PaymentService(repo) {}

    bool createInvoice(const std::string& bookingId, long long amountVnd,
                       std::string& outInvoiceId, std::string& error) override {
        callCount++;
        if (alwaysFail || callCount == failOnCallIndex) {
            error = "Mo phong loi PaymentService: khong the tao hoa don cho " + bookingId;
            return false;
        }
        return PaymentService::createInvoice(bookingId, amountVnd, outInvoiceId, error);
    }

    bool rollbackInvoice(const std::string& bookingId, std::string& error) override {
        rollbackCalls.push_back(bookingId);
        return PaymentService::rollbackInvoice(bookingId, error);
    }
};

// Mock FileManager de mo phong loi ghi file I/O
class MockFailingFileManager : public FileManager {
public:
    bool failOnWrite = true;
    bool writeAllLines(const std::string& path,
                       const LinkedList<std::string>& lines,
                       std::string& error) const override {
        (void)path; (void)lines;
        if (failOnWrite) {
            error = "Loi ghi dia I/O: Khong the ghi file du lieu.";
            return false;
        }
        return FileManager::writeAllLines(path, lines, error);
    }
};

std::string normalizeLineEndings(std::string content) {
    content.erase(
        std::remove(content.begin(), content.end(), '\r'),
        content.end()
    );
    return content;
}

void runConsistencyTests() {
    StorageBackupGuard guard;

    FileManager fileManager;
    CustomerRepository customerRepo(fileManager);
    CustomerService customerService(customerRepo);
    VenueRepository venueRepository(fileManager);
    VenueService venueService(venueRepository);
    BookingRepository bookingRepo(venueRepository);
    PaymentRepository paymentRepo(fileManager);
    PaymentService paymentService(paymentRepo);
    BookingService bookingService(bookingRepo, customerService, venueService,
                                  paymentService);

    const std::string testField = "S5-04";
    const std::string testCustId = "US02";
    const std::string testCustName = "Trần Văn Thuê";
    const std::string testCustPhone = "0909876543";

    // =========================================================================
    // TEST 1: Tao mot booking thanh cong & kiem tra lien ket hoa don
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
        long long bookingPrice = 0;
        for (auto* n = records.get_head(); n != nullptr; n = n->next) {
            if (n->data.bookingId == bookingId) {
                foundBooking = true;
                bookingPrice = n->data.priceVnd;
                assert(n->data.status == "CONFIRMED");
                assert(bookingPrice == 250000); // Gia cau hinh san 5 luc 08:00
                break;
            }
        }
        assert(foundBooking && "Booking khong duoc tim thay trong kho luu tru!");

        // Kiem tra Hoa don tuong ung duoc tao chinh xac
        Invoice inv;
        assert(paymentService.getInvoiceByBookingId(bookingId, inv, err));
        assert(inv.getBookingId() == bookingId);
        assert(inv.getTotalVnd() == bookingPrice);
        assert(inv.getStatus() == InvoiceStatus::UNPAID);
        assert(inv.getTotalVnd() > 0 && "Tong tien hoa don phai > 0!");
        std::cout << "  -> Dat don thanh cong: " << bookingId << " | Hoa don: "
                  << inv.getInvoiceId() << " | Tong: " << inv.getTotalVnd() << " VND\n";
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

        // Kiem tra ca 2 don deu co trong Bookings va co hoa don tuong ung voi gia chuan
        for (auto* n = bIds.get_head(); n != nullptr; n = n->next) {
            const std::string& bId = n->data;
            Invoice inv;
            assert(paymentService.getInvoiceByBookingId(bId, inv, err));
            assert(inv.getBookingId() == bId);
            assert(inv.getTotalVnd() == 250000);
            assert(inv.getStatus() == InvoiceStatus::UNPAID);
        }
        std::cout << "  -> Batch thanh cong 2 ca lien tiep voi day du hoa don hop le.\n";
    }

    // =========================================================================
    // TEST 3: Booking thu hai that bai o buoc Staging / Pre-validation
    // =========================================================================
    std::cout << "[TEST 3] Booking thu hai that bai (Xung dot lich o buoc pre-validation)...\n";
    {
        guard.restore();
        // Tao truoc 1 don tai 09:00 ngay 2026-11-22
        VenueBookingRequest preReq;
        preReq.customerId = testCustId; preReq.customerName = testCustName; preReq.customerPhone = testCustPhone;
        preReq.fieldId = testField; preReq.date = "2026-11-22"; preReq.startTime = "09:00"; preReq.durationMinutes = 60;
        std::string existingId, err;
        assert(bookingService.createBooking(preReq, existingId, err));

        const std::string bookingsSnapshot = guard.currentBookings();
        const std::string invoicesSnapshot = guard.currentInvoices();

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
        assert(guard.currentInvoices() == invoicesSnapshot);
        std::cout << "  -> Pre-validation chan thanh cong ca thu 2 trung lich, 0 byte du thua bi ghi.\n";
    }

    std::cout << "[TEST 3B] Batch tu choi khi thong tin khach khong dong nhat...\n";
    {
        guard.restore();
        const std::string bookingsBefore = guard.currentBookings();
        const std::string invoicesBefore = guard.currentInvoices();

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
        assert(guard.currentInvoices() == invoicesBefore);
    }

    // =========================================================================
    // TEST 4: Booking thu hai that bai trong commit -> Rollback booking 1 & invoice 1
    // =========================================================================
    std::cout << "[TEST 4] Booking thu hai that bai trong commit -> Rollback booking 1 & invoice 1...\n";
    {
        guard.restore();
        const std::string bookingsBefore = guard.currentBookings();
        const std::string invoicesBefore = guard.currentInvoices();

        // Su dung MockPaymentService de lam hoa don thu 2 that bai
        MockFailingPaymentService mockPay(paymentRepo);
        mockPay.failOnCallIndex = 2; // Booking 1 tao invoice thanh cong, Booking 2 fail
        BookingService testService(bookingRepo, customerService, venueService,
                                   mockPay);

        LinkedList<VenueBookingRequest> batchReqs;
        VenueBookingRequest r1;
        r1.customerId = testCustId; r1.customerName = testCustName; r1.customerPhone = testCustPhone;
        r1.fieldId = testField; r1.date = "2026-11-23"; r1.startTime = "08:00"; r1.durationMinutes = 60;
        batchReqs.push_back(r1);

        VenueBookingRequest r2;
        r2.customerId = testCustId; r2.customerName = testCustName; r2.customerPhone = testCustPhone;
        r2.fieldId = testField; r2.date = "2026-11-23"; r2.startTime = "09:00"; r2.durationMinutes = 60;
        batchReqs.push_back(r2);

        LinkedList<std::string> bIds;
        std::string err;
        bool ok = testService.createBookings(batchReqs, bIds, err);
        assert(!ok && "Batch phai that bai khi invoice 2 loi!");
        assert(bIds.isEmpty() && "bookingIds phai bi xoa sach!");
        assert(mockPay.rollbackCalls.size() == 1 && "Phai goi rollbackInvoice cho don dat san 1!");

        assert(normalizeLineEndings(guard.currentBookings()) ==
               normalizeLineEndings(bookingsBefore) &&
               "Bookings khong duoc khoi phuc sau rollback!");
        assert(normalizeLineEndings(guard.currentInvoices()) ==
               normalizeLineEndings(invoicesBefore) &&
               "Invoices khong duoc khoi phuc sau rollback!");
        std::cout << "  -> Rollback ca dat san 1 va hoa don 1 thanh cong khi ca 2 that bai.\n";
    }

    // =========================================================================
    // TEST 5: Booking cuoi cung that bai trong batch 3 ca -> Rollback toan bo
    // =========================================================================
    std::cout << "[TEST 5] Booking cuoi cung that bai (3 ca) -> Rollback toan bo...\n";
    {
        guard.restore();
        const std::string bookingsBefore = guard.currentBookings();
        const std::string invoicesBefore = guard.currentInvoices();

        MockFailingPaymentService mockPay(paymentRepo);
        mockPay.failOnCallIndex = 3; // Ca 1 va 2 thanh cong, ca 3 fail invoice
        BookingService testService(bookingRepo, customerService, venueService,
                                   mockPay);

        LinkedList<VenueBookingRequest> batchReqs;
        for (int i = 0; i < 3; ++i) {
            VenueBookingRequest r;
            r.customerId = testCustId; r.customerName = testCustName; r.customerPhone = testCustPhone;
            r.fieldId = testField; r.date = "2026-11-24";
            r.durationMinutes = 60;
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%02d:00", 13 + i);
            r.startTime = buf;
            batchReqs.push_back(r);
        }

        LinkedList<std::string> bIds;
        std::string err;
        bool ok = testService.createBookings(batchReqs, bIds, err);
        assert(!ok && "Batch 3 ca phai that bai khi ca 3 loi!");
        assert(mockPay.rollbackCalls.size() == 2 && "Phai rollback invoice cho 2 ca dau!");

        // Dam bao khong con bat ky don dat san hay hoa don nao do dang
        assert(normalizeLineEndings(guard.currentBookings()) ==
               normalizeLineEndings(bookingsBefore) &&
               "Bookings con lai ban ghi mo coi!");
        assert(normalizeLineEndings(guard.currentInvoices()) ==
               normalizeLineEndings(invoicesBefore) &&
               "Invoices con lai ban ghi mo coi!");
        std::cout << "  -> Rollback toan bo 3 ca va 2 hoa don thanh cong, khong co orphan record.\n";
    }

    // =========================================================================
    // TEST 6: Single Booking tao invoice that bai -> Rollback booking
    // =========================================================================
    std::cout << "[TEST 6] Single booking tao invoice that bai -> Rollback booking...\n";
    {
        guard.restore();
        const std::string bookingsBefore = guard.currentBookings();
        const std::string invoicesBefore = guard.currentInvoices();

        MockFailingPaymentService mockPay(paymentRepo);
        mockPay.alwaysFail = true;
        BookingService testService(bookingRepo, customerService, venueService,
                                   mockPay);

        VenueBookingRequest req;
        req.customerId = testCustId; req.customerName = testCustName; req.customerPhone = testCustPhone;
        req.fieldId = testField; req.date = "2026-11-25"; req.startTime = "10:00"; req.durationMinutes = 60;

        std::string bId, err;
        bool ok = testService.createBooking(req, bId, err);
        assert(!ok && "createBooking phai that bai khi invoice loi!");
        assert(bId.empty() && "bId phai bi xoa trang!");

        // Kiem tra sau rollback
        assert(normalizeLineEndings(guard.currentBookings()) ==
               normalizeLineEndings(bookingsBefore) &&
               "Booking van con luu du invoice loi!");
        assert(normalizeLineEndings(guard.currentInvoices()) ==
               normalizeLineEndings(invoicesBefore));
        std::cout << "  -> Don dat san don le duoc rollback sach se khi tao invoice loi.\n";
    }

    // =========================================================================
    // TEST 7: Chan tao Invoice voi amount <= 0
    // =========================================================================
    std::cout << "[TEST 7] Chan tao Invoice voi amount <= 0...\n";
    {
        std::string invId, err;
        assert(!paymentService.createInvoice("BK-ZERO", 0, invId, err));
        assert(!err.empty());
        assert(!paymentService.createInvoice("BK-NEG", -50000, invId, err));
        assert(!err.empty());
        std::cout << "  -> Da chan tuyet doi hoa don voi so tien <= 0.\n";
    }

    // =========================================================================
    // TEST 8: Chong trung lap hoa don khi Retry (Idempotency)
    // =========================================================================
    std::cout << "[TEST 8] Chong trung lap hoa don khi Retry...\n";
    {
        guard.restore();
        std::string invId, err;
        assert(paymentService.createInvoice("BK-RETRY-01", 300000, invId, err));

        LinkedList<Invoice> invList1;
        assert(paymentRepo.loadAllInvoices(invList1, err));
        int initialCount = invList1.size();

        // Ghi lai (retry) voi cung bookingId
        Invoice retryInv(invId, "BK-RETRY-01", 350000, DateTime::today(), InvoiceStatus::UNPAID);
        assert(paymentRepo.saveInvoice(retryInv, err));

        LinkedList<Invoice> invList2;
        assert(paymentRepo.loadAllInvoices(invList2, err));
        assert(invList2.size() == initialCount && "Retry khong duoc lam tang so luong hoa don!");

        Invoice fetched;
        assert(paymentService.getInvoiceByBookingId("BK-RETRY-01", fetched, err));
        assert(fetched.getTotalVnd() == 350000 && "Retry phai cap nhat ban ghi hien tai!");
        std::cout << "  -> Retry cap nhat tai cho, khong tao ban ghi duplicate.\n";
    }

    // =========================================================================
    // TEST 9: Rollback Invoice phuong thuc truc tiep
    // =========================================================================
    std::cout << "[TEST 9] Kiem tra phuong thuc rollbackInvoice...\n";
    {
        guard.restore();
        std::string invId, err;
        assert(paymentService.createInvoice("BK-ROLLBACK-TEST", 200000, invId, err));

        Invoice checkInv;
        assert(paymentService.getInvoiceByBookingId("BK-ROLLBACK-TEST", checkInv, err));

        // Tien hanh rollback bang bookingId
        assert(paymentService.rollbackInvoice("BK-ROLLBACK-TEST", err));

        // Kiem tra hoa don da bi xoa hoan toan
        Invoice deadInv;
        assert(!paymentService.getInvoiceByBookingId("BK-ROLLBACK-TEST", deadInv, err));
        std::cout << "  -> rollbackInvoice da go bo hoa don thanh cong khoi invoices.txt.\n";
    }

    // =========================================================================
    // TEST 10: Xu ly loi I/O ghi file (FailingFileManager)
    // =========================================================================
    std::cout << "[TEST 10] Xu ly loi ghi file I/O...\n";
    {
        MockFailingFileManager failingFm;
        PaymentRepository failingRepo(failingFm);
        Invoice dummyInv("INV-DUMMY", "BK-DUMMY", 100000, "2026-11-20", InvoiceStatus::UNPAID);
        std::string err;
        bool saved = failingRepo.saveInvoice(dummyInv, err);
        assert(!saved && "Luu voi file manager loi phai tra ve false!");
        assert(!err.empty() && "Phai bao cao thong diep loi chi tiet!");
        std::cout << "  -> Loi I/O ghi file duoc bao cao chinh xac: " << err << "\n";
    }

    // =========================================================================
    // TEST 11: Kiem tra tong the tinh nhat quan khong co Orphan Record
    // =========================================================================
    std::cout << "[TEST 11] Kiem tra toan ven khong co Orphan Record...\n";
    {
        guard.restore();
        LinkedList<VenueBookingRecord> allB;
        LinkedList<Invoice> allI;
        std::string err;
        assert(bookingRepo.loadAll(allB, err));
        assert(paymentRepo.loadAllInvoices(allI, err));

        // Kiem tra moi Booking hop le (CONFIRMED hoac COMPLETED) deu co hoa don tuong ung
        for (auto* b = allB.get_head(); b != nullptr; b = b->next) {
            if (b->data.status == "CONFIRMED" || b->data.status == "COMPLETED") {
                bool found = false;
                for (auto* i = allI.get_head(); i != nullptr; i = i->next) {
                    if (i->data.getBookingId() == b->data.bookingId) {
                        found = true;
                        assert(i->data.getTotalVnd() == b->data.priceVnd);
                        break;
                    }
                }
                assert(found && "Phat hien Booking CONFIRMED khong co hoa don!");
            }
        }
        std::cout << "  -> 100% don dat san CONFIRMED/COMPLETED co hoa don tuong ung nhat quan.\n";
    }

    std::cout << "\n>>> ALL 11 BOOKING AND PAYMENT CONSISTENCY TEST SUITES PASSED! <<<\n";
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
