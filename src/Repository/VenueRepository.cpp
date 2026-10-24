#include "Repository/VenueRepository.h"

#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "Storage/FileManager.h"

namespace {

void tachTruong(const std::string& line, LinkedList<std::string>& fields) {
    fields.clear();
    std::istringstream input(line);
    std::string value;
    while (std::getline(input, value, '|')) {
        fields.push_back(value);
    }
    if (!line.empty() && line.back() == '|') {
        fields.push_back("");
    }
}

const std::string* truongTai(const LinkedList<std::string>& fields, int index) {
    auto* node = fields.get_head();
    while (node != nullptr && index > 0) {
        node = node->next;
        --index;
    }
    return node == nullptr ? nullptr : &node->data;
}

int loaiSanTuMa(const std::string& fieldId);

bool parseSo(const std::string& value, long long& result) {
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

std::string ownerMacDinh(const LinkedList<VenueFacilityRecord>& facilities) {
    for (auto* node = facilities.get_head(); node != nullptr; node = node->next) {
        if (node->data.ownerId == "US01") {
            return node->data.ownerId;
        }
    }
    auto* head = facilities.get_head();
    return head == nullptr ? std::string() : head->data.ownerId;
}

bool coField(const LinkedList<VenueFieldRecord>& fields,
             const std::string& fieldId) {
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        if (node->data.fieldId == fieldId) {
            return true;
        }
    }
    return false;
}

void ensureLegacyField(LinkedList<VenueFieldRecord>& fields,
                       const std::string& fieldId,
                       const std::string& ownerId) {
    if (fieldId.empty() || ownerId.empty() || coField(fields, fieldId)) return;
    VenueFieldRecord field;
    field.fieldId = fieldId;
    field.ownerId = ownerId;
    field.name = "Sân lịch sử " + fieldId;
    field.size = loaiSanTuMa(fieldId);
    field.status = 1;
    field.rules = "Sân lịch sử đã migrate, không nhận lượt đặt mới.";
    fields.push_back(field);
}

std::string timCustomerIdTheoSDT(const std::string& phone) {
    FileManager manager;
    LinkedList<std::string> accounts;
    std::string ignoredError;
    if (!manager.readAllLines("data/accounts.txt", accounts, ignoredError)) {
        return phone;
    }
    for (auto* node = accounts.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        const std::string* kind = truongTai(parts, 0);
        const std::string* id = truongTai(parts, 1);
        const std::string* accountPhone = truongTai(parts, 5);
        if (kind != nullptr && *kind == "CUSTOMER" && id != nullptr &&
            accountPhone != nullptr && *accountPhone == phone) {
            return *id;
        }
    }
    return phone;
}

int loaiSanTuMa(const std::string& fieldId) {
    if (fieldId.rfind("S11-", 0) == 0) return 11;
    if (fieldId.rfind("S7-", 0) == 0) return 7;
    return 5;
}

std::string tenTrangThai(int status) {
    if (status == 1) return "INACTIVE";
    if (status == 2) return "MAINTENANCE";
    return "ACTIVE";
}

bool atomicWrite(const std::string& path,
                 const LinkedList<std::string>& lines,
                 std::string& error) {
    FileManager manager;
    const std::string tempPath = path + ".tmp";
    if (!manager.writeAllLines(tempPath, lines, error)) {
        return false;
    }
    std::error_code ec;
    std::filesystem::remove(path + ".replace", ec);
    ec.clear();
    if (std::filesystem::exists(path, ec)) {
        std::filesystem::copy_file(
            path,
            path + ".replace",
            std::filesystem::copy_options::overwrite_existing,
            ec
        );
        if (ec) {
            error = "Không thể sao lưu dữ liệu trước khi cập nhật: " + path;
            std::filesystem::remove(tempPath, ec);
            return false;
        }
        std::filesystem::remove(path, ec);
        if (ec) {
            error = "Không thể thay thế dữ liệu: " + path;
            std::filesystem::remove(tempPath, ec);
            return false;
        }
    }
    std::filesystem::rename(tempPath, path, ec);
    if (ec) {
        error = "Không thể hoàn tất cập nhật dữ liệu: " + path;
        return false;
    }
    return true;
}

}  // Kết thúc không gian tên ẩn danh

