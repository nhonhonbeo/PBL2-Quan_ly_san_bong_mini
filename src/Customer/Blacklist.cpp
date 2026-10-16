#include "Customer/Blacklist.h"

Blacklist::Blacklist(const std::string& customerId, const std::string& reason,
                     const std::string& createdAt, bool active)
    : customer_id_(customerId), reason_(reason), created_at_(createdAt),
      active_(active) {}

const std::string& Blacklist::getCustomerId() const { return customer_id_; }

const std::string& Blacklist::getReason() const { return reason_; }

const std::string& Blacklist::getCreatedAt() const { return created_at_; }

bool Blacklist::isActive() const { return active_; }

void Blacklist::deactivate() { active_ = false; }
