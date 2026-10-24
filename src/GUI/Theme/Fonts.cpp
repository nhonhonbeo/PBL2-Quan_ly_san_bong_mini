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

// LÝ DO CHỌN: Thư mục chứa file thực thi không phụ thuộc vào "thư mục làm việc hiện tại",
// nên chạy .exe bằng double-click / từ IDE / từ thư mục khác vẫn tìm được assets/fonts.
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

// Thu thập các file font ứng viên trong assets/fonts, đi ngược lên tối đa 5 cấp thư mục cha.
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

// Font hệ thống làm phương án dự phòng (đều hỗ trợ tiếng Việt).
void collectSystemFonts(std::vector<fs::path>& out) {
    static const char* systemFonts[] = {
#ifdef _WIN32
        "C:\\Windows\\Fonts\\arial.ttf",
        "C:\\Windows\\Fonts\\segoeui.ttf",
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

std::uint32_t be32(const std::vector<unsigned char>& d, std::size_t p) {
    return (std::uint32_t(d[p]) << 24) | (std::uint32_t(d[p + 1]) << 16) |
           (std::uint32_t(d[p + 2]) << 8) | std::uint32_t(d[p + 3]);
}
std::uint32_t be16(const std::vector<unsigned char>& d, std::size_t p) {
    return (std::uint32_t(d[p]) << 8) | std::uint32_t(d[p + 1]);
}

// LÝ DO CHỌN: Một file .ttf bị hỏng (cắt cụt, bị Git đổi CRLF, là file LFS pointer, tải lỗi...)
// khiến ImGui đọc ra dữ liệu rác: hiện chữ sai, assertion hoặc crash. Kiểm tra cấu trúc
// sfnt trước khi nạp để loại file hỏng và chuyển sang font khác thay vì sập ứng dụng.
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
        const std::size_t rec = 12 + 16 * i;
        const std::uint64_t offset = be32(d, rec + 8);
        const std::uint64_t length = be32(d, rec + 12);
        if (offset + length > d.size()) { why = "file bị cắt cụt hoặc hỏng (bảng nằm ngoài file)"; return false; }
        const std::uint32_t name = be32(d, rec);
        if (name == 0x68656164u) {  // 'head'
            if (length < 54 || be32(d, static_cast<std::size_t>(offset) + 12) != 0x5F0F3CF5u) {
                why = "bảng 'head' sai magic (file bị biến đổi byte)";
                return false;
            }
            hasHead = true;
        } else if (name == 0x636D6170u) {  // 'cmap'
            hasCmap = true;
        }
    }
    if (!hasHead || !hasCmap) { why = "thiếu bảng 'head' hoặc 'cmap'"; return false; }
    return true;
}

bool readFile(const fs::path& path, std::vector<unsigned char>& data) {
    std::ifstream in(path, std::ios::binary);  // overload fs::path: an toàn với đường dẫn Unicode trên Windows
    if (!in) return false;
    in.seekg(0, std::ios::end);
    const std::streamoff size = in.tellg();
    if (size <= 0) return false;
    in.seekg(0, std::ios::beg);
    data.resize(static_cast<std::size_t>(size));
    return static_cast<bool>(in.read(reinterpret_cast<char*>(data.data()), size));
}

// Dải glyph: toàn bộ Latin + tiếng Việt, cộng thêm các ký hiệu mà giao diện đang dùng.
// Nếu thiếu glyph trong dải này, ImGui sẽ vẽ ký tự đó thành dấu '?'.
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
        0
    };
    builder.AddRanges(extraRanges);
    builder.BuildRanges(&out);
}

}  // namespace

namespace GUI::Theme {

Fonts loadFonts() {
    ImGuiIO& io = ImGui::GetIO();

    // Tìm font hợp lệ đầu tiên: ưu tiên assets/fonts của dự án, sau đó tới font hệ thống.
    std::vector<fs::path> candidates;
    std::error_code ec;
    collectFontsFrom(fs::current_path(ec), candidates);
    collectFontsFrom(executableDirectory(), candidates);
    collectSystemFonts(candidates);

    // LÝ DO CHỌN: Dữ liệu font phải static vì ImGui giữ con trỏ tới nó (FontDataOwnedByAtlas = false).
    static std::vector<unsigned char> fontData;
    std::string usedPath;
    for (const fs::path& candidate : candidates) {
        std::vector<unsigned char> data;
        std::string why;
        if (!readFile(candidate, data)) {
            std::cerr << "[Font] Không đọc được: " << candidate.string() << "\n";
            continue;
        }
        if (!validateFont(data, why)) {
            std::cerr << "[Font] Bỏ qua font hỏng '" << candidate.string() << "': " << why << "\n";
            continue;
        }
        fontData = std::move(data);
        usedPath = candidate.string();
        break;
    }

    // Fallback về font mặc định của ImGui để giao diện luôn mở được, không gây sập ứng dụng.
    if (usedPath.empty()) {
        std::cerr << "[Font] Không có font hợp lệ nào. Dùng font mặc định; tiếng Việt sẽ không hiển thị đúng.\n";
        ImFont* fallback = io.Fonts->AddFontDefault();
        return {fallback, fallback, fallback};
    }
    std::cerr << "[Font] Đang dùng: " << usedPath << " (" << fontData.size() << " byte)\n";

    // LÝ DO CHỌN: ImFontConfig::GlyphRanges chỉ lưu CON TRỎ; mảng phải sống suốt vòng đời font
    // nên khai báo static (biến cục bộ sẽ bị hủy -> ImGui đọc bộ nhớ rác).
    static ImVector<ImWchar> glyphRanges;
    glyphRanges.clear();
    buildGlyphRanges(io, glyphRanges);

    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;  // dữ liệu do fontData (static) quản lý
    config.OversampleH = 2;
    config.OversampleV = 2;
    config.PixelSnapH = true;

    const int dataSize = static_cast<int>(fontData.size());
    Fonts fonts;
    fonts.regular = io.Fonts->AddFontFromMemoryTTF(fontData.data(), dataSize, 18.0F, &config, glyphRanges.Data);
    fonts.heading = io.Fonts->AddFontFromMemoryTTF(fontData.data(), dataSize, 28.0F, &config, glyphRanges.Data);
    fonts.small = io.Fonts->AddFontFromMemoryTTF(fontData.data(), dataSize, 14.0F, &config, glyphRanges.Data);

    if (fonts.regular == nullptr || fonts.heading == nullptr || fonts.small == nullptr) {
        std::cerr << "[Font] Không thể nạp font '" << usedPath << "'. Dùng font mặc định.\n";
        io.Fonts->Clear();
        ImFont* fallback = io.Fonts->AddFontDefault();
        return {fallback, fallback, fallback};
    }
    io.FontDefault = fonts.regular;

    // Dựng atlas ngay (khi mọi dữ liệu còn hợp lệ) thay vì để ImGui dựng trễ ở khung hình đầu tiên.
    if (!io.Fonts->Build()) {
        std::cerr << "[Font] Không thể dựng atlas font.\n";
    }
    return fonts;
}

}  // namespace GUI::Theme