#include "Repository/CustomerRepository.h"

#include <sstream>

#include "Account/Customer.h"
#include "Core/HashTable.h"
#include "Core/LinkedList.h"
#include "Customer/Blacklist.h"
#include "Customer/Discount.h"
#include "Customer/Review.h"
#include "Storage/FileManager.h"

namespace {

void tachTruong(const std::string& line, LinkedList<std::string>& tokens) {
    tokens.clear();
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, '|')) {
        tokens.push_back(token);
    }
}

const std::string* layTruong(const LinkedList<std::string>& tokens, size_t index) {
    size_t i = 0;
    for (auto* node = tokens.get_head(); node != nullptr; node = node->next) {
        if (i == index) return &node->data;
        ++i;
    }
    return nullptr;
}

}  // namespace

CustomerRepository::CustomerRepository(FileManager& fileManager)
    : file_manager_(fileManager) {}

bool CustomerRepository::findCustomerByUserId(const std::string& userId,
                                              Customer& out,
                                              std::string& error) const {
    LinkedList<std::string> lines;
    if (file_manager_.readAllLines("data/customers.txt", lines, error)) {
        LinkedList<Customer> customerList;
        for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
            LinkedList<std::string> parts;
            tachTruong(node->data, parts);
            const std::string* id = layTruong(parts, 0);
            const std::string* uName = layTruong(parts, 1);
            const std::string* pwd = layTruong(parts, 2);
            const std::string* name = layTruong(parts, 3);
            const std::string* phone = layTruong(parts, 4);
            const std::string* bCount = layTruong(parts, 5);
            const std::string* cCount = layTruong(parts, 6);
            if (id && uName && pwd && name && phone) {
                Customer c(*id, *uName, *pwd, *name, *phone);
                int bc = bCount ? std::stoi(*bCount) : 0;
                int cc = cCount ? std::stoi(*cCount) : 0;
                c.setStatistics(bc, cc);
                customerList.push_back(c);
            }
        }
        // LÝ DO CHỌN: HashTable lập chỉ mục tra cứu khách hàng theo ID hoặc SĐT O(1).
        HashTable<std::string, Customer*> customerMap;
        for (auto* node = customerList.get_head(); node != nullptr; node = node->next) {
            customerMap.insert(node->data.getUserId(), &node->data);
            customerMap.insert(node->data.get_phone_number(), &node->data);
        }
        Customer* found = nullptr;
        if (customerMap.search(userId, found) && found != nullptr) {
            out = *found;
            return true;
        }
    }

    LinkedList<std::string> accLines;
    if (file_manager_.readAllLines("data/accounts.txt", accLines, error)) {
        LinkedList<Customer> accList;
        for (auto* node = accLines.get_head(); node != nullptr; node = node->next) {
            LinkedList<std::string> parts;
            tachTruong(node->data, parts);
            const std::string* kind = layTruong(parts, 0);
            const std::string* id = layTruong(parts, 1);
            const std::string* uName = layTruong(parts, 2);
            const std::string* pwd = layTruong(parts, 3);
            const std::string* name = layTruong(parts, 4);
            const std::string* phone = layTruong(parts, 5);
            const std::string* bCount = layTruong(parts, 6);
            const std::string* cCount = layTruong(parts, 7);
            if (kind && *kind == "CUSTOMER" && id && uName && pwd && name && phone) {
                Customer c(*id, *uName, *pwd, *name, *phone);
                int bc = bCount ? std::stoi(*bCount) : 0;
                int cc = cCount ? std::stoi(*cCount) : 0;
                c.setStatistics(bc, cc);
                accList.push_back(c);
            }
        }
        HashTable<std::string, Customer*> accMap;
        for (auto* node = accList.get_head(); node != nullptr; node = node->next) {
            accMap.insert(node->data.getUserId(), &node->data);
            accMap.insert(node->data.get_phone_number(), &node->data);
        }
        Customer* found = nullptr;
        if (accMap.search(userId, found) && found != nullptr) {
            out = *found;
            return true;
        }
    }

    error = "Khong tim thay ho so khach hang: " + userId;
    return false;
}

