#include <string>
#include "GUI/App/Application.h"

// LÝ DO CHỌN: Điểm vào (entry point) của phiên bản GUI phân tích tham số dòng lệnh
// để kích hoạt cờ --demo theo đúng yêu cầu đề bài, sau đó ủy quyền toàn bộ vòng đời cho Application.
int main(int argc, char* argv[]) {
    bool runDemo = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--demo") {
            runDemo = true;
            break;
        }
    }

    Application application(runDemo);
    if (!application.init()) {
        return 1;
    }

    application.run();
    return 0;
}
