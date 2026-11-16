// [장수현] 정점 구조와 메시(VAO/VBO). 실습의 Vertex{위치,색}에 법선·UV를 추가했다(Phong 조명, 텍스처용).
// 색은 정점이 아니라 셰이더 uniform(uColor)으로 전달한다.
#pragma once
#include "core/Platform.h"

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

class Mesh
{
public:
    void Create(const std::vector<Vertex> &vertices); // 정적 VBO 1회 업로드.
    void Draw() const;
    void Destroy();

private:
    GLuint vao = 0, vbo = 0;
    GLsizei count = 0;
};

std::vector<Vertex> MakeCubeVertices();  // 중심 원점, 한 변 1, 바깥 법선(CCW).
std::vector<Vertex> MakeFloorVertices(); // x 범위 [-1,1], z 범위 [0,1], y=0, 법선 +Y, uv (0~1, 0~1).
std::vector<Vertex> MakeQuadVertices();  // UI용 단위 사각형 (0,0)~(1,1), z=0.
