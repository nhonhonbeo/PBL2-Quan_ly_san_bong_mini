#pragma once

#include <string>
#include <vector>
#include "GUI/App/Facade.h"
#include "GUI/App/Screen.h"
#include "GUI/Widgets/Table.h"

namespace GUI::Screens::Renter {

// LÝ DO CHỌN: TaoGiaiScreen cung cấp biểu mẫu gửi yêu cầu tổ chức giải đấu phong trào tới Chủ sân
// và bảng theo dõi trạng thái phê duyệt (Chờ duyệt / Đã duyệt / Bị từ chối kèm lý do).
class TaoGiaiScreen : public GUI::App::Screen {
public:
    TaoGiaiScreen() = default;
    ~TaoGiaiScreen() override = default;

    void onEnter() override;
    void render() override;
    void onExit() override;

    void reloadViewModel();

private:
    void renderFormTaoGiai();
    void renderBangTheoDoiTrangThai();

    // Dữ liệu Form gửi yêu cầu
    std::string formTenGiai_;
    std::string formNgayBatDau_;
    std::string formNgayKetThuc_;
    int formSoDoi_ = 8;
    int formLoaiSanIndex_ = 0; // 0: Sân 5, 1: Sân 7, 2: Sân 11
    std::string formGhiChu_;
    bool formTenGiaiDaCham_ = false;
    bool daGuiYeuCau_ = false;

    // Dữ liệu Bảng theo dõi
    std::vector<GUI::App::GiaiDauInfo> danhSachGiaiCuaToi_;
    std::vector<Widgets::TableColumn> tableCols_;
    std::vector<Widgets::TableRow> tableRows_;
    Widgets::TableState tableState_;
};

}  // namespace GUI::Screens::Renter
