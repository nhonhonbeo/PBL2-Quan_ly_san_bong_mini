#include "GUI/App/Application.h"

int main() {
    Application application;
    if (!application.init()) {
        return 1;
    }

    application.run();
    return 0;
}
