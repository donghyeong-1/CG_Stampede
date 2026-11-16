// 글자 출력. Windows GDI로 문자열을 비트맵에 그린 뒤 OpenGL 텍스처(알파 1채널)로 올려 UI 사각형에 입힌다.
// 한글을 포함한 UTF-8 문자열을 쓸 수 있다. 같은 (문자열, 크기)는 캐시해서 다시 만들지 않는다.
#pragma once
#include "core/Platform.h"
#include <unordered_map>

struct TextTexture
{
    GLuint texture = 0;
    int width = 0, height = 0; // 픽셀.
};

class TextCache
{
public:
    const TextTexture &Get(const std::string &utf8, int pixelHeight);
    void Destroy();

private:
    std::unordered_map<std::string, TextTexture> entries;
};
