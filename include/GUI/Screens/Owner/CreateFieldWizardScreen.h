#pragma once

#include <functional>
#include <string>
#include <vector>
#include "GUI/App/Screen.h"

namespace GUI::Screens::Owner {

// LÝ DO CHỌN: CreateFieldWizardScreen hướng dẫn Chủ sân tạo sân bóng theo quy trình 4 bước tuần tự (Wizard),
// giảm tải nhận thức và hạn chế tối đa sai sót khi thiết lập thông số sân và quy định cơ sở.
class CreateFieldWizardScreen : public GUI::App::Screen {
public:
    explicit CreateFieldWizardScreen(std::function<void()> onHoanTat = nullptr);
    ~CreateFieldWizardScreen() override = default;

    void onEnter() override;
    void render() override;
    void onExit() override;

private:
    void renderProgressBar();
    void renderStep1LoaiSan();
    void renderStep2SoLuong();
    void renderStep3QuyDinh();
    void renderStep4XacNhan();
    void renderNavigationButtons();
    void renderResultModal();

    // ==========================================
    // DỮ LIỆU VIEWMODEL
    // ==========================================
    int currentStep_ = 1; // 1, 2, 3, 4

    // Bước 1: Loại sân (5, 7, 11)
    int loaiSan_ = 5;

    // Bước 2: Số lượng sân cần tạo
    int soLuong_ = 2;

    // Bước 3: Quy định sử dụng sân
    std::string quyDinh_;

    // Bước 4: Thông tin chủ sân (điền sẵn từ Session)
    std::string tenChuSan_;
    std::string sdtChuSan_;
    std::string tenCoSo_;

    // Kết quả sau khi tạo
    bool moPopupKetQua_ = false;
    std::vector<std::string> danhSachMaSanVuaTao_;

    std::function<void()> onHoanTatCallback_;
};

}  // namespace GUI::Screens::Owner
