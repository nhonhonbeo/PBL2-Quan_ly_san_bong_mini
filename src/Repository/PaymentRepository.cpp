#include "Repository/PaymentRepository.h"

#include <sstream>
#include <stdexcept>
#include <utility>

#include "Storage/FileManager.h"

namespace {

bool tachSoKhongAm(const std::string& value, long long& result) {
    if (value.empty()) return false;
    for (const char digit : value) {
        if (digit < '0' || digit > '9') return false;
    }
    try {
        size_t read = 0;
        result = std::stoll(value, &read);
        return read == value.size();
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }
}

bool tachMaTrangThai(const std::string& value, int maxValue, int& result) {
    long long parsed = 0;
    if (!tachSoKhongAm(value, parsed) || parsed > maxValue) return false;
    result = static_cast<int>(parsed);
    return true;
}

void tachTruong(const std::string& line, LinkedList<std::string>& tokens) {
    tokens.clear();
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, '|')) {
        tokens.push_back(token);
    }
    if (!line.empty() && line.back() == '|') tokens.push_back("");
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

PaymentRepository::PaymentRepository(FileManager& fileManager)
    : file_manager_(fileManager) {}

bool PaymentRepository::loadAllInvoices(LinkedList<Invoice>& out, std::string& error) const {
    // LÝ DO CHỌN: LinkedList nạp danh sách hóa đơn từ data/invoices.txt tuần tự.
    LinkedList<std::string> lines;
    if (!file_manager_.readAllLines("data/invoices.txt", lines, error)) {
        return false;
    }
    LinkedList<Invoice> loaded;
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        if (parts.size() < 4 || parts.size() > 5) {
            error = "Bản ghi hóa đơn trong data/invoices.txt sai số trường.";
            return false;
        }
        const std::string* id = layTruong(parts, 0);
        const std::string* bId = layTruong(parts, 1);
        const std::string* totalStr = layTruong(parts, 2);
        const std::string* created = layTruong(parts, 3);
        const std::string* stStr = layTruong(parts, 4);

        long long total = 0;
        int statusCode = 0;
        if (id == nullptr || bId == nullptr || totalStr == nullptr ||
            created == nullptr || !tachSoKhongAm(*totalStr, total) ||
            (stStr != nullptr && !tachMaTrangThai(*stStr, 2, statusCode))) {
            error = "Bản ghi hóa đơn trong data/invoices.txt không hợp lệ.";
            return false;
        }
        const InvoiceStatus status = static_cast<InvoiceStatus>(statusCode);
        loaded.push_back(Invoice(*id, *bId, total, *created, status));
    }

    out = std::move(loaded);
    return true;
}

bool PaymentRepository::saveAllInvoices(const LinkedList<Invoice>& invoices, std::string& error) const {
    // LÝ DO CHỌN: LinkedList nối và lưu tuần tự toàn bộ hóa đơn ra data/invoices.txt.
    LinkedList<std::string> lines;
    for (auto* node = invoices.get_head(); node != nullptr; node = node->next) {
        const Invoice& inv = node->data;
        const int status = static_cast<int>(inv.getStatus());
        if (inv.getTotalVnd() < 0 || status < 0 || status > 2) {
            error = "Không thể lưu hóa đơn có số tiền hoặc trạng thái không hợp lệ.";
            return false;
        }
        std::string line =
            FileManager::join(inv.getInvoiceId()) + "|" +
            FileManager::join(inv.getBookingId()) + "|" +
            std::to_string(inv.getTotalVnd()) + "|" +
            FileManager::join(inv.getCreatedAt()) + "|" +
            std::to_string(status);
        lines.push_back(line);
    }
    return file_manager_.writeAllLines("data/invoices.txt", lines, error);
}

bool PaymentRepository::saveInvoice(const Invoice& invoice, std::string& error) {
    LinkedList<Invoice> list;
    if (!loadAllInvoices(list, error)) {
        return false;
    }
    const bool found = list.replaceFirstIf(
        [&invoice](const Invoice& existing) {
            return existing.getInvoiceId() == invoice.getInvoiceId() ||
                   existing.getBookingId() == invoice.getBookingId();
        },
        invoice);
    if (!found) {
        list.push_back(invoice);
    }
    return saveAllInvoices(list, error);
}

