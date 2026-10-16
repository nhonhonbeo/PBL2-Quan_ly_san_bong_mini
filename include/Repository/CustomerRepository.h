#ifndef CUSTOMER_REPOSITORY_H
#define CUSTOMER_REPOSITORY_H

#include <string>

class Blacklist;
class CustomerProfile;
class Discount;
class FileManager;
class Review;

// LÝ DO CHỌN: Repository chỉ ánh xạ Customer với file; các rule blacklist,
// ưu đãi và review được giữ ở CustomerService.
class CustomerRepository {
private:
    FileManager& file_manager_;

public:
    explicit CustomerRepository(FileManager& fileManager);

    bool findCustomerByUserId(const std::string& userId,
                              CustomerProfile& out,
                              std::string& error) const;
    bool saveCustomer(const CustomerProfile& customer, std::string& error);
    bool updateCustomer(const CustomerProfile& customer, std::string& error);

    bool findActiveBlacklist(const std::string& customerId, Blacklist& out,
                             std::string& error) const;
    bool saveBlacklist(const Blacklist& blacklist, std::string& error);
    bool saveDiscount(const Discount& discount, std::string& error);
    bool saveReview(const Review& review, std::string& error);
};

#endif
