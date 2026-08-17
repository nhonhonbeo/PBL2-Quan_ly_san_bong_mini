#ifndef GIAI_H
#define GIAI_H
#include <string>

class Giai {
private:
    std::string tenGiai;
    std::string maKhachDatSuKien;
    std::string lichDienRa;

public:
    Giai(const std::string& ten, const std::string& maKhach, const std::string& lich);

    // TODO: chi can luu tru + hien thi, khong can logic phuc tap
};

#endif