bool CustomerRepository::loadAllCustomers(LinkedList<Customer>& out, std::string& error) const {
    out.clear();
    // LÝ DO CHỌN: LinkedList nạp danh sách khách hàng từ tập tin customers.txt tuần tự.
    LinkedList<std::string> lines;
    if (file_manager_.readAllLines("data/customers.txt", lines, error) && lines.size() > 0) {
        for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
            LinkedList<std::string> parts;
            tachTruong(node->data, parts);
            const std::string* id = layTruong(parts, 0);
            const std::string* uName = layTruong(parts, 1);
            const std::string* pwd = layTruong(parts, 2);
            const std::string* name = layTruong(parts, 3);
            const std::string* phone = layTruong(parts, 4);
            const std::string* bCount = layTruong(parts, 5);
            const std::string* cCount = layTruong(parts, 6);
            if (id && uName && pwd && name && phone) {
                Customer c(*id, *uName, *pwd, *name, *phone);
                int bc = bCount ? std::stoi(*bCount) : 0;
                int cc = cCount ? std::stoi(*cCount) : 0;
                c.setStatistics(bc, cc);
                out.push_back(c);
            }
        }
        return true;
    }

    LinkedList<std::string> accLines;
    if (file_manager_.readAllLines("data/accounts.txt", accLines, error)) {
        for (auto* node = accLines.get_head(); node != nullptr; node = node->next) {
            LinkedList<std::string> parts;
            tachTruong(node->data, parts);
            const std::string* kind = layTruong(parts, 0);
            const std::string* id = layTruong(parts, 1);
            const std::string* uName = layTruong(parts, 2);
            const std::string* pwd = layTruong(parts, 3);
            const std::string* name = layTruong(parts, 4);
            const std::string* phone = layTruong(parts, 5);
            const std::string* bCount = layTruong(parts, 6);
            const std::string* cCount = layTruong(parts, 7);
            if (kind && *kind == "CUSTOMER" && id && uName && pwd && name && phone) {
                Customer c(*id, *uName, *pwd, *name, *phone);
                int bc = bCount ? std::stoi(*bCount) : 0;
                int cc = cCount ? std::stoi(*cCount) : 0;
                c.setStatistics(bc, cc);
                out.push_back(c);
            }
        }
        return true;
    }
    return false;
}

bool CustomerRepository::saveCustomer(const Customer& customer, std::string& error) {
    LinkedList<std::string> lines;
    file_manager_.readAllLines("data/customers.txt", lines, error);
    std::string serialized =
        FileManager::join(customer.getUserId()) + "|" +
        FileManager::join(customer.get_user_name()) + "|" +
        FileManager::join(customer.get_password()) + "|" +
        FileManager::join(customer.get_name()) + "|" +
        FileManager::join(customer.get_phone_number()) + "|" +
        std::to_string(customer.getBookingCount()) + "|" +
        std::to_string(customer.getCancelCount());
    lines.push_back(serialized);
    return file_manager_.writeAllLines("data/customers.txt", lines, error);
}

bool CustomerRepository::updateCustomer(const Customer& customer, std::string& error) {
    LinkedList<std::string> lines;
    file_manager_.readAllLines("data/customers.txt", lines, error);
    std::string serialized =
        FileManager::join(customer.getUserId()) + "|" +
        FileManager::join(customer.get_user_name()) + "|" +
        FileManager::join(customer.get_password()) + "|" +
        FileManager::join(customer.get_name()) + "|" +
        FileManager::join(customer.get_phone_number()) + "|" +
        std::to_string(customer.getBookingCount()) + "|" +
        std::to_string(customer.getCancelCount());

    bool found = false;
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        const std::string* id = layTruong(parts, 0);
        if (id && *id == customer.getUserId()) {
            node->data = serialized;
            found = true;
            break;
        }
    }
    if (!found) {
        lines.push_back(serialized);
    }
    file_manager_.writeAllLines("data/customers.txt", lines, error);

    // Đồng bộ số liệu sang accounts.txt
    LinkedList<std::string> accLines;
    if (file_manager_.readAllLines("data/accounts.txt", accLines, error)) {
        for (auto* node = accLines.get_head(); node != nullptr; node = node->next) {
            LinkedList<std::string> parts;
            tachTruong(node->data, parts);
            const std::string* kind = layTruong(parts, 0);
            const std::string* id = layTruong(parts, 1);
            if (kind && *kind == "CUSTOMER" && id && *id == customer.getUserId()) {
                node->data = "CUSTOMER|" +
                             FileManager::join(customer.getUserId()) + "|" +
                             FileManager::join(customer.get_user_name()) + "|" +
                             FileManager::join(customer.get_password()) + "|" +
                             FileManager::join(customer.get_name()) + "|" +
                             FileManager::join(customer.get_phone_number()) + "|" +
                             std::to_string(customer.getBookingCount()) + "|" +
                             std::to_string(customer.getCancelCount());
                break;
            }
        }
        file_manager_.writeAllLines("data/accounts.txt", accLines, error);
    }
    return true;
}

