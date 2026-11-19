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

const std::string* layTruong(const LinkedList<std::string>& tokens, size_t index);

void tachTruong(const std::string& line, LinkedList<std::string>& tokens) {
    tokens.clear();
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, '|')) {
        tokens.push_back(token);
    }
}

bool parseCustomer(const LinkedList<std::string>& parts, Customer& customer) {
    const std::string* id = layTruong(parts, 0);
    const std::string* userName = layTruong(parts, 1);
    const std::string* password = layTruong(parts, 2);
    const std::string* name = layTruong(parts, 3);
    const std::string* phone = layTruong(parts, 4);
    if (!id || !userName || !password || !name || !phone) return false;

    customer = Customer(*id, *userName, *password, *name, *phone);
    const std::string* bookingCount = layTruong(parts, 5);
    const std::string* cancelCount = layTruong(parts, 6);
    customer.setStatistics(bookingCount ? std::stoi(*bookingCount) : 0,
                           cancelCount ? std::stoi(*cancelCount) : 0);
    return true;
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
    : file_manager_(fileManager),
      customer_index_loaded_(false) {}

void CustomerRepository::indexCustomer(const Customer& customer) {
    customer_index_.insert(customer.getUserId(), customer);
    customer_index_.insert(customer.get_phone_number(), customer);
}

void CustomerRepository::loadCustomerIndex() const {
    if (customer_index_loaded_) return;

    LinkedList<std::string> customerLines;
    LinkedList<std::string> accountLines;
    std::string customerError;
    std::string accountError;
    const bool customersLoaded = file_manager_.readAllLines(
        "data/customers.txt", customerLines, customerError);
    const bool accountsLoaded = file_manager_.readAllLines(
        "data/accounts.txt", accountLines, accountError);

    HashTable<std::string, Customer> newIndex;
    if (customersLoaded) {
        for (auto* node = customerLines.get_head(); node != nullptr; node = node->next) {
            LinkedList<std::string> parts;
            tachTruong(node->data, parts);
            Customer customer;
            if (parseCustomer(parts, customer)) {
                newIndex.insert(customer.getUserId(), customer);
                newIndex.insert(customer.get_phone_number(), customer);
            }
        }
    }

    if (accountsLoaded) {
        for (auto* node = accountLines.get_head(); node != nullptr; node = node->next) {
            LinkedList<std::string> parts;
            tachTruong(node->data, parts);
            const std::string* kind = layTruong(parts, 0);
            if (!kind || *kind != "CUSTOMER") continue;

            LinkedList<std::string> customerParts;
            for (std::size_t i = 1; i < static_cast<std::size_t>(parts.size()); ++i) {
                const std::string* part = layTruong(parts, i);
                if (part) customerParts.push_back(*part);
            }
            Customer customer;
            if (!parseCustomer(customerParts, customer)) continue;
            if (!newIndex.contains(customer.getUserId())) {
                newIndex.insert(customer.getUserId(), customer);
            }
            if (!newIndex.contains(customer.get_phone_number())) {
                newIndex.insert(customer.get_phone_number(), customer);
            }
        }
    }

    customer_index_ = std::move(newIndex);
    customer_file_error_ = std::move(customerError);
    account_file_error_ = std::move(accountError);
    customer_index_loaded_ = customersLoaded && accountsLoaded;
}

bool CustomerRepository::findCustomerByUserId(const std::string& userId,
                                              Customer& out,
                                              std::string& error) const {
    error.clear();
    loadCustomerIndex();
    const Customer* found = customer_index_.find(userId);
    if (found != nullptr) {
        out = *found;
        return true;
    }

    if (!account_file_error_.empty()) {
        error = account_file_error_;
        return false;
    }
    if (!customer_file_error_.empty()) {
        error = customer_file_error_;
        return false;
    }
    error = "Khong tim thay ho so khach hang: " + userId;
    return false;
}

bool CustomerRepository::loadAllCustomers(LinkedList<Customer>& out, std::string& error) const {
    out.clear();
    customer_index_loaded_ = false;
    // LÃ DO CHá»ŒN: LinkedList náº¡p danh sÃ¡ch khÃ¡ch hÃ ng tá»« táº­p tin customers.txt tuáº§n tá»±.
    LinkedList<std::string> lines;
    if (!file_manager_.readAllLines("data/customers.txt", lines, error)) {
        customer_index_loaded_ = false;
        return false;
    }
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        if (parts.size() < 7) continue;
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

    // Náº¡p thÃªm khÃ¡ch hÃ ng tá»« accounts.txt náº¿u chÆ°a cÃ³ trong customers.txt
    LinkedList<std::string> accLines;
    const bool accountsLoaded =
        file_manager_.readAllLines("data/accounts.txt", accLines, error);
    if (accountsLoaded) {
        for (auto* node = accLines.get_head(); node != nullptr; node = node->next) {
            LinkedList<std::string> parts;
            tachTruong(node->data, parts);
            if (parts.size() < 8) continue;
            const std::string* kind = layTruong(parts, 0);
            if (!kind || *kind != "CUSTOMER") continue;
            const std::string* id = layTruong(parts, 1);
            const std::string* uName = layTruong(parts, 2);
            const std::string* pwd = layTruong(parts, 3);
            const std::string* name = layTruong(parts, 4);
            const std::string* phone = layTruong(parts, 5);
            const std::string* bCount = layTruong(parts, 6);
            const std::string* cCount = layTruong(parts, 7);

            bool existed = false;
            for (auto* cNode = out.get_head(); cNode != nullptr; cNode = cNode->next) {
                if (id && cNode->data.getUserId() == *id) {
                    existed = true;
                    break;
                }
            }
            if (!existed && id && uName && pwd && name && phone) {
                Customer c(*id, *uName, *pwd, *name, *phone);
                int bc = bCount ? std::stoi(*bCount) : 0;
                int cc = cCount ? std::stoi(*cCount) : 0;
                c.setStatistics(bc, cc);
                out.push_back(c);
            }
        }
    }
    if (accountsLoaded) {
        HashTable<std::string, Customer> newIndex;
        for (auto* node = out.get_head(); node != nullptr; node = node->next) {
            newIndex.insert(node->data.getUserId(), node->data);
            newIndex.insert(node->data.get_phone_number(), node->data);
        }
        customer_index_ = std::move(newIndex);
        customer_file_error_.clear();
        account_file_error_.clear();
        customer_index_loaded_ = true;
    } else {
        customer_index_loaded_ = false;
        account_file_error_ = error;
    }
    return true;
}

bool CustomerRepository::saveCustomer(const Customer& customer, std::string& error) {
    LinkedList<std::string> lines;
    if (!file_manager_.readAllLines("data/customers.txt", lines, error)) {
        return false;
    }
    std::string serialized =
        FileManager::join(customer.getUserId()) + "|" +
        FileManager::join(customer.get_user_name()) + "|" +
        FileManager::join(customer.get_password()) + "|" +
        FileManager::join(customer.get_name()) + "|" +
        FileManager::join(customer.get_phone_number()) + "|" +
        std::to_string(customer.getBookingCount()) + "|" +
        std::to_string(customer.getCancelCount());
    lines.push_back(serialized);
    if (!file_manager_.writeAllLines("data/customers.txt", lines, error)) {
        return false;
    }
    if (customer_index_loaded_) {
        customer_index_loaded_ = false;
        indexCustomer(customer);
        customer_index_loaded_ = true;
    }
    return true;
}

bool CustomerRepository::updateCustomer(const Customer& customer, std::string& error) {
    LinkedList<std::string> lines;
    if (!file_manager_.readAllLines("data/customers.txt", lines, error)) {
        return false;
    }
    std::string serialized =
        FileManager::join(customer.getUserId()) + "|" +
        FileManager::join(customer.get_user_name()) + "|" +
        FileManager::join(customer.get_password()) + "|" +
        FileManager::join(customer.get_name()) + "|" +
        FileManager::join(customer.get_phone_number()) + "|" +
        std::to_string(customer.getBookingCount()) + "|" +
        std::to_string(customer.getCancelCount());

    const bool found = lines.replaceFirstIf(
        [&customer](const std::string& line) {
            LinkedList<std::string> parts;
            tachTruong(line, parts);
            const std::string* id = layTruong(parts, 0);
            return id && *id == customer.getUserId();
        },
        serialized);
    if (!found) {
        lines.push_back(serialized);
    }
    if (!file_manager_.writeAllLines("data/customers.txt", lines, error)) {
        return false;
    }

    // Äá»“ng bá»™ sá»‘ liá»‡u sang accounts.txt
    LinkedList<std::string> accLines;
    if (file_manager_.readAllLines("data/accounts.txt", accLines, error)) {
        std::string accountSerialized =
            "CUSTOMER|" +
            FileManager::join(customer.getUserId()) + "|" +
            FileManager::join(customer.get_user_name()) + "|" +
            FileManager::join(customer.get_password()) + "|" +
            FileManager::join(customer.get_name()) + "|" +
            FileManager::join(customer.get_phone_number()) + "|" +
            std::to_string(customer.getBookingCount()) + "|" +
            std::to_string(customer.getCancelCount());
        accLines.replaceFirstIf(
            [&customer](const std::string& line) {
                LinkedList<std::string> parts;
                tachTruong(line, parts);
                const std::string* kind = layTruong(parts, 0);
                const std::string* id = layTruong(parts, 1);
                return kind && *kind == "CUSTOMER" && id &&
                       *id == customer.getUserId();
            },
            std::move(accountSerialized));
        if (!file_manager_.writeAllLines("data/accounts.txt", accLines, error)) {
            customer_index_loaded_ = false;
            return false;
        }
    } else {
        customer_index_loaded_ = false;
        return true;
    }
    if (customer_index_loaded_) {
        customer_index_loaded_ = false;
        customer_index_.removeIf(
            [&customer](const Entry<std::string, Customer>& entry) {
                return entry.value.getUserId() == customer.getUserId();
            });
        indexCustomer(customer);
        customer_index_loaded_ = true;
    }
    return true;
}

bool CustomerRepository::findActiveBlacklist(const std::string& customerId,
                                            Blacklist& out,
                                            std::string& error) const {
    error.clear();
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
    error.clear();
    return false;
}

bool CustomerRepository::loadAllBlacklists(LinkedList<Blacklist>& out, std::string& error) const {
    out.clear();
    // LÃ DO CHá»ŒN: LinkedList quáº£n lÃ½ danh sÃ¡ch Ä‘en khÃ¡ch hÃ ng náº¡p tá»« blacklist.txt.
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
    if (!file_manager_.readAllLines("data/blacklist.txt", lines, error)) {
        return false;
    }
    std::string serialized =
        FileManager::join(blacklist.getCustomerId()) + "|" +
        FileManager::join(blacklist.getReason()) + "|" +
        FileManager::join(blacklist.getCreatedAt()) + "|" +
        (blacklist.isActive() ? "1" : "0");

    const bool found = lines.replaceFirstIf(
        [&blacklist](const std::string& line) {
            LinkedList<std::string> parts;
            tachTruong(line, parts);
            const std::string* id = layTruong(parts, 0);
            return id && *id == blacklist.getCustomerId();
        },
        serialized);
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
    if (!file_manager_.readAllLines("data/discounts.txt", lines, error)) {
        return false;
    }
    std::string serialized =
        FileManager::join(discount.getDiscountId()) + "|" +
        FileManager::join(discount.getCustomerId()) + "|" +
        std::to_string(discount.getPercentage()) + "|" +
        std::to_string(discount.getQualifiedBookingCount()) + "|" +
        FileManager::join(discount.getGrantedAt()) + "|" +
        std::to_string(static_cast<int>(discount.getStatus())) + "|" +
        FileManager::join(discount.getUsedBookingId());

    const bool found = lines.replaceFirstIf(
        [&discount](const std::string& line) {
            LinkedList<std::string> parts;
            tachTruong(line, parts);
            const std::string* id = layTruong(parts, 0);
            return id && *id == discount.getDiscountId();
        },
        serialized);
    if (!found) {
        lines.push_back(serialized);
    }
    return file_manager_.writeAllLines("data/discounts.txt", lines, error);
}

bool CustomerRepository::loadAllDiscounts(LinkedList<Discount>& out, std::string& error) const {
    out.clear();
    // LÃ DO CHá»ŒN: LinkedList náº¡p danh sÃ¡ch Æ°u Ä‘Ã£i tá»« discounts.txt tuáº§n tá»±.
    LinkedList<std::string> lines;
    if (!file_manager_.readAllLines("data/discounts.txt", lines, error)) {
        return false;
    }
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        if (parts.size() < 6) continue;
        const std::string* id = layTruong(parts, 0);
        const std::string* custId = layTruong(parts, 1);
        const std::string* pctStr = layTruong(parts, 2);
        const std::string* countStr = layTruong(parts, 3);
        const std::string* grantedAt = layTruong(parts, 4);
        const std::string* stStr = layTruong(parts, 5);
        const std::string* usedId = layTruong(parts, 6);

        int pct = pctStr ? std::stoi(*pctStr) : 0;
        int count = countStr ? std::stoi(*countStr) : 0;
        DiscountStatus st = DiscountStatus::AVAILABLE;
        if (stStr) {
            try { st = static_cast<DiscountStatus>(std::stoi(*stStr)); } catch (...) {}
        }
        Discount d(id ? *id : "", custId ? *custId : "", pct, count,
                   grantedAt ? *grantedAt : "", st, usedId ? *usedId : "");
        out.push_back(d);
    }
    return true;
}

