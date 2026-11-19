#include "GUI/App/Facade.h"

#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "Core/DateTime.h"
#include "Field/Field.h"
#include "GUI/App/Session.h"
#include "GUI/Screens/Owner/OwnerShellScreen.h"
#include "GUI/Screens/Renter/RenterShellScreen.h"
#include "GUI/App/FacadeMappers.h"

namespace GUI::App {

using namespace detail;

Facade::Facade()
    : file_manager_(), account_repository_(file_manager_),
      venue_repository_(file_manager_), customer_repository_(file_manager_),
      booking_repository_(venue_repository_),
      report_repository_(venue_repository_),
      tournament_repository_(file_manager_),
      field_repository_(venue_repository_),
      account_service_(account_repository_, customer_repository_),
      venue_service_(venue_repository_), customer_service_(customer_repository_),
      booking_service_(booking_repository_, customer_service_, venue_service_),
      report_service_(report_repository_),
      tournament_service_(tournament_repository_, booking_repository_),
      field_service_(field_repository_, venue_service_) {
    loadData();
}

Facade& Facade::getInstance() {
    static Facade instance;
    return instance;
}

void Facade::refreshCache() {
    std::string err;
    if (!account_service_.loadAccounts(err)) {
        std::cerr << "Không thể nạp tài khoản: " << err << '\n';
    }
    danhSachUser_ = account_service_.getAccounts();

    User* cur = Session::getInstance().getUser();
    if (cur) {
        for (const auto& u : danhSachUser_) {
            if (u && u->get_user_id() == cur->get_user_id()) {
                Session::getInstance().login(u, u->get_account_type());
                break;
            }
        }
    }

    if (!venue_service_.initializeStorage(err)) {
        std::cerr << "Không thể khởi tạo dữ liệu sân: " << err << '\n';
    }

    std::map<std::tuple<std::string, int, int, std::string>, long long>
        updatedPriceCache;
    LinkedList<VenuePriceRecord> prices;
    if (venue_service_.getPriceMatrix(prices, err)) {
        for (auto* node = prices.get_head(); node != nullptr; node = node->next) {
            const VenuePriceRecord& price = node->data;
            updatedPriceCache[std::make_tuple(
                price.ownerId, price.fieldSize, price.durationMinutes,
                price.startTime
            )] = price.priceVnd;
        }
        giaMaTranCache_.swap(updatedPriceCache);
    } else {
        std::cerr << "Không thể nạp bảng giá sân: " << err << '\n';
    }

    danhSachSan_.clear();
    if (!loadAnhSan(err)) {
        std::cerr << "Không thể nạp đường dẫn ảnh sân: " << err << '\n';
    }
    LinkedList<ReportFieldRecord> repFields;
    if (report_service_.loadFieldRecords(repFields, err)) {
        for (auto* n = repFields.get_head(); n; n = n->next) {
            danhSachSan_.push_back({n->data.fieldId, n->data.ownerUserId, n->data.name,
                                    n->data.size, static_cast<int>(n->data.hourlyRateVnd),
                                    n->data.underMaintenance, n->data.rules, getAnhSan(n->data.fieldId)});
        }
    } else {
        std::cerr << "Không thể nạp danh sách sân: " << err << '\n';
    }

    danhSachDonDat_.clear();
    LinkedList<ReportBookingRecord> repBookings;
    if (report_service_.loadBookingRecords(repBookings, err)) {
        for (auto* n = repBookings.get_head(); n; n = n->next) {
            danhSachDonDat_.push_back(toBookingInfo(n->data));
        }
    } else {
        std::cerr << "Không thể nạp lịch sử đặt sân: " << err << '\n';
    }

    danhSachNguongUuDai_.clear();
    for (const auto& t : customer_service_.getDiscountThresholds()) {
        danhSachNguongUuDai_.push_back({t.id, t.minBookings, t.percentage, t.tierName});
    }

    for (const auto& kh : danhSachKhachHang_) {
        mapIndexKhachSDT_.remove(kh.sdt);
    }
    danhSachKhachHang_.clear();
    LinkedList<Customer> customers;
    if (!customer_service_.getAllCustomers(customers, err)) {
        std::cerr << "Không thể nạp danh sách khách hàng: " << err << '\n';
    }
    for (auto* n = customers.get_head(); n; n = n->next) {
        KhachHangInfo kh{n->data.getUserId(), n->data.get_name(), n->data.get_phone_number(),
                         n->data.getBookingCount(), n->data.getCancelCount(), false, "", "", 0};
        if (!customer_service_.isBlacklisted(
                kh.sdt, kh.biChan, kh.lyDoChan, err)) {
            std::cerr << "Không thể kiểm tra danh sách đen: " << err << '\n';
        }
        std::string discountError;
        kh.mucUuDaiPhanTram =
            customer_service_.getCustomerDiscountPercentage(kh.sdt, discountError);
        if (!discountError.empty()) {
            std::cerr << "Không thể nạp ưu đãi khách hàng: "
                      << discountError << '\n';
        }
        mapIndexKhachSDT_.insert(kh.sdt, danhSachKhachHang_.size());
        danhSachKhachHang_.push_back(kh);
    }

    LinkedList<Blacklist> blacklists;
    if (!customer_service_.getAllBlacklists(blacklists, err)) {
        std::cerr << "Không thể nạp danh sách đen: " << err << '\n';
    }
    for (auto* n = blacklists.get_head(); n; n = n->next) {
        size_t idx = 0;
        if (!mapIndexKhachSDT_.search(n->data.getCustomerId(), idx)) {
            KhachHangInfo bkh{n->data.getCustomerId(), n->data.getCustomerId(), n->data.getCustomerId(),
                              0, 0, n->data.isActive(), n->data.getReason(), n->data.getCreatedAt(), 0};
            mapIndexKhachSDT_.insert(bkh.sdt, danhSachKhachHang_.size());
            danhSachKhachHang_.push_back(bkh);
        }
    }

    danhSachDanhGia_.clear();
    LinkedList<Review> reviews;
    if (!customer_service_.getAllReviews(reviews, err)) {
        std::cerr << "Không thể nạp đánh giá: " << err << '\n';
    }
    for (auto* n = reviews.get_head(); n; n = n->next) {
        danhSachDanhGia_.push_back({n->data.getReviewId(), n->data.getCustomerName(),
                                   n->data.getCustomerId(), n->data.getFieldId(),
                                   n->data.getRating(), n->data.getCreatedAt(), n->data.getComment()});
    }

    danhSachGiaiDau_.clear();
    LinkedList<Tournament> tourList;
    if (!tournament_service_.loadAllTournaments(tourList, err)) {
        std::cerr << "Không thể nạp danh sách giải đấu: " << err << '\n';
    }
    for (auto* n = tourList.get_head(); n; n = n->next) danhSachGiaiDau_.push_back(toGiaiDauInfo(n->data));
}
void Facade::loadData() {
    if (!daNapDuLieu_) {
        refreshCache();
        daNapDuLieu_ = true;
    }
}

void Facade::saveData() {
    std::string error;
    if (!saveAnhSan(error)) {
        std::cerr << "Không thể lưu đường dẫn ảnh sân: " << error << '\n';
    }
}

}  // namespace GUI::App
