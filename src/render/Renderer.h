// [장수현] 렌더러. 월드(Phong) 그리기와 UI(직교 투영) 그리기를 담당한다.
// 실습 23의 DrawCube(parent, position, size, color, vp)처럼 부모 행렬을 받아 계층 변환을 쌓는다.
#pragma once
#include "core/Platform.h"
#include "render/Light.h"
#include "render/Mesh.h"
#include "render/Shader.h"

class Renderer
{
public:
    bool Init();
    void Shutdown();

    // ── 월드(3D) ──
    void BeginScene(const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &eye,
                    const DirLight &light, int framebufferWidth, int framebufferHeight);
    // parent * T(position) * S(size). 크기는 전체 길이(반크기 아님).
    void DrawCube(const glm::mat4 &parent, const glm::vec3 &position, const glm::vec3 &size,
                  const glm::vec3 &color, float highlight = 0.0f, float alpha = 1.0f);
    // 바닥: x 범위 [-halfWidth, halfWidth], z 범위 [0, length]. 텍스처가 repeat 횟수만큼 반복된다.
    void DrawFloor(float halfWidth, float length, const glm::vec2 &uvRepeat);

    // ── UI(2D) ──  좌표는 프레임버퍼 픽셀, 좌상단 원점.
    void BeginUi(int framebufferWidth, int framebufferHeight);
    void DrawRect(float x, float y, float width, float height, const glm::vec4 &rgba);
    void EndUi();

private:
    Shader phong, ui;
    Mesh cube, floorPlane, quad;
    GLuint floorTexture = 0;
};
