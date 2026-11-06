#include <cassert>
#include <iostream>
#include <string>

#include "Account/Customer.h"
#include "Customer/Blacklist.h"
#include "Customer/Discount.h"
#include "Customer/Review.h"
#include "Repository/CustomerRepository.h"
#include "Service/CustomerService.h"
#include "Storage/FileManager.h"

int main() {
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
    bool isBlocked = false;
    std::string reason;
    assert(customerService.isBlacklisted("US02", isBlocked, reason));
    assert(!isBlocked);

    // Phone 0944112233 exists in data/blacklist.txt
    assert(customerService.isBlacklisted("0944112233", isBlocked, reason));
    assert(isBlocked);
    assert(!reason.empty());

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

    // 4. Test CustomerService: addReview validation
    Review badRating("RV99", "US02", "S5-01", "BK-0001", 6, "2026-11-06", "Good field");
    assert(!customerService.addReview(badRating, err));
    assert(!err.empty());

    Review emptyComment("RV99", "US02", "S5-01", "BK-0001", 5, "2026-11-06", "");
    assert(!customerService.addReview(emptyComment, err));
    assert(!err.empty());

    std::cout << "ALL CUSTOMER REPOSITORY & SERVICE TESTS PASSED!" << std::endl;
    return 0;
}