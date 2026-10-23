#include "GUI/Theme/Fonts.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <imgui.h>

namespace {

const char* findFontFile() {
    static const char* candidates[] = {
        "assets/fonts/Roboto-Regular.ttf",
        "assets/fonts/NotoSans-Regular.ttf",
        "assets/fonts/BeVietnamPro-Regular.ttf"
    };

    for (const char* path : candidates) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }

    static const char* parentCandidates[] = {
        "../assets/fonts/Roboto-Regular.ttf",
        "../assets/fonts/NotoSans-Regular.ttf",
        "../assets/fonts/BeVietnamPro-Regular.ttf"
    };
    for (const char* path : parentCandidates) {
        if (std::filesystem::exists(path)) return path;
    }

    const std::filesystem::path fontDirectory{"assets/fonts"};
    if (std::filesystem::is_directory(fontDirectory)) {
        for (const std::filesystem::directory_entry& entry :
             std::filesystem::directory_iterator(fontDirectory)) {
            const std::filesystem::path extension = entry.path().extension();
            if (entry.is_regular_file() && (extension == ".ttf" || extension == ".otf")) {
                static const std::string discoveredPath = entry.path().string();
                return discoveredPath.c_str();
            }
        }
    }
    const std::filesystem::path parentFontDirectory{"../assets/fonts"};
    if (std::filesystem::is_directory(parentFontDirectory)) {
        for (const std::filesystem::directory_entry& entry :
             std::filesystem::directory_iterator(parentFontDirectory)) {
            const std::filesystem::path extension = entry.path().extension();
            if (entry.is_regular_file() && (extension == ".ttf" || extension == ".otf")) {
                static const std::string discoveredPath = entry.path().string();
                return discoveredPath.c_str();
            }
        }
    }
    return nullptr;
}

}  // namespace

namespace GUI::Theme {

Fonts loadFonts() {
    ImGuiIO& io = ImGui::GetIO();
    const char* fontPath = findFontFile();

    // LÝ DO CHỌN: Fallback về font mặc định của ImGui nếu không tìm thấy font trong assets/fonts/
    // để đảm bảo giao diện luôn mở được bình thường, không gây sập (crash) ứng dụng.
    if (fontPath == nullptr) {
        std::cerr << "Không tìm thấy font .ttf/.otf trong assets/fonts/. "
                     "Sử dụng font mặc định; tiếng Việt có thể không hiển thị đầy đủ.\n";
        ImFont* fallback = io.Fonts->AddFontDefault();
        return {fallback, fallback, fallback};
    }

    // LÝ DO CHỌN: io.Fonts->GetGlyphRangesVietnamese() cung cấp dải Unicode đầy đủ
    // bao gồm toàn bộ ký tự Latinh mở rộng và dấu thanh tiếng Việt (Ắ, Ằ, Ể, Ố, Ự...).
    // LÝ DO CHỌN: Bổ sung glyph toán học cần thiết mà không nạp dải ký tự thừa.
    ImFontGlyphRangesBuilder glyphBuilder;
    glyphBuilder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
    glyphBuilder.AddChar(0x2265);
    ImVector<ImWchar> glyphRanges;
    glyphBuilder.BuildRanges(&glyphRanges);
    Fonts fonts;
    fonts.regular = io.Fonts->AddFontFromFileTTF(fontPath, 18.0F, nullptr, glyphRanges.Data);
    fonts.heading = io.Fonts->AddFontFromFileTTF(fontPath, 28.0F, nullptr, glyphRanges.Data);
    fonts.small = io.Fonts->AddFontFromFileTTF(fontPath, 14.0F, nullptr, glyphRanges.Data);

    if (fonts.regular == nullptr || fonts.heading == nullptr || fonts.small == nullptr) {
        std::cerr << "Không thể nạp font '" << fontPath
                  << "'. Sử dụng font mặc định; tiếng Việt có thể không hiển thị đầy đủ.\n";
        io.Fonts->Clear();
        ImFont* fallback = io.Fonts->AddFontDefault();
        return {fallback, fallback, fallback};
    }

    return fonts;
}

}  // namespace GUI::Theme
