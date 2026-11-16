// [장수현] 방향광. 진행도(0~1)에 따라 색·세기가 바뀐다.
// TODO(장수현): 시작/끝 색상과 Phong 계수 확정.
#pragma once
#include "core/Platform.h"

struct DirLight
{
    glm::vec3 directionToLight{-0.4f, 1.0f, -0.3f}; // 표면에서 광원 쪽 방향.
    glm::vec3 color{1.0f};                          // 세기가 곱해진 색.
    float ambient = 0.3f;
};

inline DirLight LightForProgress(float progress)
{
    const glm::vec3 startColor(1.0f, 0.95f, 0.80f); // 시작: 따뜻한 낮빛.
    const glm::vec3 endColor(1.0f, 0.45f, 0.25f);   // 끝: 산불의 붉은빛.
    float t = std::clamp(progress, 0.0f, 1.0f);
    DirLight light;
    light.color = glm::mix(startColor, endColor, t) * glm::mix(1.0f, 0.85f, t);
    light.ambient = glm::mix(0.32f, 0.22f, t);
    return light;
}