bool VenueRepository::loadFacilities(
    LinkedList<VenueFacilityRecord>& records,
    std::string& error
) const {
    FileManager manager;
    LinkedList<std::string> lines;
    if (!manager.readAllLines("data/accounts.txt", lines, error)) return false;
    records.clear();
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        const std::string* kind = truongTai(parts, 0);
        if (kind == nullptr || *kind != "OWNER") continue;
        const std::string* ownerId = truongTai(parts, 1);
        const std::string* phone = truongTai(parts, 5);
        const std::string* name = truongTai(parts, 6);
        const std::string* address = truongTai(parts, 7);
        const std::string* open = truongTai(parts, 9);
        const std::string* close = truongTai(parts, 10);
        if (ownerId == nullptr || phone == nullptr || name == nullptr ||
            address == nullptr || open == nullptr || close == nullptr) {
            error = "Bản ghi chủ sân trong data/accounts.txt không hợp lệ.";
            return false;
        }
        VenueFacilityRecord record;
        record.ownerId = *ownerId;
        record.name = *name;
        record.address = *address;
        record.phone = *phone;
        record.openTime = *open;
        record.closeTime = *close;
        if (const std::string* enabled60 = truongTai(parts, 12)) {
            record.allows60Minutes = *enabled60 != "0";
        }
        if (const std::string* enabled90 = truongTai(parts, 13)) {
            record.allows90Minutes = *enabled90 != "0";
        }
        records.push_back(record);
    }
    return true;
}

bool VenueRepository::loadFields(
    LinkedList<VenueFieldRecord>& records,
    std::string& error
) const {
    FileManager manager;
    LinkedList<std::string> lines;
    if (!manager.readAllLines("data/fields.txt", lines, error)) return false;
    records.clear();
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        VenueFieldRecord field;
        long long size = 0;
        if (parts.size() == 6) {
            field.fieldId = *truongTai(parts, 0);
            field.ownerId = *truongTai(parts, 1);
            field.name = *truongTai(parts, 2);
            if (!parseSo(*truongTai(parts, 3), size)) {
                error = "Loại sân trong data/fields.txt không hợp lệ.";
                return false;
            }
            field.size = static_cast<int>(size);
            const std::string& status = *truongTai(parts, 4);
            field.status = status == "MAINTENANCE" ? 2 : (status == "INACTIVE" ? 1 : 0);
            field.rules = *truongTai(parts, 5);
        } else if (parts.size() == 7) {
            long long oldRate = 0;
            if (!parseSo(*truongTai(parts, 3), size) ||
                !parseSo(*truongTai(parts, 4), oldRate)) {
                error = "Bản ghi sân cũ trong data/fields.txt không hợp lệ.";
                return false;
            }
            field.fieldId = *truongTai(parts, 0);
            field.ownerId = *truongTai(parts, 1);
            field.name = *truongTai(parts, 2);
            field.size = static_cast<int>(size);
            field.status = *truongTai(parts, 5) == "1" ? 2 : 0;
            field.rules = *truongTai(parts, 6);
        } else {
            error = "Bản ghi sân trong data/fields.txt không đúng định dạng.";
            return false;
        }
        records.push_back(field);
    }
    return true;
}

bool VenueRepository::saveFields(
    const LinkedList<VenueFieldRecord>& records,
    std::string& error
) const {
    LinkedList<std::string> lines;
    for (auto* node = records.get_head(); node != nullptr; node = node->next) {
        const VenueFieldRecord& field = node->data;
        lines.push_back(
            FileManager::join(field.fieldId) + "|" +
            FileManager::join(field.ownerId) + "|" +
            FileManager::join(field.name) + "|" +
            std::to_string(field.size) + "|" +
            tenTrangThai(field.status) + "|" +
            FileManager::join(field.rules)
        );
    }
    return atomicWrite("data/fields.txt", lines, error);
}

