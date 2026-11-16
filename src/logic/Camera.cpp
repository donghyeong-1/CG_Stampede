#include "logic/Camera.h"
#include "core/Config.h"

void Camera::Reset(const glm::vec3 &t)
{
    target = t;
}

void Camera::Follow(const glm::vec3 &t, float dt)
{
    float blend = 1.0f - std::exp(-cfg::CAMERA_FOLLOW_SPEED * dt); // 프레임 속도와 무관한 보간.
    target += (t - target) * blend;
}

glm::mat4 Camera::View() const
{
    return glm::lookAt(Eye(), target + lookAhead, glm::vec3(0, 1, 0));
}

glm::mat4 Camera::Projection(int framebufferWidth, int framebufferHeight) const
{
    float aspect = static_cast<float>(std::max(framebufferWidth, 1)) / std::max(framebufferHeight, 1);
    return glm::perspective(glm::radians(cfg::CAMERA_FOV_DEG), aspect, 0.1f, 200.0f);
}

Ray Camera::ScreenRay(float mouseX, float mouseY, int framebufferWidth, int framebufferHeight) const
{
    // OpenGL 창 좌표는 좌하단 원점이므로 y를 뒤집는다. z=0은 근평면, z=1은 원평면.
    glm::vec4 viewport(0, 0, framebufferWidth, framebufferHeight);
    glm::vec3 window(mouseX, static_cast<float>(framebufferHeight) - mouseY, 0.0f);
    glm::mat4 view = View();
    glm::mat4 projection = Projection(framebufferWidth, framebufferHeight);
    glm::vec3 nearPoint = glm::unProject(window, view, projection, viewport);
    window.z = 1.0f;
    glm::vec3 farPoint = glm::unProject(window, view, projection, viewport);
    return {nearPoint, glm::normalize(farPoint - nearPoint)};
}
