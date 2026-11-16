#include "render/Texture.h"

namespace
{
void SetRepeatMipmap()
{
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

unsigned int ReadU32(const std::string &data, size_t offset)
{
    unsigned int value = 0;
    for (int i = 3; i >= 0; --i) { value = (value << 8) | static_cast<unsigned char>(data[offset + static_cast<size_t>(i)]); }
    return value;
}
} // namespace

GLuint LoadBmpTexture(const char *assetName)
{
    std::string data = ReadTextFile(assetName); // 바이너리 그대로 읽는다.
    if (data.size() < 54 || data[0] != 'B' || data[1] != 'M') { return 0; }
    unsigned int pixelOffset = ReadU32(data, 10);
    int width = static_cast<int>(ReadU32(data, 18));
    int height = static_cast<int>(ReadU32(data, 22));
    unsigned int bitsPerPixel = ReadU32(data, 28) & 0xFFFFu;
    unsigned int compression = ReadU32(data, 30);
    if (width <= 0 || height <= 0 || compression != 0 || (bitsPerPixel != 24 && bitsPerPixel != 32)) { return 0; }
    size_t rowSize = (static_cast<size_t>(width) * bitsPerPixel + 31) / 32 * 4; // 행은 4바이트 단위로 채워진다.
    if (data.size() < pixelOffset + rowSize * static_cast<size_t>(height)) { return 0; }

    // BMP는 아래 행부터, BGR 순서로 저장된다. OpenGL도 아래 행이 먼저이므로 그대로 올린다.
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, bitsPerPixel == 24 ? GL_BGR : GL_BGRA, GL_UNSIGNED_BYTE,
                 data.data() + pixelOffset);
    SetRepeatMipmap();
    return texture;
}

GLuint MakeCheckerTexture(int size, int cells, glm::vec3 colorA, glm::vec3 colorB)
{
    std::vector<unsigned char> pixels(static_cast<size_t>(size) * size * 3);
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            bool odd = ((x * cells / size) + (y * cells / size)) % 2 != 0;
            const glm::vec3 &c = odd ? colorA : colorB;
            size_t i = (static_cast<size_t>(y) * size + x) * 3;
            pixels[i] = static_cast<unsigned char>(c.r * 255);
            pixels[i + 1] = static_cast<unsigned char>(c.g * 255);
            pixels[i + 2] = static_cast<unsigned char>(c.b * 255);
        }
    }
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    SetRepeatMipmap();
    return texture;
}