bool VenueRepository::loadPrices(
    LinkedList<VenuePriceRecord>& records,
    std::string& error
) const {
    FileManager manager;
    LinkedList<std::string> lines;
    if (!manager.readAllLines("data/prices.txt", lines, error)) return false;
    records.clear();
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        long long size = 0;
        long long duration = 0;
        long long price = 0;
        if (parts.size() != 5 ||
            !parseSo(*truongTai(parts, 1), size) ||
            !parseSo(*truongTai(parts, 2), duration) ||
            !parseSo(*truongTai(parts, 4), price)) {
            error = "Bản ghi giá trong data/prices.txt không hợp lệ.";
            return false;
        }
        VenuePriceRecord record;
        record.ownerId = *truongTai(parts, 0);
        record.fieldSize = static_cast<int>(size);
        record.durationMinutes = static_cast<int>(duration);
        record.startTime = *truongTai(parts, 3);
        record.priceVnd = price;
        records.push_back(record);
    }
    return true;
}

bool VenueRepository::savePrices(
    const LinkedList<VenuePriceRecord>& records,
    std::string& error
) const {
    LinkedList<std::string> lines;
    for (auto* node = records.get_head(); node != nullptr; node = node->next) {
        const VenuePriceRecord& price = node->data;
        lines.push_back(
            FileManager::join(price.ownerId) + "|" +
            std::to_string(price.fieldSize) + "|" +
            std::to_string(price.durationMinutes) + "|" +
            FileManager::join(price.startTime) + "|" +
            std::to_string(price.priceVnd)
        );
    }
    return atomicWrite("data/prices.txt", lines, error);
}

bool VenueRepository::updateFacilityHours(
    const std::string& ownerId,
    const std::string& openTime,
    const std::string& closeTime,
    bool allows60Minutes,
    bool allows90Minutes,
    std::string& error
) const {
    FileManager manager;
    LinkedList<std::string> accounts;
    if (!manager.readAllLines("data/accounts.txt", accounts, error)) return false;
    bool found = false;
    for (auto* node = accounts.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> fields;
        tachTruong(node->data, fields);
        const std::string* kind = truongTai(fields, 0);
        const std::string* id = truongTai(fields, 1);
        if (kind == nullptr || *kind != "OWNER" || id == nullptr ||
            *id != ownerId) {
            continue;
        }
        LinkedList<std::string> updatedFields;
        const int lastIndex = fields.size() > 13 ? fields.size() - 1 : 13;
        auto* field = fields.get_head();
        for (int index = 0; index <= lastIndex; ++index) {
            const std::string* existing =
                field == nullptr ? nullptr : &field->data;
            if (field != nullptr) field = field->next;
            if (index == 9) updatedFields.push_back(openTime);
            else if (index == 10) updatedFields.push_back(closeTime);
            else if (index == 12) {
                updatedFields.push_back(allows60Minutes ? "1" : "0");
            } else if (index == 13) {
                updatedFields.push_back(allows90Minutes ? "1" : "0");
            } else if (existing != nullptr) {
                updatedFields.push_back(*existing);
            } else {
                updatedFields.push_back("");
            }
        }
        std::string line;
        bool first = true;
        for (auto* field = updatedFields.get_head(); field != nullptr; field = field->next) {
            if (!first) line += '|';
            line += FileManager::join(field->data);
            first = false;
        }
        node->data = line;
        found = true;
        break;
    }
    if (!found) {
        error = "Không có quyền thay đổi giờ hoạt động của cơ sở này.";
        return false;
    }
    return atomicWrite("data/accounts.txt", accounts, error);
}

