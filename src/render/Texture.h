// 텍스처. 바닥은 assets/textures/ground.bmp를 읽고, 파일이 없으면 코드로 만든 체크 무늬로 대신한다.
#pragma once
#include "core/Platform.h"

// 24/32비트 무압축 BMP를 읽어 반복(GL_REPEAT)·밉맵 텍스처로 만든다. 실패하면 0.
GLuint LoadBmpTexture(const char *assetName);

GLuint MakeCheckerTexture(int size, int cells, glm::vec3 colorA, glm::vec3 colorB);
