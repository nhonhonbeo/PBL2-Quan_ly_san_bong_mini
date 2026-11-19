#include "GUI/App/Application.h"

// LÝ DO CHỌN: Điểm vào GUI ủy quyền toàn bộ vòng đời cho Application.
int main() {
    Application application;
    if (!application.init()) {
        return 1;
    }

    application.run();
    return 0;
}
