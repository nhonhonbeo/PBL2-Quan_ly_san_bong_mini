#include <cassert>
#include <iostream>
#include <string>

#include "Storage/FileManager.h"
#include "Repository/BookingRepository.h"
#include "Repository/CustomerRepository.h"
#include "Repository/FieldRepository.h"
#include "Repository/VenueRepository.h"
#include "Service/BookingService.h"
#include "Service/CustomerService.h"
#include "Field/Field.h"
#include "Service/FieldService.h"
#include "Service/VenueService.h"

int main() {
    std::cout << "=== RUNNING TASK 08 UNIFY VENUE & FIELD ARCHITECTURE TESTS ===" << std::endl;

    FileManager fm;
    VenueRepository venueRepo(fm);
    VenueService venueService(venueRepo);

    CustomerRepository custRepo(fm);
    CustomerService custService(custRepo);

    BookingRepository bookingRepo(venueRepo);

    // 1. Kiểm tra FieldRepository hoạt động như persistence adapter với VenueRepository được inject
    FieldRepository fieldRepo(venueRepo);
    LinkedList<VenueFieldRecord> repoFields;
    std::string err;
    assert(fieldRepo.loadAll(repoFields, err));
    std::cout << "[PASS] FieldRepository loaded fields via injected VenueRepository: " << repoFields.size() << " fields." << std::endl;

    // 2. Kiểm tra FieldService nhận VenueService & VenueRepository qua DI
    FieldService fieldService(fieldRepo, venueService);
    LinkedList<VenueFieldRecord> serviceFields;
    assert(fieldService.getFields("US01", 0, true, serviceFields, err));
    assert(serviceFields.size() > 0);
    std::cout << "[PASS] FieldService getFields through injected VenueService successful." << std::endl;

    // 3. Kiểm tra BookingService nhận VenueService & VenueRepository qua DI không tự khởi tạo lại
    BookingService bookingService(bookingRepo, custService, venueService);
    VenueBookingRequest req;
    req.customerId = "US02";
    req.customerName = "Trần Văn Thuê";
    req.customerPhone = "0909876543";
    req.fieldId = "S5-05";
    req.date = "2026-12-31";
    req.startTime = "08:00";
    req.durationMinutes = 60;
    std::string bId;
    assert(bookingService.createBooking(req, bId, err));
    assert(!bId.empty());
    std::cout << "[PASS] BookingService createBooking through injected VenueService succeeded: " << bId << std::endl;

    // Hủy đơn kiểm thử để dọn dẹp
    assert(bookingService.cancelBooking(bId, "US02", err));
    std::cout << "[PASS] Cleaned up test booking." << std::endl;

    // 4. Kiểm tra tương thích FieldRepository CRUD trên Domain Entity
    Field loadedField("", "", "", FieldSize::FIVE, 0, "", "06:00", "23:00", FieldStatus::ACTIVE);
    assert(fieldRepo.findById("S5-05", loadedField, err));
    assert(loadedField.getFieldId() == "S5-05");
    std::cout << "[PASS] FieldRepository findById domain adapter verified." << std::endl;

    std::cout << ">>> ALL TASK 08 ARCHITECTURE TESTS PASSED! <<<" << std::endl;
    return 0;
}


