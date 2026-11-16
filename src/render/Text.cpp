#include "render/Text.h"

namespace
{
const size_t MAX_ENTRIES = 300; // 초 단위 숫자처럼 계속 바뀌는 문자열이 쌓이지 않게 한다.

TextTexture Rasterize(const std::string &utf8, int pixelHeight)
{
    TextTexture result;
    int wideLength = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), nullptr, 0);
    if (wideLength <= 0) { return result; }
    std::wstring wide(static_cast<size_t>(wideLength), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), wide.data(), wideLength);

    HDC dc = CreateCompatibleDC(nullptr);
    if (!dc) { return result; }
    HFONT font = CreateFontW(-pixelHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Malgun Gothic");
    HGDIOBJ oldFont = SelectObject(dc, font);
    SIZE size{};
    GetTextExtentPoint32W(dc, wide.c_str(), wideLength, &size);
    int width = std::max(static_cast<int>(size.cx) + 2, 1);
    int height = std::max(static_cast<int>(size.cy), 1);

    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height; // 음수 = 위에서 아래로(첫 행이 글자 윗부분).
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    void *bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (bitmap && bits)
    {
        HGDIOBJ oldBitmap = SelectObject(dc, bitmap);
        std::fill_n(static_cast<unsigned char *>(bits), static_cast<size_t>(width) * height * 4, static_cast<unsigned char>(0));
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(255, 255, 255));
        TextOutW(dc, 1, 0, wide.c_str(), wideLength);
        GdiFlush();

        // 검은 바탕에 흰 글자를 그렸으므로 밝기(초록 채널)가 곧 글자의 불투명도다.
        const unsigned char *pixels = static_cast<const unsigned char *>(bits);
        std::vector<unsigned char> alpha(static_cast<size_t>(width) * height);
        for (size_t i = 0; i < alpha.size(); ++i) { alpha[i] = pixels[i * 4 + 1]; }

        glGenTextures(1, &result.texture);
        glBindTexture(GL_TEXTURE_2D, result.texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, alpha.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        result.width = width;
        result.height = height;
        SelectObject(dc, oldBitmap);
    }
    if (bitmap) { DeleteObject(bitmap); }
    SelectObject(dc, oldFont);
    DeleteObject(font);
    DeleteDC(dc);
    return result;
}
} // namespace

const TextTexture &TextCache::Get(const std::string &utf8, int pixelHeight)
{
    std::string key = std::to_string(pixelHeight) + ":" + utf8;
    auto found = entries.find(key);
    if (found != entries.end()) { return found->second; }
    if (entries.size() >= MAX_ENTRIES) { Destroy(); }
    return entries.emplace(key, Rasterize(utf8, std::max(pixelHeight, 4))).first->second;
}

void TextCache::Destroy()
{
    for (auto &entry : entries)
    {
        if (entry.second.texture) { glDeleteTextures(1, &entry.second.texture); }
    }
    entries.clear();
}
