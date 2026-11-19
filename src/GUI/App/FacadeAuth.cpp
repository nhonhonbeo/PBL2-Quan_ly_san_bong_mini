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

bool Facade::kiemTraSDTDaTonTai(const std::string& sdt) const {
    for (const auto& u : danhSachUser_) if (u && u->get_phone_number() == sdt) return true;
    return false;
}

KetQuaDuLieu<std::shared_ptr<User>> Facade::dangNhap(const std::string& sdt, const std::string& matKhau) {
    loadData();
    std::string err;
    std::shared_ptr<User> user = account_service_.authenticate(sdt, matKhau, err);
    if (!user) {
        if (!kiemTraSDTDaTonTai(sdt)) return {false, u8"Số điện thoại này chưa được đăng ký trong hệ thống.", nullptr};
        return {false, u8"Mật khẩu đăng nhập không chính xác.", nullptr};
    }
    Session::getInstance().login(user, user->get_account_type());
    return {true, "", user};
}

KetQua Facade::dangKyNguoiThue(const std::string& hoTen, const std::string& sdt, const std::string& matKhau) {
    loadData();
    std::string err;
    if (!account_service_.registerCustomer(hoTen, sdt, matKhau, err)) {
        return {false, err};
    }
    refreshCache();
    return {true, ""};
}

KetQua Facade::dangKyChuSan(const std::string& hoTen, const std::string& sdt, const std::string& matKhau,
                            const std::string& tenCoSo, const std::string& diaChi) {
    loadData();
    std::string err;
    if (!account_service_.registerOwner(hoTen, sdt, matKhau, tenCoSo, diaChi, err)) {
        return {false, err};
    }
    refreshCache();
    return {true, ""};
}

void Facade::moManHinhChinhChoUser(User* user, ScreenManager& sm) {
    if (!user) return;
    std::string acc = user->get_account_type();
    if (dynamic_cast<Owner*>(user) || acc == "Owner" || acc == u8"Chủ sân") {
        sm.replace(std::make_unique<GUI::Screens::Owner::OwnerShellScreen>());
    } else {
        sm.replace(std::make_unique<GUI::Screens::Renter::RenterShellScreen>());
    }
}

}  // namespace GUI::App
