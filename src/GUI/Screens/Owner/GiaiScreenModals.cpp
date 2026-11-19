#include "GUI/Screens/Owner/GiaiScreen.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
#include <imgui.h>

#include "Core/DateTime.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/ComboRadio.h"
#include "GUI/Widgets/DatePicker.h"
#include "GUI/Widgets/FormValidator.h"
#include "GUI/Widgets/InputFields.h"
#include "GUI/Widgets/Modal.h"
#include "GUI/Widgets/Toast.h"
#include "GUI/App/ViewFormat.h"
#include "GiaiScreenHelpers.h"

namespace GUI::Screens::Owner {

using namespace GUI::App::ViewFormat;
using namespace detail;

void GiaiScreen::renderModalXacNhanDuyet() {
    std::string noiDung = u8"Bạn có chắc chắn muốn phê duyệt giải đấu mã " + maGiaiCanDuyet_ +
                          u8"Hệ thống sẽ kiểm tra trùng lịch sân bóng trước khi xác nhận.";

    Widgets::ModalXacNhan(
        u8"Xác nhận phê duyệt giải đấu",
        noiDung.c_str(),
        [this]() {
            // Khi duyệt gọi Facade kiểm tra trùng lịch sân bóng
            auto kq = GUI::App::Facade::getInstance().duyetGiaiDau(maGiaiCanDuyet_);
            if (kq.ok) {
                reloadViewModel();
            } else {
                // Hiện Toast đỏ nội dung lỗi do nghiệp vụ trả về
            }
        },
        u8"Phê duyệt",
        u8"Đóng",
        false
    );
}

void GiaiScreen::renderModalTuChoi() {
    Widgets::KiemTraVaMoModal(u8"Nhập lý do từ chối yêu cầu");

    if (ImGui::BeginPopupModal(u8"Nhập lý do từ chối yêu cầu", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::Text(u8"Vui lòng nhập lý do từ chối giải đấu mã %s:", maGiaiCanTuChoi_.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();

        char buf[256];
        std::snprintf(buf, sizeof(buf), "%s", lyDoTuChoi_.c_str());
        ImGui::SetNextItemWidth(360.0f);
        if (ImGui::InputText("##input_reject_reason", buf, sizeof(buf))) {
            lyDoTuChoi_ = buf;
        }

        ImGui::Spacing();
        ImGui::Spacing();

        if (Widgets::NutNguyHiem(u8"Xác nhận từ chối", ImVec2(160.0f, 32.0f))) {
            auto kq = GUI::App::Facade::getInstance().tuChoiGiaiDau(maGiaiCanTuChoi_, lyDoTuChoi_);
            if (kq.ok) {
                reloadViewModel();
            } else {
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(0.0f, 12.0f);

        if (Widgets::NutPhu(u8"Hủy bỏ", ImVec2(100.0f, 32.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void GiaiScreen::renderModalSuaGiai() {
    if (moPopupSua_) {
        ImGui::OpenPopup(u8"Chỉnh sửa thông tin giải đấu");
        moPopupSua_ = false;
    }

    if (ImGui::BeginPopupModal(u8"Chỉnh sửa thông tin giải đấu", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::Text(u8"Cập nhật thông tin cho giải đấu: %s", giaiDangSua_.maGiai.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Tên giải
        char tenBuf[128];
        std::snprintf(tenBuf, sizeof(tenBuf), "%s", giaiDangSua_.tenGiai.c_str());
        ImGui::SetNextItemWidth(380.0f);
        if (ImGui::InputText(u8"Tên giải đấu", tenBuf, sizeof(tenBuf))) {
            giaiDangSua_.tenGiai = tenBuf;
        }
        ImGui::Spacing();

        // Ngày bắt đầu / kết thúc
        Widgets::DatePicker(
            u8"Ngày bắt đầu", "##edit_dp_start",
            giaiDangSua_.ngayBatDau, nullptr, true
        );
        Widgets::DatePicker(
            u8"Ngày kết thúc", "##edit_dp_end",
            giaiDangSua_.ngayKetThuc, nullptr, true
        );
        ImGui::Spacing();

        // Sân bóng
        char sanBuf[32];
        std::snprintf(sanBuf, sizeof(sanBuf), "%s", giaiDangSua_.maSan.c_str());
        ImGui::SetNextItemWidth(180.0f);
        if (ImGui::InputText(u8"Mã sân", sanBuf, sizeof(sanBuf))) {
            giaiDangSua_.maSan = sanBuf;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (Widgets::NutChinh(u8"Lưu thay đổi", ImVec2(130.0f, 32.0f))) {
            auto kq = GUI::App::Facade::getInstance().suaGiaiDau(giaiDangSua_);
            if (kq.ok) {
                reloadViewModel();
                ImGui::CloseCurrentPopup();
            } else {
            }
        }

        ImGui::SameLine(0.0f, 12.0f);

        if (Widgets::NutPhu(u8"Đóng", ImVec2(90.0f, 32.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void GiaiScreen::renderModalHuyGiai() {
    std::string noiDung = u8"Bạn có chắc chắn muốn hủy bỏ giải đấu " + maGiaiCanHuy_ +
                          u8"Thao tác này sẽ giải phóng toàn bộ khung giờ của sân bóng.";

    Widgets::ModalXacNhan(
        u8"Xác nhận hủy giải đấu",
        noiDung.c_str(),
        [this]() {
            auto kq = GUI::App::Facade::getInstance().huyGiaiDau(maGiaiCanHuy_);
            if (kq.ok) {
                reloadViewModel();
            } else {
            }
        },
        u8"Xác nhận hủy",
        u8"Đóng",
        true // Nút đỏ nguy hiểm
    );
}

}  // namespace GUI::Screens::Owner