bool PaymentRepository::removeInvoice(const std::string& invoiceIdOrBookingId, std::string& error) {
    LinkedList<Invoice> list;
    if (!loadAllInvoices(list, error)) return false;
    bool removed = list.removeIf([&](const Invoice& inv) {
        return inv.getInvoiceId() == invoiceIdOrBookingId ||
               inv.getBookingId() == invoiceIdOrBookingId;
    });
    if (removed) {
        return saveAllInvoices(list, error);
    }
    return true;
}

bool PaymentRepository::findInvoiceByBookingId(const std::string& bookingId,
                                              Invoice& out,
                                              std::string& error) const {
    LinkedList<Invoice> list;
    if (!loadAllInvoices(list, error)) return false;
    for (auto* node = list.get_head(); node != nullptr; node = node->next) {
        if (node->data.getBookingId() == bookingId) {
            out = node->data;
            return true;
        }
    }
    error = "Khong tim thay hoa don cho don: " + bookingId;
    return false;
}

bool PaymentRepository::loadAllPayments(LinkedList<Payment>& out, std::string& error) const {
    // LÝ DO CHỌN: LinkedList nạp danh sách giao dịch thanh toán từ data/payments.txt tuần tự.
    LinkedList<std::string> lines;
    if (!file_manager_.readAllLines("data/payments.txt", lines, error)) {
        return false;
    }
    LinkedList<Payment> loaded;
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        if (parts.size() < 4 || parts.size() > 5) {
            error = "Bản ghi thanh toán trong data/payments.txt sai số trường.";
            return false;
        }
        const std::string* id = layTruong(parts, 0);
        const std::string* invId = layTruong(parts, 1);
        const std::string* amountStr = layTruong(parts, 2);
        const std::string* methodStr = layTruong(parts, 3);
        const std::string* paidAt = layTruong(parts, 4);

        long long amount = 0;
        int methodCode = 0;
        if (id == nullptr || invId == nullptr || amountStr == nullptr ||
            methodStr == nullptr || !tachSoKhongAm(*amountStr, amount) ||
            !tachMaTrangThai(*methodStr, 2, methodCode)) {
            error = "Bản ghi thanh toán trong data/payments.txt không hợp lệ.";
            return false;
        }
        const PaymentMethod method = static_cast<PaymentMethod>(methodCode);
        loaded.push_back(Payment(*id, *invId, amount, method,
                                 paidAt ? *paidAt : ""));
    }

    out = std::move(loaded);
    return true;
}

bool PaymentRepository::saveAllPayments(const LinkedList<Payment>& payments, std::string& error) const {
    // LÝ DO CHỌN: LinkedList nối và lưu tuần tự toàn bộ giao dịch ra data/payments.txt.
    LinkedList<std::string> lines;
    for (auto* node = payments.get_head(); node != nullptr; node = node->next) {
        const Payment& p = node->data;
        const int method = static_cast<int>(p.getMethod());
        if (p.getAmountVnd() < 0 || method < 0 || method > 2) {
            error = "Không thể lưu thanh toán có số tiền hoặc phương thức không hợp lệ.";
            return false;
        }
        std::string line =
            FileManager::join(p.getPaymentId()) + "|" +
            FileManager::join(p.getInvoiceId()) + "|" +
            std::to_string(p.getAmountVnd()) + "|" +
            std::to_string(method) + "|" +
            FileManager::join(p.getPaidAt());
        lines.push_back(line);
    }
    return file_manager_.writeAllLines("data/payments.txt", lines, error);
}

bool PaymentRepository::savePayment(const Payment& payment, std::string& error) {
    LinkedList<Payment> list;
    if (!loadAllPayments(list, error)) return false;
    const bool found = list.replaceFirstIf(
        [&payment](const Payment& existing) {
            return existing.getPaymentId() == payment.getPaymentId();
        },
        payment);
    if (!found) {
        list.push_back(payment);
    }
    return saveAllPayments(list, error);
}

bool PaymentRepository::findPaymentByInvoiceId(const std::string& invoiceId,
                                              Payment& out,
                                              std::string& error) const {
    LinkedList<Payment> list;
    if (!loadAllPayments(list, error)) return false;
    for (auto* node = list.get_head(); node != nullptr; node = node->next) {
        if (node->data.getInvoiceId() == invoiceId) {
            out = node->data;
            return true;
        }
    }
    error = "Khong tim thay giao dich cho hoa don: " + invoiceId;
    return false;
}