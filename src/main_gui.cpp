#include "GUI/App/Application.h"

// LÝ DO CHỌN: Điểm vào (entry point) của phiên bản GUI được giữ cực kỳ tinh gọn,
// chỉ khởi tạo và ủy quyền toàn bộ vòng đời ứng dụng cho class Application.
int main() {
    Application application;
    if (!application.init()) {
        return 1;
    }

    application.run();
    return 0;
}