bool VenueRepository::loadBookings(
    LinkedList<VenueBookingRecord>& records,
    std::string& error
) const {
    FileManager manager;
    LinkedList<std::string> lines;
    if (!manager.readAllLines("data/bookings.txt", lines, error)) return false;
    records.clear();
    LinkedList<VenueFacilityRecord> facilities;
    if (!loadFacilities(facilities, error)) return false;
    const std::string defaultOwner = ownerMacDinh(facilities);

    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        VenueBookingRecord booking;
        if (parts.size() == 12) {
            long long duration = 0;
            long long price = 0;
            if (!parseSo(*truongTai(parts, 9), duration) ||
                !parseSo(*truongTai(parts, 10), price)) {
                error = "Giá hoặc thời lượng trong data/bookings.txt không hợp lệ.";
                return false;
            }
            booking.bookingId = *truongTai(parts, 0);
            booking.fieldId = *truongTai(parts, 1);
            booking.ownerId = *truongTai(parts, 2);
            booking.customerId = *truongTai(parts, 3);
            booking.customerName = *truongTai(parts, 4);
            booking.customerPhone = *truongTai(parts, 5);
            booking.date = *truongTai(parts, 6);
            booking.startTime = *truongTai(parts, 7);
            booking.endTime = *truongTai(parts, 8);
            booking.durationMinutes = static_cast<int>(duration);
            booking.priceVnd = price;
            booking.status = *truongTai(parts, 11);
        } else if (parts.size() == 8 || parts.size() == 9) {
            booking.bookingId = *truongTai(parts, 0);
            booking.fieldId = *truongTai(parts, 1);
            booking.customerName = *truongTai(parts, 2);
            booking.customerPhone = *truongTai(parts, 3);
            booking.date = *truongTai(parts, 4);
            const std::string time = *truongTai(parts, 5);
            const size_t separator = time.find('-');
            if (separator == std::string::npos) {
                error = "Khung giờ trong bản ghi đặt cũ không hợp lệ.";
                return false;
            }
            booking.startTime = time.substr(0, separator);
            booking.endTime = time.substr(separator + 1);
            long long price = 0;
            if (!parseSo(*truongTai(parts, 6), price)) {
                error = "Giá trong bản ghi đặt cũ không hợp lệ.";
                return false;
            }
            booking.priceVnd = price;
            booking.durationMinutes = 90;
            booking.status = *truongTai(parts, 7);
            booking.ownerId = parts.size() == 9
                ? *truongTai(parts, 8)
                : defaultOwner;
            booking.customerId = timCustomerIdTheoSDT(booking.customerPhone);
        } else {
            error = "Bản ghi đặt sân trong data/bookings.txt không đúng định dạng.";
            return false;
        }
        records.push_back(booking);
    }
    return true;
}

bool VenueRepository::saveBookings(
    const LinkedList<VenueBookingRecord>& records,
    std::string& error
) const {
    LinkedList<std::string> lines;
    for (auto* node = records.get_head(); node != nullptr; node = node->next) {
        const VenueBookingRecord& booking = node->data;
        lines.push_back(
            FileManager::join(booking.bookingId) + "|" +
            FileManager::join(booking.fieldId) + "|" +
            FileManager::join(booking.ownerId) + "|" +
            FileManager::join(booking.customerId) + "|" +
            FileManager::join(booking.customerName) + "|" +
            FileManager::join(booking.customerPhone) + "|" +
            FileManager::join(booking.date) + "|" +
            FileManager::join(booking.startTime) + "|" +
            FileManager::join(booking.endTime) + "|" +
            std::to_string(booking.durationMinutes) + "|" +
            std::to_string(booking.priceVnd) + "|" +
            FileManager::join(booking.status)
        );
    }
    return atomicWrite("data/bookings.txt", lines, error);
}

