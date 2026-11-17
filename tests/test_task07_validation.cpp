#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "Account/Customer.h"
#include "Account/Owner.h"
#include "Account/User.h"
#include "Core/DateTime.h"
#include "Customer/Blacklist.h"
#include "Customer/Discount.h"
#include "Customer/Review.h"
#include "Payment/Invoice.h"
#include "Payment/Payment.h"
#include "Repository/AccountRepository.h"
#include "Repository/BookingRepository.h"
#include "Repository/CustomerRepository.h"
#include "Repository/PaymentRepository.h"
#include "Repository/TournamentRepository.h"
#include "Repository/VenueRepository.h"
#include "Service/AccountService.h"
#include "Service/BookingService.h"
#include "Service/CustomerService.h"
#include "Service/PaymentService.h"
#include "Service/TournamentService.h"
#include "Service/VenueService.h"
#include "Storage/FileManager.h"
#include "Tournament/Tournament.h"

int main() {
    std::cout << "=== RUNNING TASK 07 FULL VALIDATION AND ID GENERATION TESTS ===" << std::endl;

    FileManager fm;
    std::string err;

    // =========================================================================
    // 1. ACCOUNT: Sinh User ID an toàn, chống trùng, kiểm tra validation
    // =========================================================================
    std::cout << "\n--- [1. ACCOUNT VALIDATION & ID GENERATION] ---" << std::endl;
    AccountRepository accRepo(fm);
    AccountService accService(accRepo);
    assert(accService.loadAccounts(err));

    std::string nextId = accService.generateNextUserId();
    std::cout << "Next User ID generated: " << nextId << std::endl;
    for (const auto& u : accService.getAccounts()) {
        assert(u->get_user_id() != nextId);
    }

    CustomerRepository custRepo(fm);
    // SĐT sai format (< 10 số, không bắt đầu bằng 0, có chữ)
    assert(!accService.registerCustomer("Khach Hang", "12345", "123456", custRepo, err));
    assert(!err.empty());
    assert(!accService.registerCustomer("Khach Hang", "090123456a", "123456", custRepo, err));
    // Mật khẩu ngắn (< 6 ký tự)
    assert(!accService.registerCustomer("Khach Hang", "0901112233", "123", custRepo, err));
    // Tên trống
    assert(!accService.registerCustomer("", "0901112233", "123456", custRepo, err));
    std::cout << "[PASS] Account registration validation verified." << std::endl;

    // =========================================================================
    // 2. BOOKING: Chuẩn hóa ngày giờ, kiểm tra sân bảo trì, trùng giờ, quyền hủy
    // =========================================================================
    std::cout << "\n--- [2. BOOKING VALIDATION & STATUS/TIME RULES] ---" << std::endl;
    VenueRepository venueRepo;
    VenueService venueService(venueRepo);
    CustomerService custService(custRepo);
    PaymentRepository payRepo(fm);
    PaymentService payService(&payRepo);
    BookingRepository bookingRepo(fm);
    BookingService bookingService(bookingRepo, custService, &venueService, &venueRepo, &payService);

    // Chuẩn hóa ngày dd/MM/yyyy và YYYY-MM-DD
    std::string isoDate = DateTime::toIsoDate("25/12/2026");
    assert(isoDate == "2026-12-25");
    assert(DateTime::compareDate("25/12/2026", "2026-12-25") == 0);

    // Đặt sân trong quá khứ bị từ chối
    VenueBookingRequest pastReq;
    pastReq.customerId = "US02";
    pastReq.customerName = "Trần Văn Thuê";
    pastReq.customerPhone = "0909876543";
    pastReq.fieldId = "S5-05";
    pastReq.date = "2020-01-01";
    pastReq.startTime = "08:00";
    pastReq.durationMinutes = 60;
    std::string pastBkId;
    assert(!bookingService.createBooking(pastReq, pastBkId, err));
    std::cout << "[PASS] Blocked booking in past date: " << err << std::endl;

    // Đặt vào sân INACTIVE / bảo trì (sân S5-01 là INACTIVE)
    VenueBookingRequest maintReq = pastReq;
    maintReq.fieldId = "S5-01";
    maintReq.date = "2026-12-25";
    std::string maintBkId;
    assert(!bookingService.createBooking(maintReq, maintBkId, err));
    std::cout << "[PASS] Blocked booking on inactive/maintenance field: " << err << std::endl;

    // Quyền hủy booking: Khách hàng khác không thể hủy đơn của người khác
    assert(!bookingService.cancelBooking("BK-000006", "US_WRONG_CUSTOMER", err));
    std::cout << "[PASS] Blocked cancellation by unauthorized customer: " << err << std::endl;

    // =========================================================================
    // 3. CUSTOMER: Không ghi nhận trùng booking trong thống kê, kiểm tra blacklist
    // =========================================================================
    std::cout << "\n--- [3. CUSTOMER STATISTICS & DEDUP] ---" << std::endl;
    Customer cBefore;
    assert(custRepo.findCustomerByUserId("US02", cBefore, err));
    int bCountBefore = cBefore.getBookingCount();

    // Dọn dẹp bản ghi kiểm thử nếu có từ trước
    const std::string dedupBookingId = "BK-TEST-DEDUP-01";
    LinkedList<std::string> recLines;
    if (fm.readAllLines("data/recorded_bookings.txt", recLines, err)) {
        recLines.removeIf([&](const std::string& l) { return l.rfind(dedupBookingId + "|", 0) == 0; });
        fm.writeAllLines("data/recorded_bookings.txt", recLines, err);
    }

    // Ghi nhận lần 1 cho BK-TEST-DEDUP
    assert(custService.recordSuccessfulBooking("US02", dedupBookingId, err));
    Customer cMid;
    assert(custRepo.findCustomerByUserId("US02", cMid, err));
    assert(cMid.getBookingCount() == bCountBefore + 1);

    // Ghi nhận lần 2 cho CÙNG booking ID -> Không được tăng lần nữa (Idempotency)
    assert(custService.recordSuccessfulBooking("US02", dedupBookingId, err));
    Customer cAfter;
    assert(custRepo.findCustomerByUserId("US02", cAfter, err));
    assert(cAfter.getBookingCount() == bCountBefore + 1);
    std::cout << "[PASS] Booking statistic dedup prevents multi-counting same booking." << std::endl;

    // Khôi phục lại dữ liệu thống kê và dọn file kiểm thử
    cBefore.setStatistics(bCountBefore, cBefore.getCancelCount());
    custRepo.updateCustomer(cBefore, err);
    if (fm.readAllLines("data/recorded_bookings.txt", recLines, err)) {
        recLines.removeIf([&](const std::string& l) { return l.rfind(dedupBookingId + "|", 0) == 0; });
        fm.writeAllLines("data/recorded_bookings.txt", recLines, err);
    }

    // =========================================================================
    // 4. DISCOUNT: Điều kiện sử dụng, không vượt số lần (USED/EXPIRED), kiểm tra thời hạn
    // =========================================================================
    std::cout << "\n--- [4. DISCOUNT LIFECYCLE & VALIDATION] ---" << std::endl;
    Discount availDisc("DC-TEST-01", "US02", 10, 10, "2026-11-01", DiscountStatus::AVAILABLE);
    assert(custService.validateDiscount(availDisc, "US02", err));

    // Không cho khách hàng khác dùng discount của US02
    assert(!custService.validateDiscount(availDisc, "US05", err));
    std::cout << "[PASS] Blocked discount usage by wrong customer: " << err << std::endl;

    // Discount đã USED không cho dùng tiếp
    Discount usedDisc("DC-TEST-02", "US02", 10, 10, "2026-11-01", DiscountStatus::USED, "BK-OLD");
    assert(!custService.validateDiscount(usedDisc, "US02", err));
    std::cout << "[PASS] Blocked already USED discount: " << err << std::endl;

    // Discount EXPIRED không cho dùng tiếp
    Discount expDisc("DC-TEST-03", "US02", 10, 10, "2026-11-01", DiscountStatus::EXPIRED);
    assert(!custService.validateDiscount(expDisc, "US02", err));
    std::cout << "[PASS] Blocked EXPIRED discount: " << err << std::endl;

    // =========================================================================
    // 5. REVIEW: Không cho đánh giá đơn chưa hoàn tất / trùng đánh giá
    // =========================================================================
    std::cout << "\n--- [5. REVIEW VALIDATION & PERMISSION] ---" << std::endl;
    // Khách hàng chưa hoàn tất ca ở sân S5-04 không được đánh giá
    Review unearnedReview("RV-TEST", "US02", "S5-04", "BK-TEMP", 5, "2026-11-14", "Danh gia ao", "Tran Van Thue");
    assert(!custService.addReview(unearnedReview, err));
    std::cout << "[PASS] Blocked review without completed booking: " << err << std::endl;

    // Rating ngoài khoảng 1-5 sao
    Review invalidStar("RV-TEST", "US02", "S5-01", "BK-1005", 10, "2026-11-14", "Qua tot", "Tran Van Thue");
    assert(!custService.addReview(invalidStar, err));
    std::cout << "[PASS] Blocked invalid star rating: " << err << std::endl;

    // =========================================================================
    // 6. TOURNAMENT: Lịch giải, kiểm tra trùng lịch sân & đơn đặt sân
    // =========================================================================
    std::cout << "\n--- [6. TOURNAMENT VALIDATION & CONFLICTS] ---" << std::endl;
    TournamentRepository tourRepo(fm);
    TournamentService tourService(tourRepo, bookingRepo);

    // Ngày kết thúc trước ngày bắt đầu
    Tournament invalidDatesTour(
        "GD-ERR", "0901234567", "Giai Loi Ngay", "S5-05",
        "28/12/2026", "20/12/2026", "Ghi chu",
        TournamentStatus::REQUESTED, "", "Chủ Giải", "08:00 - 10:00"
    );
    std::string outTId;
    assert(!tourService.guiYeuCauTaoGiai(invalidDatesTour, outTId, err));
    std::cout << "[PASS] Blocked tournament with end date earlier than start date: " << err << std::endl;

    // Trùng lịch với đơn đặt sân BK-000006 trên sân S5-05 ngày 2026-10-25 lúc 06:00-07:00
    Tournament conflictWithBookingTour(
        "GD-CONFLICT", "0901234567", "Giai Trung Lich Booking", "S5-05",
        "25/10/2026", "25/10/2026", "Ghi chu",
        TournamentStatus::REQUESTED, "", "Chủ Giải", "06:00 - 07:30"
    );
    assert(!tourService.checkOverlap(conflictWithBookingTour, "GD-CONFLICT", err));
    std::cout << "[PASS] Blocked tournament overlapping with active booking: " << err << std::endl;

    // =========================================================================
    // 7. PAYMENT: Không thanh toán lại hóa đơn đã trả, kiểm tra số tiền chính xác
    // =========================================================================
    std::cout << "\n--- [7. PAYMENT INTEGRITY & INVOICE RULES] ---" << std::endl;
    Invoice invTest("INV-TEST-001", "BK-TEST-001", 300000, DateTime::today(), InvoiceStatus::PAID);
    assert(invTest.isPaid());
    assert(!invTest.canAcceptPayment()); // Không cho thanh toán lại hóa đơn đã thanh toán

    Payment mismatchPayment("PAY-TEST", "INV-TEST-001", 200000, PaymentMethod::CASH, DateTime::today());
    assert(!payService.validatePayment(invTest, mismatchPayment, err));
    std::cout << "[PASS] Blocked paying paid invoice / wrong amount: " << err << std::endl;

    Invoice unpaidInv("INV-TEST-002", "BK-TEST-002", 300000, DateTime::today(), InvoiceStatus::UNPAID);
    assert(unpaidInv.canAcceptPayment());
    Payment correctPayment("PAY-TEST2", "INV-TEST-002", 300000, PaymentMethod::TRANSFER, DateTime::today());
    assert(payService.validatePayment(unpaidInv, correctPayment, err));
    std::cout << "[PASS] Successfully validated legitimate payment transaction." << std::endl;

    std::cout << "\n>>> ALL TASK 07 BUSINESS VALIDATIONS & CHECKS PASSED! <<<" << std::endl;
    return 0;
}

