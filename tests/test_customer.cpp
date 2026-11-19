#include <cassert>
#include <iostream>

#include "Account/Customer.h"
#include "Customer/Review.h"
#include "Repository/CustomerRepository.h"
#include "Service/CustomerService.h"
#include "Storage/FileManager.h"

int main() {
    FileManager fileManager;
    CustomerRepository customerRepository(fileManager);
    CustomerService customerService(customerRepository);

    Customer customer;
    std::string error;
    assert(customerRepository.findCustomerByUserId("US02", customer, error));
    assert(customer.getUserId() == "US02");
    assert(customer.get_phone_number() == "0909876543");

    const int originalBookings = customer.getBookingCount();
    const int originalCancellations = customer.getCancelCount();

    assert(customerService.recordCancellation("US02", error));
    assert(customerRepository.findCustomerByUserId("US02", customer, error));
    assert(customer.getCancelCount() == originalCancellations + 1);

    assert(customerService.recordSuccessfulBooking("US02", error));
    assert(customerRepository.findCustomerByUserId("US02", customer, error));
    assert(customer.getBookingCount() == originalBookings + 1);

    customer.setStatistics(originalBookings, originalCancellations);
    assert(customerRepository.updateCustomer(customer, error));

    Review invalidRating("RV99", "US02", "S5-01", "BK-0001", 6,
                         "2026-11-06", "Good field");
    assert(!customerService.addReview(invalidRating, error));
    assert(!error.empty());

    Review emptyComment("RV99", "US02", "S5-01", "BK-0001", 5,
                        "2026-11-06", "");
    assert(!customerService.addReview(emptyComment, error));
    assert(!error.empty());

    std::cout << "CUSTOMER PROFILE, STATISTICS AND REVIEW TESTS PASSED!\n";
    return 0;
}
