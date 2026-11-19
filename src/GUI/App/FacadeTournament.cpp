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

std::vector<GiaiDauInfo> Facade::getDanhSachGiaiDau() const {
    User* owner = Session::getInstance().getUser();
    std::vector<GiaiDauInfo> list;
    if (!owner || owner->get_account_type() != "Owner") return list;
    for (const auto& g : danhSachGiaiDau_) {
        for (const auto& f : danhSachSan_) {
            if (f.maSan == g.maSan && f.ownerUserId == owner->get_user_id()) { list.push_back(g); break; }
        }
    }
    return list;
}

std::vector<GiaiDauInfo> Facade::getYeuCauGiaiDauChoDuyet() const {
    std::vector<GiaiDauInfo> ds;
    for (const auto& g : getDanhSachGiaiDau()) if (g.trangThai == TrangThaiGiaiDau::ChoDuyet) ds.push_back(g);
    return ds;
}

int Facade::getSoLuongYeuCauChoDuyet() const { return static_cast<int>(getYeuCauGiaiDauChoDuyet().size()); }

GiaiDauInfo* Facade::getGiaiDau(const std::string& maGiai) {
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return nullptr;
    for (auto& g : danhSachGiaiDau_) {
        if (g.maGiai != maGiai) continue;
        for (const auto& f : danhSachSan_) {
            if (f.maSan == g.maSan && f.ownerUserId == owner->get_user_id()) return &g;
        }
    }
    return nullptr;
}

KetQua Facade::duyetGiaiDau(const std::string& maGiai) {
    std::string error;
    if (!tournament_service_.duyetGiaiDau(maGiai, error)) return {false, error};
    refreshCache();
    return {true, ""};
}

KetQua Facade::tuChoiGiaiDau(const std::string& maGiai, const std::string& lyDo) {
    std::string error;
    if (!tournament_service_.tuChoiGiaiDau(maGiai, lyDo, error)) return {false, error};
    refreshCache();
    return {true, ""};
}

KetQua Facade::taoGiaiDauThuCong(const GiaiDauInfo& giaiMoi) {
    Tournament t = toTournament(giaiMoi, TournamentStatus::CONFIRMED);
    std::string outId, error;
    if (!tournament_service_.taoGiaiDauThuCong(t, outId, error)) return {false, error};
    refreshCache();
    return {true, ""};
}

KetQua Facade::suaGiaiDau(const GiaiDauInfo& giaiCapNhat) {
    Tournament t = toTournament(giaiCapNhat, static_cast<TournamentStatus>(static_cast<int>(giaiCapNhat.trangThai)));
    std::string error;
    if (!tournament_service_.suaGiaiDau(t, error)) return {false, error};
    refreshCache();
    return {true, ""};
}

KetQua Facade::huyGiaiDau(const std::string& maGiai) {
    std::string error;
    if (!tournament_service_.huyGiaiDau(maGiai, error)) return {false, error};
    refreshCache();
    return {true, ""};
}

}  // namespace GUI::App
