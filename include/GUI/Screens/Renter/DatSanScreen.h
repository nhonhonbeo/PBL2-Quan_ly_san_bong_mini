#pragma once

#include <functional>
#include <map>
#include <string>

#include "Core/LinkedList.h"
#include "GUI/App/Screen.h"
#include "Repository/VenueRepository.h"
#include "Service/VenueService.h"

namespace GUI::Screens::Renter {

// LÝ DO CHỌN: ViewModel dùng LinkedList theo cấu trúc nghiệp vụ, còn dữ liệu
// được nạp theo sự kiện để render không đọc file trong mỗi frame.
class DatSanScreen : public GUI::App::Screen {
public:
    explicit DatSanScreen(std::function<void()> onXemLichCuaToi = nullptr);
    ~DatSanScreen() override = default;

    void onEnter() override;
    void render() override;
    void onExit() override;
    void reloadViewModel();

private:
    void renderThanhTienTrinh();
    void renderCanhBaoDanhSachDen();
    void renderBuoc1ChonCoSo();
    void renderBuoc2ChonSan();
    void renderBuoc3ChonNgayVaKhungGio();
    void renderBuoc4XacNhan();
    void renderPhieuDatSan();
    void reloadSlots();
    void chonCoSo(const VenueFacilityRecord& facility);
    void capNhatTongTien();
    bool slotDangChon(const std::string& startTime) const;

    std::function<void()> onXemLichCuaToi_;
    int buocHienTai_ = 1;
    bool biChanDatSan_ = false;
    bool choPhep60Phut_ = true;
    bool choPhep90Phut_ = true;
    int loaiSanChon_ = 5;
    int thoiLuongChon_ = 60;
    std::string tuKhoaCoSo_;
    std::string ownerIdChon_;
    std::string tenCoSoChon_;
    std::string diaChiCoSoChon_;
    std::string ngayChon_;
    std::string maSanChon_;
    std::string tenSanChon_;
    std::string khungGioChon_;
    std::string loiChonCa_;
    long long giaGoc_ = 0;
    int phanTramGiam_ = 0;
    long long soTienGiam_ = 0;
    long long tongTien_ = 0;
    std::string maDonDaDat_;
    std::string loiTaiTaiDuLieu_;
    LinkedList<VenueFacilityRecord> danhSachCoSo_;
    LinkedList<VenueFieldRecord> danhSachSan_;
    LinkedList<VenueSlotRecord> danhSachKhungGioTrong_;
    LinkedList<VenueSlotRecord> cacCaDangChon_;
    std::map<std::string, int> soSanTheoCoSo_;
    std::map<std::string, long long> giaThapNhatTheoCoSo_;
    std::map<int, bool> loaiSanCoSan_;
};

}  // namespace GUI::Screens::Renter
