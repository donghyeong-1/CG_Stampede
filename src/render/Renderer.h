// [장수현] 렌더러. 월드(Phong) 그리기와 UI(직교 투영) 그리기를 담당한다.
// 실습 23의 DrawCube(parent, position, size, color, vp)처럼 부모 행렬을 받아 계층 변환을 쌓는다.
#pragma once
#include "core/Platform.h"
#include "render/Light.h"
#include "render/Mesh.h"
#include "render/Shader.h"
#include "render/Text.h"

enum class TextAlign { Left, Center, Right };

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
    // 조명을 받지 않고 색 그대로 그린다(균열 바닥의 불빛처럼 스스로 빛나는 면).
    void DrawGlowCube(const glm::mat4 &parent, const glm::vec3 &position, const glm::vec3 &size, const glm::vec3 &color);
    // 바닥 조각: x 범위 [-halfWidth, halfWidth], z 범위 [zStart, zEnd]. 텍스처 한 장이 tileMeters를 덮고,
    // 월드 좌표 기준으로 UV를 잡아 조각이 나뉘어도(균열) 무늬가 이어진다.
    void DrawFloor(float halfWidth, float zStart, float zEnd, float tileMeters);

    // ── UI(2D) ──  좌표는 프레임버퍼 픽셀, 좌상단 원점.
    void BeginUi(int framebufferWidth, int framebufferHeight);
    void DrawRect(float x, float y, float width, float height, const glm::vec4 &rgba);
    // 글자(UTF-8, 한글 가능). (x, y)는 정렬 기준점의 윗변, pixelHeight는 글자 높이. 그린 폭을 반환한다.
    float DrawLabel(const std::string &text, float x, float y, float pixelHeight, const glm::vec4 &rgba,
                    TextAlign align = TextAlign::Left);
    void EndUi();

private:
    Shader phong, ui;
    Mesh cube, floorPlane, quad;
    GLuint floorTexture = 0;
    TextCache textCache;
};
