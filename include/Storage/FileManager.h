#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include <string>
#include <vector>

// Lop tap trung xu ly doc/ghi file cho toan he thong
// Moi module (Account, Field, Booking...) se goi lop nay de luu/tai du lieu
class FileManager {
public:
    static std::vector<std::string> docFile(const std::string& duongDan);
    static bool ghiFile(const std::string& duongDan, const std::vector<std::string>& dongDuLieu);
    static bool themDong(const std::string& duongDan, const std::string& dong);

    // TODO: em dinh dang du lieu (vd: phan cach bang dau '|')
    // va viet ham parse rieng cho tung loai entity (San, DatSan, KhachHang...)
};

#endif
