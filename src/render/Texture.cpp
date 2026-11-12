#include "render/Texture.h"

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
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return texture;
}
