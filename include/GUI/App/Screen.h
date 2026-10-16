#pragma once

namespace GUI::App {

// LÝ DO CHỌN: đa hình để ScreenManager điều khiển mọi màn hình qua một giao diện chung.
// Định nghĩa vòng đời chuẩn: onEnter() nạp dữ liệu ViewModel, render() vẽ giao diện, onExit() dọn dẹp.
class Screen {
public:
    virtual ~Screen() = default;

    // Gọi một lần khi màn hình bắt đầu được hiển thị (nạp dữ liệu vào ViewModel)
    virtual void onEnter() {}

    // Gọi mỗi frame để render giao diện đồ họa
    virtual void render() = 0;

    // Gọi khi màn hình bị đóng hoặc chuyển sang màn hình khác
    virtual void onExit() {}
};

}  // namespace GUI::App