bool VenueRepository::migrateLegacyData(std::string& error) const {
    LinkedList<VenueFieldRecord> fields;
    LinkedList<VenueBookingRecord> bookings;
    if (!loadFields(fields, error) || !loadBookings(bookings, error)) return false;

    bool needsMigration = false;
    FileManager manager;
    LinkedList<std::string> fieldLines;
    LinkedList<std::string> bookingLines;
    if (!manager.readAllLines("data/fields.txt", fieldLines, error) ||
        !manager.readAllLines("data/bookings.txt", bookingLines, error)) {
        return false;
    }
    for (auto* node = fieldLines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        if (parts.size() != 6) needsMigration = true;
    }
    for (auto* node = bookingLines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        if (parts.size() != 12) needsMigration = true;
    }
    if (!needsMigration) return true;

    LinkedList<VenueFacilityRecord> facilities;
    if (!loadFacilities(facilities, error)) return false;
    const std::string defaultOwner = ownerMacDinh(facilities);
    if (defaultOwner.empty() && !bookings.isEmpty()) {
        error = "Không tìm thấy chủ sân mặc định để migrate dữ liệu cũ.";
        return false;
    }

    for (auto* bookingNode = bookings.get_head();
         bookingNode != nullptr;
         bookingNode = bookingNode->next) {
        VenueBookingRecord& booking = bookingNode->data;
        if (booking.ownerId.empty()) booking.ownerId = defaultOwner;
        if (booking.customerId == booking.customerPhone)
            booking.customerId = timCustomerIdTheoSDT(booking.customerPhone);
        ensureLegacyField(fields, booking.fieldId, booking.ownerId);
    }

    for (const char* source : {"data/reviews.txt", "data/tournaments.txt"}) {
        LinkedList<std::string> sourceLines;
        if (!manager.readAllLines(source, sourceLines, error)) return false;
        for (auto* node = sourceLines.get_head(); node != nullptr; node = node->next) {
            LinkedList<std::string> parts;
            tachTruong(node->data, parts);
            const int fieldIndex = std::string(source) == "data/reviews.txt" ? 3 : 6;
            const std::string* fieldId = truongTai(parts, fieldIndex);
            if (fieldId != nullptr)
                ensureLegacyField(fields, *fieldId, defaultOwner);
        }
    }

    for (auto* fieldNode = fields.get_head();
         fieldNode != nullptr;
         fieldNode = fieldNode->next) {
        if (fieldNode->data.ownerId.empty()) {
            fieldNode->data.ownerId = defaultOwner;
        }
    }

    for (const char* path : {"data/fields.txt", "data/bookings.txt"}) {
        std::error_code ec;
        const std::string backupPath = std::string(path) + ".bak";
        if (std::filesystem::exists(path, ec) &&
            !std::filesystem::exists(backupPath, ec)) {
            std::filesystem::copy_file(
                path,
                backupPath,
                std::filesystem::copy_options::none,
                ec
            );
            if (ec) {
                error = "Không thể sao lưu dữ liệu cũ: " + std::string(path);
                return false;
            }
        }
    }
    return saveFields(fields, error) && saveBookings(bookings, error);
}

