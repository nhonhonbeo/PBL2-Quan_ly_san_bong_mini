#include "GUI/App/Facade.h"

#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "Core/DateTime.h"
#include "Field/Field.h"
#include "GUI/App/Session.h"
#include "GUI/App/FacadeMappers.h"

namespace GUI::App {

using namespace detail;

std::vector<KhachHangInfo> Facade::getDanhSachKhachHang() const { return danhSachKhachHang_; }

std::vector<KhachHangInfo> Facade::getDanhSachKhachHangCuaChuSan() const {
    User* owner = Session::getInstance().getUser();
    if (!owner) return danhSachKhachHang_;
    std::unordered_map<std::string, bool> booked;
    for (const auto& b : danhSachDonDat_) if (b.ownerUserId == owner->get_user_id()) booked[b.sdt] = true;
    std::vector<KhachHangInfo> list;
    for (const auto& kh : danhSachKhachHang_) if (booked[kh.sdt]) list.push_back(kh);
    return list;
}

KhachHangInfo* Facade::timKhachHangTheoSDT(const std::string& sdt) {
    // LÝ DO CHỌN: HashTable tra cứu chỉ mục khách hàng theo SĐT trong O(1).
    size_t idx = 0;
    if (mapIndexKhachSDT_.search(sdt, idx) && idx < danhSachKhachHang_.size()) return &danhSachKhachHang_[idx];
    return nullptr;
}

std::vector<DanhGiaInfo> Facade::getDanhSachDanhGia() const {
    User* owner = Session::getInstance().getUser();
    std::vector<DanhGiaInfo> reviews;
    if (!owner) return reviews;
    for (const auto& r : danhSachDanhGia_) {
        for (const auto& f : danhSachSan_) {
            if (f.maSan == r.maSan && f.ownerUserId == owner->get_user_id()) { reviews.push_back(r); break; }
        }
    }
    return reviews;
}

float Facade::getDiemTrungBinhToanCoSo() const {
    float score = 0.0f; std::string err;
    return customer_service_.getAverageRatingOverall(score, err) ? score : 0.0f;
}

std::map<std::string, float> Facade::getDiemTrungBinhTungSan() const {
    std::map<std::string, float> result, ratings;
    for (const auto& s : getDanhSachSanBongCuaChuSan()) result[s.maSan] = 0.0f;
    std::string err;
    if (customer_service_.getAverageRatingsMap(ratings, err)) {
        for (auto& item : result) {
            auto it = ratings.find(item.first);
            if (it != ratings.end()) item.second = it->second;
        }
    }
    return result;
}

std::vector<DonDatSanInfo> Facade::getLichSuDatCuaKhach(const std::string& sdt) const {
    std::vector<DonDatSanInfo> lichSu;
    for (const auto& b : danhSachDonDat_) if (b.sdt == sdt) lichSu.push_back(b);
    return lichSu;
}

std::vector<DonDatSanInfo> Facade::getLichSuDatCuaKhachCuaChuSan(const std::string& sdt) const {
    User* owner = Session::getInstance().getUser();
    std::vector<DonDatSanInfo> history;
    if (!owner) return history;
    for (const auto& b : getLichSuDatCuaKhach(sdt)) if (b.ownerUserId == owner->get_user_id()) history.push_back(b);
    return history;
}

std::vector<DanhGiaInfo> Facade::getDanhGiaCuaKhach(const std::string& sdt) const {
    std::vector<DanhGiaInfo> list;
    for (const auto& r : danhSachDanhGia_) if (r.sdt == sdt) list.push_back(r);
    return list;
}

std::vector<DanhGiaInfo> Facade::getDanhGiaCuaKhachCuaChuSan(const std::string& sdt) const {
    User* owner = Session::getInstance().getUser();
    std::vector<DanhGiaInfo> list;
    if (!owner) return list;
    for (const auto& r : getDanhGiaCuaKhach(sdt)) {
        for (const auto& s : danhSachSan_) {
            if (s.maSan == r.maSan && s.ownerUserId == owner->get_user_id()) { list.push_back(r); break; }
        }
    }
    return list;
}

}  // namespace GUI::App
