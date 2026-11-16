// [장수현] 텍스처. 현재는 코드로 만든 체크 무늬(바닥 자리표시자). 실제 이미지는 stb_image 등으로 교체한다.
// TODO(장수현): assets/textures/ 의 협곡 바닥 이미지 로딩으로 교체.
#pragma once
#include "core/Platform.h"

GLuint MakeCheckerTexture(int size, int cells, glm::vec3 colorA, glm::vec3 colorB);