bool CustomerRepository::findActiveBlacklist(const std::string& customerId,
                                            Blacklist& out,
                                            std::string& error) const {
    LinkedList<std::string> lines;
    if (!file_manager_.readAllLines("data/blacklist.txt", lines, error)) {
        return false;
    }
    std::string phone = customerId;
    Customer c("", "", "", "", "");
    std::string ignoredErr;
    if (findCustomerByUserId(customerId, c, ignoredErr)) {
        phone = c.get_phone_number();
    }

    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        const std::string* id = layTruong(parts, 0);
        const std::string* reason = layTruong(parts, 1);
        const std::string* date = layTruong(parts, 2);
        const std::string* status = layTruong(parts, 3);
        if (id && (*id == customerId || *id == phone)) {
            bool active = (status && *status == "1");
            out = Blacklist(*id, reason ? *reason : "", date ? *date : "", active);
            return true;
        }
    }
    error = "Khong tim thay blacklist";
    return false;
}

bool CustomerRepository::loadAllBlacklists(LinkedList<Blacklist>& out, std::string& error) const {
    out.clear();
    // LÝ DO CHỌN: LinkedList quản lý danh sách đen khách hàng nạp từ blacklist.txt.
    LinkedList<std::string> lines;
    if (!file_manager_.readAllLines("data/blacklist.txt", lines, error)) {
        return false;
    }
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        if (parts.size() < 4) continue;
        const std::string* id = layTruong(parts, 0);
        const std::string* reason = layTruong(parts, 1);
        const std::string* date = layTruong(parts, 2);
        const std::string* status = layTruong(parts, 3);
        if (id && reason && date && status) {
            out.push_back(Blacklist(*id, *reason, *date, *status == "1"));
        }
    }
    return true;
}

bool CustomerRepository::saveBlacklist(const Blacklist& blacklist, std::string& error) {
    LinkedList<std::string> lines;
    file_manager_.readAllLines("data/blacklist.txt", lines, error);
    std::string serialized =
        FileManager::join(blacklist.getCustomerId()) + "|" +
        FileManager::join(blacklist.getReason()) + "|" +
        FileManager::join(blacklist.getCreatedAt()) + "|" +
        (blacklist.isActive() ? "1" : "0");

    bool found = false;
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        const std::string* id = layTruong(parts, 0);
        if (id && *id == blacklist.getCustomerId()) {
            node->data = serialized;
            found = true;
            break;
        }
    }
    if (!found) {
        lines.push_back(serialized);
    }
    return file_manager_.writeAllLines("data/blacklist.txt", lines, error);
}

bool CustomerRepository::removeBlacklist(const std::string& customerId, std::string& error) {
    Blacklist bl(customerId, "", "", false);
    return saveBlacklist(bl, error);
}

