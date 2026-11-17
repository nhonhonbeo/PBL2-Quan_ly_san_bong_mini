#include <cassert>
#include <iostream>
#include <string>

#include "Core/LinkedList.h"
#include "Customer/Discount.h"
#include "Storage/FileManager.h"
#include "Repository/AccountRepository.h"
#include "Repository/BookingRepository.h"
#include "Repository/CustomerRepository.h"
#include "Repository/FieldRepository.h"
#include "Repository/PaymentRepository.h"
#include "Repository/ReportRepository.h"
#include "Repository/TournamentRepository.h"
#include "Repository/VenueRepository.h"
#include "Service/AccountService.h"
#include "Service/BookingService.h"
#include "Service/CustomerService.h"
#include "Service/FieldService.h"
#include "Service/PaymentService.h"
#include "Service/ReportService.h"
#include "Service/TournamentService.h"
#include "Service/VenueService.h"

int main() {
    std::cout << "=== RUNNING TASK 11 SERVICE LAYER RESPONSIBILITY TESTS ===" << std::endl;

    FileManager fm;
    std::string err;

    // 1. AccountService: Single Responsibility = Account & Auth
    AccountRepository accRepo(fm);
    AccountService accService(accRepo);
    assert(accService.loadAccounts(err));
    auto user = accService.authenticate("0901234567", "123456", err);
    assert(user != nullptr);
    assert(user->get_account_type() == "Owner");
    std::cout << "[PASS] AccountService: Auth & credential verification isolated." << std::endl;

    // 2. CustomerService: Single Responsibility = Customer Profile, Blacklist, Discount, Review
    CustomerRepository custRepo(fm);
    CustomerService custService(custRepo);
    bool isBlocked = false;
    std::string reason;
    assert(custService.isBlacklisted("US02", isBlocked, reason, err));
    assert(!isBlocked);

    LinkedList<Discount> discounts;
    assert(custRepo.findDiscountsByCustomerId("US02", discounts, err));
    assert(discounts.size() > 0);
    assert(custService.validateDiscount(discounts.get_head()->data, "US02", err));
    std::cout << "[PASS] CustomerService: Profile, Blacklist & Discount lifecycle isolated." << std::endl;

    // 3. VenueService: Single Responsibility = Facility, Hours, Matrix Pricing, Slots
    VenueRepository venueRepo;
    VenueService venueService(venueRepo);
    LinkedList<VenueSlotRecord> slots;
    assert(venueService.getSlots("S5-05", "2026-12-30", 60, slots, err));
    assert(slots.size() > 0);
    std::cout << "[PASS] VenueService: Facility time matrix & schedule isolated." << std::endl;

    // 4. FieldService: Single Responsibility = Physical Field Management (CRUD, Status)
    FieldRepository fieldRepo(fm, &venueRepo);
    FieldService fieldService(fieldRepo, &venueService, &venueRepo);
    LinkedList<VenueFieldRecord> ownerFields;
    assert(fieldService.getFields("US01", 0, true, ownerFields, err));
    assert(ownerFields.size() > 0);
    std::cout << "[PASS] FieldService: Field physical configuration and status isolated." << std::endl;

    // 5. PaymentService: Single Responsibility = Invoices & Payments
    PaymentRepository payRepo(fm);
    PaymentService payService(&payRepo);
    std::string testInvId;
    assert(payService.createInvoice("BK-RESP-01", 100000, testInvId, err));
    assert(testInvId == "INV-BK-RESP-01");
    Invoice inv;
    assert(payService.getInvoiceByBookingId("BK-RESP-01", inv, err));
    assert(inv.getTotalVnd() == 100000);
    assert(payService.rollbackInvoice("BK-RESP-01", err));
    std::cout << "[PASS] PaymentService: Invoices and transactional payment isolated." << std::endl;

    // 6. BookingService: Single Responsibility = Booking Lifecycle (Coordination via DI)
    BookingRepository bookingRepo(fm);
    BookingService bookingService(bookingRepo, custService, &venueService, &venueRepo, &payService);
    VenueBookingRequest bReq;
    bReq.customerId = "US02";
    bReq.customerName = "Trần Văn Thuê";
    bReq.customerPhone = "0909876543";
    bReq.fieldId = "S5-05";
    bReq.date = "2026-12-30";
    bReq.startTime = "10:00";
    bReq.durationMinutes = 60;
    std::string newBkId;
    assert(bookingService.createBooking(bReq, newBkId, err));
    assert(!newBkId.empty());
    assert(bookingService.cancelBooking(newBkId, "US02", err));
    std::cout << "[PASS] BookingService: Booking lifecycle coordination via DI verified." << std::endl;

    // 7. ReportService: Single Responsibility = Aggregations & Dashboard
    ReportRepository reportRepo;
    ReportService reportService;
    ReportDashboardMetrics metrics;
    LinkedList<ReportRevenueDay> revenueDays;
    LinkedList<ReportBookingRecord> recentBookings;
    assert(reportService.buildOwnerDashboard("US01", reportRepo, metrics, revenueDays, recentBookings, err));
    std::cout << "[PASS] ReportService: Aggregation calculations isolated." << std::endl;

    // 8. TournamentService: Single Responsibility = Tournaments, Teams & Matches
    TournamentRepository tourRepo(fm);
    TournamentService tourService(tourRepo, bookingRepo);
    LinkedList<Tournament> tours;
    assert(tourService.loadAllTournaments(tours, err));
    std::cout << "[PASS] TournamentService: Tournaments and tournament matches isolated." << std::endl;

    std::cout << "\n>>> ALL TASK 11 SERVICE RESPONSIBILITY SUITES PASSED! <<<" << std::endl;
    return 0;
}


