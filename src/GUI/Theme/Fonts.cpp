#include "GUI/Theme/Fonts.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <imgui.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

namespace fs = std::filesystem;

fs::path executableDirectory() {
    std::error_code ec;
#ifdef _WIN32
    wchar_t buffer[MAX_PATH] = {0};
    const DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (length > 0 && length < MAX_PATH) {
        return fs::path(buffer).parent_path();
    }
#else
    const fs::path exePath = fs::read_symlink("/proc/self/exe", ec);
    if (!ec && !exePath.empty()) {
        return exePath.parent_path();
    }
#endif
    return fs::current_path(ec);
}

bool isFontFile(const fs::path& path) {
    std::error_code ec;
    if (!fs::is_regular_file(path, ec)) {
        return false;
    }
    std::string extension = path.extension().string();
    for (char& c : extension) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return extension == ".ttf" || extension == ".otf";
}

// LÝ DO CHỌN: Ưu tiên font hệ thống Segoe UI theo tiêu chuẩn Windows Desktop App,
// sau đó đến các font đính kèm trong assets/fonts (Roboto, Arial).
void collectSystemFonts(std::vector<fs::path>& out) {
    static const char* systemFonts[] = {
#ifdef _WIN32
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
        "C:\\Windows\\Fonts\\tahoma.ttf",
        "C:\\Windows\\Fonts\\times.ttf"
#else
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/Library/Fonts/Arial.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf"
#endif
    };
    for (const char* path : systemFonts) {
        if (isFontFile(fs::path(path))) out.emplace_back(path);
    }
}

void collectFontsFrom(fs::path start, std::vector<fs::path>& out) {
    static const char* preferred[] = {
        "Roboto-Regular.ttf", "Arial.ttf", "NotoSans-Regular.ttf", "BeVietnamPro-Regular.ttf"
    };

    std::error_code ec;
    for (int depth = 0; depth < 6 && !start.empty(); ++depth) {
        const fs::path fontDirectory = start / "assets" / "fonts";
        if (fs::is_directory(fontDirectory, ec)) {
            for (const char* name : preferred) {
                const fs::path candidate = fontDirectory / name;
                if (isFontFile(candidate)) out.push_back(candidate);
            }
            for (const fs::directory_entry& entry : fs::directory_iterator(fontDirectory, ec)) {
                if (isFontFile(entry.path())) out.push_back(entry.path());
            }
        }
        const fs::path parent = start.parent_path();
        if (parent == start) break;
        start = parent;
    }
}

std::uint32_t be32(const std::vector<unsigned char>& d, std::size_t p) {
    return (std::uint32_t(d[p]) << 24) | (std::uint32_t(d[p + 1]) << 16) |
           (std::uint32_t(d[p + 2]) << 8) | std::uint32_t(d[p + 3]);
}
std::uint32_t be16(const std::vector<unsigned char>& d, std::size_t p) {
    return (std::uint32_t(d[p]) << 8) | std::uint32_t(d[p + 1]);
}

bool validateFont(const std::vector<unsigned char>& d, std::string& why) {
    if (d.size() < 28) { why = "file quá nhỏ"; return false; }
    const std::uint32_t tag = be32(d, 0);
    if (tag != 0x00010000u && tag != 0x74727565u && tag != 0x4F54544Fu) {
        why = "không phải file TrueType/OpenType (sai chữ ký đầu file)";
        return false;
    }
    const std::size_t numTables = be16(d, 4);
    if (numTables == 0 || 12 + 16 * numTables > d.size()) { why = "bảng thư mục font bị hỏng"; return false; }

    bool hasHead = false, hasCmap = false;
    for (std::size_t i = 0; i < numTables; ++i) {
        const std::size_t rec = 12 + i * 16;
        const std::uint32_t tTag = be32(d, rec);
        const std::uint32_t off  = be32(d, rec + 8);
        const std::uint32_t len  = be32(d, rec + 12);
        if (off + len > d.size()) { why = "bảng sfnt vượt kích thước"; return false; }
        if (tTag == 0x68656164u) hasHead = true;
        if (tTag == 0x636D6170u) hasCmap = true;
    }
    if (!hasHead || !hasCmap) { why = "thiếu bảng bắt buộc (head hoặc cmap)"; return false; }
    return true;
}