bool CustomerRepository::saveDiscount(const Discount& discount, std::string& error) {
    LinkedList<std::string> lines;
    file_manager_.readAllLines("data/discounts.txt", lines, error);
    std::string serialized =
        FileManager::join(discount.getDiscountId()) + "|" +
        FileManager::join(discount.getCustomerId()) + "|" +
        std::to_string(discount.getPercentage()) + "|" +
        std::to_string(discount.getQualifiedBookingCount()) + "|" +
        FileManager::join(discount.getGrantedAt()) + "|" +
        std::to_string(static_cast<int>(discount.getStatus())) + "|" +
        FileManager::join(discount.getUsedBookingId());

    bool found = false;
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        const std::string* id = layTruong(parts, 0);
        if (id && *id == discount.getDiscountId()) {
            node->data = serialized;
            found = true;
            break;
        }
    }
    if (!found) {
        lines.push_back(serialized);
    }
    return file_manager_.writeAllLines("data/discounts.txt", lines, error);
}

bool CustomerRepository::saveReview(const Review& review, std::string& error) {
    LinkedList<std::string> lines;
    file_manager_.readAllLines("data/reviews.txt", lines, error);

    std::string custName = review.getCustomerName();
    if (custName.empty()) {
        custName = review.getCustomerId();
        Customer c("", "", "", "", "");
        std::string ignoredErr;
        if (findCustomerByUserId(review.getCustomerId(), c, ignoredErr)) {
            custName = c.get_name();
        }
    }

    std::string serialized =
        FileManager::join(review.getReviewId()) + "|" +
        FileManager::join(custName) + "|" +
        FileManager::join(review.getCustomerId()) + "|" +
        FileManager::join(review.getFieldId()) + "|" +
        std::to_string(review.getRating()) + "|" +
        FileManager::join(review.getCreatedAt()) + "|" +
        FileManager::join(review.getComment());

    bool found = false;
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        const std::string* id = layTruong(parts, 0);
        if (id && *id == review.getReviewId()) {
            node->data = serialized;
            found = true;
            break;
        }
    }
    if (!found) {
        lines.push_back(serialized);
    }
    return file_manager_.writeAllLines("data/reviews.txt", lines, error);
}

bool CustomerRepository::loadAllReviews(LinkedList<Review>& out, std::string& error) const {
    out.clear();
    // LÝ DO CHỌN: LinkedList nạp danh sách nhận xét đánh giá từ reviews.txt tuần tự.
    LinkedList<std::string> lines;
    if (!file_manager_.readAllLines("data/reviews.txt", lines, error)) {
        return false;
    }
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        if (parts.size() < 7) continue;
        const std::string* id = layTruong(parts, 0);
        const std::string* custName = layTruong(parts, 1);
        const std::string* custPhone = layTruong(parts, 2);
        const std::string* fieldId = layTruong(parts, 3);
        const std::string* starStr = layTruong(parts, 4);
        const std::string* date = layTruong(parts, 5);
        const std::string* content = layTruong(parts, 6);

        int stars = 5;
        if (starStr) {
            try { stars = std::stoi(*starStr); } catch (...) {}
        }
        Review r(id ? *id : "",
                 custPhone ? *custPhone : "",
                 fieldId ? *fieldId : "",
                 "",
                 stars,
                 date ? *date : "",
                 content ? *content : "",
                 custName ? *custName : "");
        out.push_back(r);
    }
    return true;
}

bool CustomerRepository::hasCompletedBooking(const std::string& customerId,
                                             const std::string& fieldId) const {
    LinkedList<std::string> lines;
    std::string err;
    if (!file_manager_.readAllLines("data/bookings.txt", lines, err)) {
        return false;
    }
    std::string phone = customerId;
    Customer c("", "", "", "", "");
    std::string ignoredErr;
    if (findCustomerByUserId(customerId, c, ignoredErr)) {
        phone = c.get_phone_number();
    }

    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        const std::string* cId = layTruong(parts, 1);
        const std::string* cPhone = layTruong(parts, 3);
        const std::string* fId = layTruong(parts, 4);
        const std::string* status = layTruong(parts, 9);
        if (cId && cPhone && fId && status) {
            if (*fId == fieldId &&
                (*cId == customerId || *cPhone == customerId || *cId == phone || *cPhone == phone) &&
                (*status == "COMPLETED" || *status == u8"Đã hoàn thành")) {
                return true;
            }
        }
    }
    return false;
}
