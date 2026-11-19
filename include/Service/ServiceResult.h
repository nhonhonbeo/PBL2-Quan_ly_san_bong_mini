#pragma once

#include <optional>
#include <string>
#include <utility>

enum class ServiceErrorCode {
    None,
    InvalidData,
    NotFound,
    Unauthorized,
    ScheduleConflict,
    StorageFailure,
    BusinessRuleViolation
};

// LÝ DO CHỌN: Mã lỗi tách biệt với thông báo để GUI có thể xử lý lỗi ổn định,
// còn thông báo vẫn giữ nội dung thân thiện với người dùng.
struct ServiceResult {
    ServiceErrorCode code = ServiceErrorCode::None;
    std::string message;

    bool isSuccess() const noexcept {
        return code == ServiceErrorCode::None;
    }

    explicit operator bool() const noexcept {
        return isSuccess();
    }

    static ServiceResult success() {
        return {};
    }

    static ServiceResult failure(ServiceErrorCode code, std::string message) {
        return {code, std::move(message)};
    }
};

template <typename T>
struct ServiceResultT {
    ServiceErrorCode code = ServiceErrorCode::None;
    std::string message;
    std::optional<T> value;

    bool isSuccess() const noexcept {
        return code == ServiceErrorCode::None && value.has_value();
    }

    explicit operator bool() const noexcept {
        return isSuccess();
    }

    static ServiceResultT success(T value) {
        return {ServiceErrorCode::None, {}, std::move(value)};
    }

    static ServiceResultT failure(ServiceErrorCode code, std::string message) {
        return {code, std::move(message), std::nullopt};
    }
};
