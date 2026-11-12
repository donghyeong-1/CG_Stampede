// [김동형] 쿼터뷰 카메라와 화면 좌표 → 월드 레이 변환.
// 뷰 변환 + 원근 투영. 탑승 동물을 추적하고 사선 오프셋으로 진행 방향을 비스듬히 보여 준다.
// TODO(김동형): 오프셋·FOV·보간 계수 확정, 점프 중 추적 방식.
#pragma once
#include "core/Platform.h"

struct Ray
{
    glm::vec3 origin{0};
    glm::vec3 direction{0, 0, 1}; // 정규화.
};

class Camera
{
public:
    void Reset(const glm::vec3 &target);
    void Follow(const glm::vec3 &target, float dt); // 지수 보간으로 부드럽게 추적.
    glm::vec3 Eye() const { return target + offset; }
    glm::mat4 View() const;
    glm::mat4 Projection(int framebufferWidth, int framebufferHeight) const;
    // 프레임버퍼 픽셀(좌상단 원점) → 월드 레이. 창 크기가 바뀌어도 해당 프레임버퍼 크기를 그대로 사용한다.
    Ray ScreenRay(float mouseX, float mouseY, int framebufferWidth, int framebufferHeight) const;

private:
    glm::vec3 target{0};
    glm::vec3 offset{4.0f, 8.0f, -9.0f}; // 진행 방향(+Z) 뒤쪽·위쪽·옆.
    glm::vec3 lookAhead{0.0f, 0.0f, 3.0f}; // 시선은 진행 방향 앞쪽을 향한다.
};
