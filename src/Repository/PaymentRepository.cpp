#include "Repository/PaymentRepository.h"

#include <sstream>

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

PaymentRepository::PaymentRepository(FileManager& fileManager)
    : file_manager_(fileManager) {}

bool PaymentRepository::loadAllInvoices(LinkedList<Invoice>& out, std::string& error) const {
    out.clear();
    // LÝ DO CHỌN: LinkedList nạp danh sách hóa đơn từ data/invoices.txt tuần tự.
    LinkedList<std::string> lines;
    if (!file_manager_.readAllLines("data/invoices.txt", lines, error)) {
        return false;
    }
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        if (parts.size() < 4) continue;
        const std::string* id = layTruong(parts, 0);
        const std::string* bId = layTruong(parts, 1);
        const std::string* totalStr = layTruong(parts, 2);
        const std::string* created = layTruong(parts, 3);
        const std::string* stStr = layTruong(parts, 4);

        long long total = 0;
        if (totalStr) {
            try { total = std::stoll(*totalStr); } catch (...) {}
        }
        InvoiceStatus status = InvoiceStatus::UNPAID;
        if (stStr) {
            try {
                int code = std::stoi(*stStr);
                if (code >= 0 && code <= 2) {
                    status = static_cast<InvoiceStatus>(code);
                }
            } catch (...) {}
        }

        Invoice inv(id ? *id : "", bId ? *bId : "", total,
                    created ? *created : "", status);
        out.push_back(inv);
    }
    return true;
}

bool PaymentRepository::saveAllInvoices(const LinkedList<Invoice>& invoices, std::string& error) const {
    // LÝ DO CHỌN: LinkedList nối và lưu tuần tự toàn bộ hóa đơn ra data/invoices.txt.
    LinkedList<std::string> lines;
    for (auto* node = invoices.get_head(); node != nullptr; node = node->next) {
        const Invoice& inv = node->data;
        std::string line =
            FileManager::join(inv.getInvoiceId()) + "|" +
            FileManager::join(inv.getBookingId()) + "|" +
            std::to_string(inv.getTotalVnd()) + "|" +
            FileManager::join(inv.getCreatedAt()) + "|" +
            std::to_string(static_cast<int>(inv.getStatus()));
        lines.push_back(line);
    }
    return file_manager_.writeAllLines("data/invoices.txt", lines, error);
}

bool PaymentRepository::saveInvoice(const Invoice& invoice, std::string& error) {
    LinkedList<Invoice> list;
    loadAllInvoices(list, error);
    bool found = false;
    for (auto* node = list.get_head(); node != nullptr; node = node->next) {
        if (node->data.getInvoiceId() == invoice.getInvoiceId()) {
            node->data = invoice;
            found = true;
            break;
        }
    }
    if (!found) {
        list.push_back(invoice);
    }
    return saveAllInvoices(list, error);
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
    out.clear();
    // LÝ DO CHỌN: LinkedList nạp danh sách giao dịch thanh toán từ data/payments.txt tuần tự.
    LinkedList<std::string> lines;
    if (!file_manager_.readAllLines("data/payments.txt", lines, error)) {
        return false;
    }
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        if (parts.size() < 4) continue;
        const std::string* id = layTruong(parts, 0);
        const std::string* invId = layTruong(parts, 1);
        const std::string* amountStr = layTruong(parts, 2);
        const std::string* methodStr = layTruong(parts, 3);
        const std::string* paidAt = layTruong(parts, 4);

        long long amount = 0;
        if (amountStr) {
            try { amount = std::stoll(*amountStr); } catch (...) {}
        }
        PaymentMethod method = PaymentMethod::CASH;
        if (methodStr) {
            try {
                int code = std::stoi(*methodStr);
                if (code >= 0 && code <= 2) {
                    method = static_cast<PaymentMethod>(code);
                }
            } catch (...) {}
        }

        Payment p(id ? *id : "", invId ? *invId : "", amount,
                  method, paidAt ? *paidAt : "");
        out.push_back(p);
    }
    return true;
}

bool PaymentRepository::saveAllPayments(const LinkedList<Payment>& payments, std::string& error) const {
    // LÝ DO CHỌN: LinkedList nối và lưu tuần tự toàn bộ giao dịch ra data/payments.txt.
    LinkedList<std::string> lines;
    for (auto* node = payments.get_head(); node != nullptr; node = node->next) {
        const Payment& p = node->data;
        std::string line =
            FileManager::join(p.getPaymentId()) + "|" +
            FileManager::join(p.getInvoiceId()) + "|" +
            std::to_string(p.getAmountVnd()) + "|" +
            std::to_string(static_cast<int>(p.getMethod())) + "|" +
            FileManager::join(p.getPaidAt());
        lines.push_back(line);
    }
    return file_manager_.writeAllLines("data/payments.txt", lines, error);
}

bool PaymentRepository::savePayment(const Payment& payment, std::string& error) {
    LinkedList<Payment> list;
    loadAllPayments(list, error);
    list.push_back(payment);
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