#ifndef CUSTOMER_REPOSITORY_H
#define CUSTOMER_REPOSITORY_H

#include <string>
#include "Core/LinkedList.h"

class Blacklist;
class Customer;
class Discount;
class FileManager;
class Review;

// L? DO CH?N: CustomerRepository qu?n l? vi?c tu?n t? h?a c?c th?c th? Customer,
// Blacklist, Discount v? Review v?o c?c t?p ph?ng trong th? m?c data/.
class CustomerRepository {
private:
    FileManager& file_manager_;

public:
    explicit CustomerRepository(FileManager& fileManager);

    bool findCustomerByUserId(const std::string& userId,
                              Customer& out,
                              std::string& error) const;
    bool loadAllCustomers(LinkedList<Customer>& out, std::string& error) const;
    bool saveCustomer(const Customer& customer, std::string& error);
    bool updateCustomer(const Customer& customer, std::string& error);

    bool findActiveBlacklist(const std::string& customerId, Blacklist& out,
                             std::string& error) const;
    bool loadAllBlacklists(LinkedList<Blacklist>& out, std::string& error) const;
    bool saveBlacklist(const Blacklist& blacklist, std::string& error);
    bool removeBlacklist(const std::string& customerId, std::string& error);

    bool saveDiscount(const Discount& discount, std::string& error);

    bool saveReview(const Review& review, std::string& error);
    bool loadAllReviews(LinkedList<Review>& out, std::string& error) const;
    bool hasCompletedBooking(const std::string& customerId,
                             const std::string& fieldId) const;
};

#endif
