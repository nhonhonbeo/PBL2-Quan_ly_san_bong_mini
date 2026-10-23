#pragma once

#include <string>
#include <vector>
#include "GUI/App/Facade.h"
#include "GUI/App/Screen.h"

namespace GUI::Screens::Renter {

// LÝ DO CHỌN: DanhGiaScreen chỉ cho phép người thuê đánh giá những sân bóng đã thuê xong,
// sử dụng component StarRating tương tác trực tiếp và hỗ trợ xem lại/chỉnh sửa đánh giá cũ.
class DanhGiaScreen : public GUI::App::Screen {
public:
    DanhGiaScreen() = default;
    ~DanhGiaScreen() override = default;

    void onEnter() override;
    void render() override;
    void onExit() override;

    void reloadViewModel();

private:
    void renderFormDanhGia();
    void renderDanhSachDanhGiaCuaToi();

    // Biểu mẫu đánh giá
    int formIndexSan_ = 0;
    int formSoSao_ = 5;
    std::string formNoiDung_;
    std::string formDanhGiaIdDangSua_; // Rỗng: Tạo mới, Khác rỗng: Đang sửa

    // ViewModel
    std::vector<GUI::App::SanBongInfo> danhSachSan_;
    std::vector<GUI::App::SanBongInfo> danhSachSanDaThue_;
    std::vector<GUI::App::DanhGiaInfo> danhSachDanhGiaCuaToi_;
};

}  // namespace GUI::Screens::Renter
