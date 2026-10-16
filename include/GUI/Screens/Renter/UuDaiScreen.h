#pragma once

#include <string>
#include <vector>
#include "GUI/App/Facade.h"
#include "GUI/App/Screen.h"

namespace GUI::Screens::Renter {

// LÝ DO CHỌN: UuDaiScreen hiển thị minh bạch toàn bộ quyền lợi giảm giá thành viên,
// số lần đặt đã tích lũy và thanh tiến trình ProgressBar trực quan tới mốc nâng hạng VIP tiếp theo.
class UuDaiScreen : public GUI::App::Screen {
public:
    UuDaiScreen() = default;
    ~UuDaiScreen() override = default;

    void onEnter() override;
    void render() override;
    void onExit() override;

    void reloadViewModel();

private:
    void renderThongKeUuDai();
    void renderBangCacMoc();

    int soLanDat_ = 0;
    int mucUuDaiHienTai_ = 0;
    std::string tenHangHienTai_;

    int mocKeTiep_ = 0;
    int phanTramKeTiep_ = 0;
    std::string tenHangKeTiep_;
    float tienDoKeTiep_ = 0.0f;
    int soLuotConLai_ = 0;

    std::vector<GUI::App::NguongUuDai> danhSachNguong_;
};

}  // namespace GUI::Screens::Renter