bool CustomerRepository::findDiscountsByCustomerId(const std::string& customerId,
                                                   LinkedList<Discount>& out,
                                                   std::string& error) const {
    out.clear();
    LinkedList<Discount> all;
    if (!loadAllDiscounts(all, error)) return false;
    for (auto* node = all.get_head(); node != nullptr; node = node->next) {
        if (node->data.getCustomerId() == customerId) {
            out.push_back(node->data);
        }
    }
    return true;
}

bool CustomerRepository::saveReview(const Review& review, std::string& error) {
    LinkedList<std::string> lines;
    if (!file_manager_.readAllLines("data/reviews.txt", lines, error)) {
        return false;
    }

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

    const bool found = lines.replaceFirstIf(
        [&review](const std::string& line) {
            LinkedList<std::string> parts;
            tachTruong(line, parts);
            const std::string* id = layTruong(parts, 0);
            return id && *id == review.getReviewId();
        },
        serialized);
    if (!found) {
        lines.push_back(serialized);
    }
    return file_manager_.writeAllLines("data/reviews.txt", lines, error);
}

bool CustomerRepository::loadAllReviews(LinkedList<Review>& out, std::string& error) const {
    out.clear();
    // LÃ DO CHá»ŒN: LinkedList náº¡p danh sÃ¡ch nháº­n xÃ©t Ä‘Ã¡nh giÃ¡ tá»« reviews.txt tuáº§n tá»±.
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
        if (parts.size() < 6) continue;
        const std::string* cId = layTruong(parts, 1);
        const std::string* fId = layTruong(parts, 3);
        const std::string* st = layTruong(parts, 4);
        if (cId && fId && st) {
            if ((*cId == customerId || *cId == phone) && *fId == fieldId && *st == "2") {
                return true;
            }
        }
    }
    return false;
}

bool CustomerRepository::isBookingRecorded(const std::string& bookingId) const {
    if (bookingId.empty()) return false;
    LinkedList<std::string> lines;
    std::string err;
    if (!file_manager_.readAllLines("data/recorded_bookings.txt", lines, err)) {
        return false;
    }
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        const std::string* id = layTruong(parts, 0);
        if (id && *id == bookingId) return true;
    }
    return false;
}

bool CustomerRepository::recordBookingId(const std::string& bookingId,
                                         const std::string& customerId,
                                         const std::string& type,
                                         std::string& error) {
    if (bookingId.empty()) return true;
    LinkedList<std::string> lines;
    if (!file_manager_.readAllLines("data/recorded_bookings.txt", lines, error)) {
        return false;
    }
    std::string line = FileManager::join(bookingId) + "|" +
                       FileManager::join(customerId) + "|" +
                       FileManager::join(type);
    lines.push_back(line);
    return file_manager_.writeAllLines("data/recorded_bookings.txt", lines, error);
}
