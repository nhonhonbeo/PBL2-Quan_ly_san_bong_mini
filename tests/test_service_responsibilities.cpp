#include <cassert>
#include <iostream>
#include <string>

#include "Core/LinkedList.h"
#include "Storage/FileManager.h"
#include "Repository/AccountRepository.h"
#include "Repository/BookingRepository.h"
#include "Repository/CustomerRepository.h"
#include "Repository/FieldRepository.h"
#include "Repository/ReportRepository.h"
#include "Repository/TournamentRepository.h"
#include "Repository/VenueRepository.h"
#include "Service/AccountService.h"
#include "Service/BookingService.h"
#include "Service/CustomerService.h"
#include "Service/FieldService.h"
#include "Service/ReportService.h"
#include "Service/TournamentService.h"
#include "Service/VenueService.h"

int main() {
    std::cout << "=== RUNNING TASK 11 SERVICE LAYER RESPONSIBILITY TESTS ===" << std::endl;

    FileManager fm;
    std::string err;

    // 1. AccountService: Single Responsibility = Account & Auth
    AccountRepository accRepo(fm);
    CustomerRepository custRepo(fm);
    AccountService accService(accRepo, custRepo);
    assert(accService.loadAccounts(err));
    auto user = accService.authenticate("0901234567", "123456", err);
    assert(user != nullptr);
    assert(user->get_account_type() == "Owner");
    std::cout << "[PASS] AccountService: Auth & credential verification isolated." << std::endl;

    // 2. CustomerService: Hồ sơ khách hàng, thống kê booking và đánh giá
    CustomerService custService(custRepo);
    Customer customer;
    assert(custService.findCustomer("US02", customer, err));
    assert(customer.getUserId() == "US02");
    std::cout << "[PASS] CustomerService: Customer profile lookup isolated." << std::endl;

    // 3. VenueService: Single Responsibility = Facility, Hours, Matrix Pricing, Slots
    VenueRepository venueRepo(fm);
    VenueService venueService(venueRepo);
    LinkedList<VenueSlotRecord> slots;
    assert(venueService.getSlots("S5-05", "2026-12-30", 60, slots, err));
    assert(slots.size() > 0);
    std::cout << "[PASS] VenueService: Facility time matrix & schedule isolated." << std::endl;

    // 4. FieldService: Single Responsibility = Physical Field Management (CRUD, Status)
    FieldRepository fieldRepo(venueRepo);
    FieldService fieldService(fieldRepo, venueService);
    LinkedList<VenueFieldRecord> ownerFields;
    assert(fieldService.getFields("US01", 0, true, ownerFields, err));
    assert(ownerFields.size() > 0);
    std::cout << "[PASS] FieldService: Field physical configuration and status isolated." << std::endl;

    // 5. BookingService: Single Responsibility = Booking Lifecycle (Coordination via DI)
    BookingRepository bookingRepo(venueRepo);
    BookingService bookingService(bookingRepo, custService, venueService);
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
    ReportRepository reportRepo(venueRepo);
    ReportService reportService(reportRepo);
    ReportDashboardMetrics metrics;
    LinkedList<ReportRevenueDay> revenueDays;
    LinkedList<ReportBookingRecord> recentBookings;
    assert(reportService.buildOwnerDashboard("US01", metrics, revenueDays, recentBookings, err));
    std::cout << "[PASS] ReportService: Aggregation calculations isolated." << std::endl;

    // 8. TournamentService: Single Responsibility = Tournament requests and decisions
    TournamentRepository tourRepo(fm);
    TournamentService tourService(tourRepo, bookingRepo);
    LinkedList<Tournament> tours;
    assert(tourService.loadAllTournaments(tours, err));
    std::cout << "[PASS] TournamentService: Tournaments and tournament matches isolated." << std::endl;

    std::cout << "\n>>> ALL TASK 11 SERVICE RESPONSIBILITY SUITES PASSED! <<<" << std::endl;
    return 0;
}


