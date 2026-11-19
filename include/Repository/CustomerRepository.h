#ifndef CUSTOMER_REPOSITORY_H
#define CUSTOMER_REPOSITORY_H

#include <string>
#include "Account/Customer.h"
#include "Core/HashTable.h"
#include "Core/LinkedList.h"

class Blacklist;
class Discount;
class FileManager;
class Review;

// LÝ DO CHỌN: CustomerRepository quản lý tuần tự hóa Customer, Blacklist, Discount,
// Review và nhật ký thống kê booking để chống ghi nhận trùng lặp.
class CustomerRepository {
private:
    FileManager& file_manager_;
    mutable HashTable<std::string, Customer> customer_index_;
    mutable bool customer_index_loaded_;
    mutable std::string customer_file_error_;
    mutable std::string account_file_error_;

    void loadCustomerIndex() const;
    void indexCustomer(const Customer& customer);

public:
    explicit CustomerRepository(FileManager& fileManager);
    virtual ~CustomerRepository() = default;

    virtual bool findCustomerByUserId(const std::string& userId,
                                      Customer& out,
                                      std::string& error) const;
    virtual bool loadAllCustomers(LinkedList<Customer>& out, std::string& error) const;
    virtual bool saveCustomer(const Customer& customer, std::string& error);
    virtual bool updateCustomer(const Customer& customer, std::string& error);

    virtual bool findActiveBlacklist(const std::string& customerId, Blacklist& out,
                                     std::string& error) const;
    virtual bool loadAllBlacklists(LinkedList<Blacklist>& out, std::string& error) const;
    virtual bool saveBlacklist(const Blacklist& blacklist, std::string& error);
    virtual bool removeBlacklist(const std::string& customerId, std::string& error);

    virtual bool saveDiscount(const Discount& discount, std::string& error);
    virtual bool loadAllDiscounts(LinkedList<Discount>& out, std::string& error) const;
    virtual bool findDiscountsByCustomerId(const std::string& customerId,
                                           LinkedList<Discount>& out,
                                           std::string& error) const;

    virtual bool saveReview(const Review& review, std::string& error);
    virtual bool loadAllReviews(LinkedList<Review>& out, std::string& error) const;
    virtual bool hasCompletedBooking(const std::string& customerId,
                                     const std::string& fieldId) const;

    virtual bool isBookingRecorded(const std::string& bookingId) const;
    virtual bool recordBookingId(const std::string& bookingId, const std::string& customerId,
                                 const std::string& type, std::string& error);
};

#endif
