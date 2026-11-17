#include "Service/FieldService.h"

#include <cstdio>
#include "Core/DateTime.h"
#include "Field/Field.h"
#include "Field/TimeSlot.h"
#include "Repository/FieldRepository.h"
#include "Repository/VenueRepository.h"
#include "Service/VenueService.h"

namespace {

bool isValidSize(int size) {
    return size == 5 || size == 7 || size == 11;
}

}  // namespace

FieldService::FieldService(FieldRepository& fieldRepository,
                           VenueService* venueService,
                           VenueRepository* venueRepository)
    : field_repository_(fieldRepository),
      venue_service_(venueService),
      venue_repository_(venueRepository),
      default_venue_repo_(),
      default_venue_service_(default_venue_repo_) {
    if (!venue_repository_) venue_repository_ = &default_venue_repo_;
    if (!venue_service_) venue_service_ = &default_venue_service_;
}

void FieldService::setVenueDependencies(VenueService* venueService, VenueRepository* venueRepository) {
    venue_service_ = venueService ? venueService : &default_venue_service_;
    venue_repository_ = venueRepository ? venueRepository : &default_venue_repo_;
}

bool FieldService::createMultipleFields(const std::string& requesterOwnerId, int loaiSan,
                                        int soLuong, const std::string& quyDinh,
                                        LinkedList<std::string>& outMaSan, std::string& error) {
    outMaSan.clear();
    if (requesterOwnerId.empty()) {
        error = u8"Không xác định được chủ sân yêu cầu.";
        return false;
    }
    if (!isValidSize(loaiSan)) {
        error = u8"Loại sân không hợp lệ.";
        return false;
    }
    if (soLuong <= 0) {
        error = u8"Số lượng sân cần tạo phải lớn hơn 0.";
        return false;
    }
    LinkedList<VenueFieldRecord> fields;
    if (!field_repository_.loadAll(fields, error)) return false;

    int seq = 1;
    const std::string prefix = "S" + std::to_string(loaiSan) + "-";
    for (auto* n = fields.get_head(); n != nullptr; n = n->next) {
        if (n->data.fieldId.rfind(prefix, 0) == 0) {
            try {
                int ex = std::stoi(n->data.fieldId.substr(prefix.size()));
                if (ex >= seq) seq = ex + 1;
            } catch (...) {}
        }
    }

    for (int i = 0; i < soLuong; ++i) {
        int stt = seq + i;
        char maBuf[16];
        std::snprintf(maBuf, sizeof(maBuf), "S%d-%02d", loaiSan, stt);
        VenueFieldRecord newF{maBuf, requesterOwnerId,
                              u8"Sân " + std::to_string(loaiSan) + u8" số " +
                              (stt < 10 ? "0" : "") + std::to_string(stt),
                              loaiSan, 0, quyDinh};
        if (!venue_service_->createField(requesterOwnerId, newF, error)) return false;
        outMaSan.push_back(maBuf);
    }
    return true;
}

bool FieldService::createField(const std::string& requesterOwnerId,
                               const VenueFieldRecord& field,
                               std::string& error) {
    return venue_service_->createField(requesterOwnerId, field, error);
}

bool FieldService::updateField(const std::string& requesterOwnerId,
                               const VenueFieldRecord& field,
                               std::string& error) {
    return venue_service_->updateField(requesterOwnerId, field, error);
}

bool FieldService::deleteField(const std::string& requesterOwnerId,
                               const std::string& fieldId,
                               std::string& error) {
    return venue_service_->deleteField(requesterOwnerId, fieldId, error);
}

bool FieldService::setFieldStatus(const std::string& requesterOwnerId,
                                  const std::string& fieldId,
                                  FieldStatus status,
                                  std::string& error) {
    LinkedList<VenueFieldRecord> fields;
    if (!field_repository_.loadAll(fields, error)) return false;
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        VenueFieldRecord& field = node->data;
        if (field.fieldId != fieldId) continue;
        if (field.ownerId != requesterOwnerId) {
            error = "Không có quyền thay đổi trạng thái sân của chủ sân khác.";
            return false;
        }
        field.status = static_cast<int>(status);
        return venue_repository_->saveFields(fields, error);
    }
    error = "Không tìm thấy sân cần cập nhật trạng thái.";
    return false;
}

bool FieldService::getFields(const std::string& ownerId, int fieldSize,
                             bool includeUnavailable,
                             LinkedList<VenueFieldRecord>& fields,
                             std::string& error) const {
    if (fieldSize != 0 && !isValidSize(fieldSize)) {
        error = "Loại sân không hợp lệ.";
        return false;
    }
    return venue_service_->getFields(ownerId, fieldSize, includeUnavailable, fields, error);
}

bool FieldService::setPrice(const std::string& requesterOwnerId,
                            int fieldSize,
                            int durationMinutes,
                            const std::string& startTime,
                            long long priceVnd,
                            std::string& error) const {
    return venue_service_->setFieldPrice(
        requesterOwnerId, fieldSize, durationMinutes, startTime, priceVnd, error
    );
}

bool FieldService::validateOperatingSlot(const Field& field,
                                         const TimeSlot& slot,
                                         std::string& error) const {
    if (!DateTime::isValidTime(slot.getStartTime()) ||
        !DateTime::isValidTime(slot.getEndTime())) {
        error = "Giờ bắt đầu hoặc kết thúc không đúng định dạng HH:MM.";
        return false;
    }
    if (DateTime::compareTime(slot.getStartTime(), slot.getEndTime()) >= 0) {
        error = "Giờ kết thúc phải sau giờ bắt đầu.";
        return false;
    }
    if (DateTime::compareTime(slot.getStartTime(), field.getOpenTime()) < 0 ||
        DateTime::compareTime(slot.getEndTime(), field.getCloseTime()) > 0) {
        error = "Khung giờ nằm ngoài giờ hoạt động.";
        return false;
    }
    return true;
}