bool VenueRepository::seedIfNeeded(std::string& error) const {
    LinkedList<VenueFacilityRecord> facilities;
    LinkedList<VenueFieldRecord> fields;
    LinkedList<VenuePriceRecord> prices;
    FileManager manager;
    LinkedList<std::string> seedMarker;
    if (!loadFacilities(facilities, error) ||
        !loadFields(fields, error) ||
        !loadPrices(prices, error) ||
        !manager.readAllLines("data/venue_seeded.txt", seedMarker, error)) {
        return false;
    }

    bool pricesChanged = false;
    const bool seedInitialFields =
        fields.isEmpty() && seedMarker.isEmpty();
    for (auto* facilityNode = facilities.get_head();
         facilityNode != nullptr;
         facilityNode = facilityNode->next) {
        const VenueFacilityRecord& facility = facilityNode->data;
        bool hasActiveField = false;
        for (auto* fieldNode = fields.get_head();
             fieldNode != nullptr;
             fieldNode = fieldNode->next) {
            if (fieldNode->data.ownerId == facility.ownerId &&
                fieldNode->data.status == 0) {
                hasActiveField = true;
            }
        }
        if (seedInitialFields) {
            for (const int size : {5, 7, 11}) {
                bool hasSize = false;
                for (auto* fieldNode = fields.get_head();
                     fieldNode != nullptr;
                     fieldNode = fieldNode->next) {
                    if (fieldNode->data.ownerId == facility.ownerId &&
                        fieldNode->data.size == size) {
                        hasSize = true;
                    }
                }
                if (hasSize) continue;
                VenueFieldRecord field;
                field.ownerId = facility.ownerId;
                field.size = size;
                field.name = "Sân " + std::to_string(size) + " người";
                field.fieldId = "F-" + facility.ownerId + "-" +
                    std::to_string(size);
                bool duplicate = true;
                int suffix = 1;
                while (duplicate) {
                    duplicate = coField(fields, field.fieldId);
                    if (duplicate) {
                        field.fieldId = "F-" + facility.ownerId + "-" +
                            std::to_string(size) + "-" + std::to_string(suffix++);
                    }
                }
                fields.push_back(field);
                hasActiveField = true;
            }
        }
        if (!hasActiveField) continue;

        for (const int size : {5, 7, 11}) {
            for (const int duration : {60, 90}) {
                for (int minute = 6 * 60;
                     minute + duration <= 23 * 60;
                     minute += 30) {
                    const int hour = minute / 60;
                    const int min = minute % 60;
                    char start[6];
                    std::snprintf(start, sizeof(start), "%02d:%02d", hour, min);
                    bool exists = false;
                    for (auto* priceNode = prices.get_head();
                         priceNode != nullptr;
                         priceNode = priceNode->next) {
                        const VenuePriceRecord& price = priceNode->data;
                        if (price.ownerId == facility.ownerId &&
                            price.fieldSize == size &&
                            price.durationMinutes == duration &&
                            price.startTime == start) {
                            exists = true;
                            break;
                        }
                    }
                    if (exists) continue;
                    const bool peak = hour >= 17 && hour < 21;
                    long long base = static_cast<long long>(size) * 40000LL;
                    if (facility.ownerId != "US01") base += 50000LL;
                    VenuePriceRecord price;
                    price.ownerId = facility.ownerId;
                    price.fieldSize = size;
                    price.durationMinutes = duration;
                    price.startTime = start;
                    price.priceVnd = (base + (peak ? base / 2 : 0)) *
                                     duration / 60;
                    prices.push_back(price);
                    pricesChanged = true;
                }
            }
        }
    }

    bool fieldsChanged = false;
    LinkedList<VenueFieldRecord> originalFields;
    if (!loadFields(originalFields, error)) return false;
    fieldsChanged = originalFields.size() != fields.size();
    if (fieldsChanged && !saveFields(fields, error)) return false;
    if ((pricesChanged || prices.isEmpty()) && !savePrices(prices, error)) {
        return false;
    }
    if (seedMarker.isEmpty()) {
        seedMarker.push_back("1");
        if (!manager.writeAllLines("data/venue_seeded.txt", seedMarker, error)) {
            return false;
        }
    }
    return true;
}

bool VenueRepository::isCustomerBlacklisted(
    const std::string& customerId,
    const std::string& phone,
    std::string& reason,
    std::string& error
) const {
    FileManager manager;
    LinkedList<std::string> lines;
    if (!manager.readAllLines("data/blacklist.txt", lines, error)) return false;
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        const std::string* idOrPhone = truongTai(parts, 0);
        const std::string* text = truongTai(parts, 1);
        const std::string* active = truongTai(parts, 3);
        if (idOrPhone != nullptr && text != nullptr && active != nullptr &&
            *active == "1" &&
            (*idOrPhone == customerId || *idOrPhone == phone)) {
            reason = *text;
            return true;
        }
    }
    reason.clear();
    return false;
}

bool VenueRepository::getCustomerDetails(
    const std::string& customerId,
    std::string& name,
    std::string& phone,
    std::string& error
) const {
    FileManager manager;
    LinkedList<std::string> lines;
    if (!manager.readAllLines("data/accounts.txt", lines, error)) return false;
    for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
        LinkedList<std::string> parts;
        tachTruong(node->data, parts);
        const std::string* kind = truongTai(parts, 0);
        const std::string* id = truongTai(parts, 1);
        const std::string* accountName = truongTai(parts, 4);
        const std::string* accountPhone = truongTai(parts, 5);
        if (kind != nullptr && *kind == "CUSTOMER" && id != nullptr &&
            accountName != nullptr && accountPhone != nullptr &&
            *id == customerId) {
            name = *accountName;
            phone = *accountPhone;
            return true;
        }
    }
    error = "Không tìm thấy tài khoản người thuê.";
    return false;
}