bool readFile(const fs::path& path, std::vector<unsigned char>& data) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    in.seekg(0, std::ios::end);
    const std::streamoff size = in.tellg();
    if (size <= 0) return false;
    in.seekg(0, std::ios::beg);
    data.resize(static_cast<std::size_t>(size));
    return static_cast<bool>(in.read(reinterpret_cast<char*>(data.data()), size));
}

void buildGlyphRanges(ImGuiIO& io, ImVector<ImWchar>& out) {
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());

    static const ImWchar extraRanges[] = {
        0x2013, 0x2014,  // – —
        0x2018, 0x201F,  // ‘ ’ “ ”
        0x2022, 0x2022,  // •
        0x2026, 0x2026,  // …
        0x20AB, 0x20AB,  // ₫
        0x2190, 0x2193,  // ← ↑ → ↓
        0x2264, 0x2265,  // ≤ ≥
        0x2600, 0x27BF,  // ★ ☆ ⚽ ✓ ✕
        0
    };
    builder.AddRanges(extraRanges);
    builder.BuildRanges(&out);
}

GUI::Theme::Fonts s_CurrentFonts;

}  // namespace

namespace GUI::Theme {

Fonts loadFonts() {
    ImGuiIO& io = ImGui::GetIO();

    // Tìm font hợp lệ: Ưu tiên font hệ thống Segoe UI trên Windows, sau đó assets/fonts
    std::vector<fs::path> candidates;
    std::error_code ec;
    collectSystemFonts(candidates);
    collectFontsFrom(fs::current_path(ec), candidates);
    collectFontsFrom(executableDirectory(), candidates);

    static std::vector<unsigned char> fontData;
    std::string usedPath;
    for (const fs::path& candidate : candidates) {
        std::vector<unsigned char> data;
        std::string why;
        if (!readFile(candidate, data)) {
            continue;
        }
        if (!validateFont(data, why)) {
            continue;
        }
        fontData = std::move(data);
        usedPath = candidate.string();
        break;
    }

    if (usedPath.empty()) {
        std::cerr << "[Font] Không có font hợp lệ. Dùng font mặc định.\n";
        ImFont* fallback = io.Fonts->AddFontDefault();
        s_CurrentFonts = {fallback, fallback, fallback, fallback};
        return s_CurrentFonts;
    }

    static ImVector<ImWchar> glyphRanges;
    glyphRanges.clear();
    buildGlyphRanges(io, glyphRanges);

    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    config.OversampleH = 2;
    config.OversampleV = 2;
    config.PixelSnapH = true;

    const int dataSize = static_cast<int>(fontData.size());
    Fonts fonts;
    // LÝ DO CHỌN: Phân cấp Typography chuẩn mực:
    // - regular: 15px (nội dung, form, bảng)
    // - heading: 24px (tiêu đề màn hình)
    // - title:   18px (tiêu đề khu vực, card header)
    // - small:   13px (nhãn phụ, badge, footnote)
    fonts.regular = io.Fonts->AddFontFromMemoryTTF(fontData.data(), dataSize, 15.0F, &config, glyphRanges.Data);
    fonts.heading = io.Fonts->AddFontFromMemoryTTF(fontData.data(), dataSize, 24.0F, &config, glyphRanges.Data);
    fonts.title   = io.Fonts->AddFontFromMemoryTTF(fontData.data(), dataSize, 18.0F, &config, glyphRanges.Data);
    fonts.small   = io.Fonts->AddFontFromMemoryTTF(fontData.data(), dataSize, 13.0F, &config, glyphRanges.Data);

    if (fonts.regular == nullptr || fonts.heading == nullptr || fonts.title == nullptr || fonts.small == nullptr) {
        io.Fonts->Clear();
        ImFont* fallback = io.Fonts->AddFontDefault();
        s_CurrentFonts = {fallback, fallback, fallback, fallback};
        return s_CurrentFonts;
    }

    io.FontDefault = fonts.regular;

    if (!io.Fonts->Build()) {
        std::cerr << "[Font] Không thể dựng atlas font.\n";
    }

    s_CurrentFonts = fonts;
    return fonts;
}

const Fonts& getFonts() {
    return s_CurrentFonts;
}

}  // namespace GUI::Theme
