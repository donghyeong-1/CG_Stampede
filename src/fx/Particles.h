// 입자 효과(파티클): 산불의 불꽃·연기·불티와 달리는 동물의 흙먼지.
// 입자 하나는 카메라를 향하는 사각형(빌보드) 한 장이며, 시간이 지나면서 크기·색·투명도가 바뀌다 사라진다.
// 화면 연출 전용이다. 게임 규칙(logic/Simulation)에는 영향을 주지 않는다.
#pragma once
#include "core/Platform.h"
#include "render/Renderer.h"

struct Particle
{
    glm::vec3 position{0};
    glm::vec3 velocity{0};
    float life = 0.0f, maxLife = 1.0f; // 남은 수명 / 전체 수명(초).
    float sizeStart = 0.3f, sizeEnd = 0.3f;
    glm::vec4 colorStart{1}, colorEnd{1}; // 수명에 따라 보간한다.
    float lift = 0.0f;                    // 위쪽 가속도(+면 떠오르고 -면 가라앉는다).
    float drag = 0.0f;                    // 속도 감쇠.
    bool additive = false;                // true: 빛처럼 더해 그린다(불꽃). false: 반투명으로 덮는다(연기·먼지).
};

class ParticleSystem
{
public:
    void Update(float dt);
    void Draw(Renderer &renderer) const;
    void Clear() { particles.clear(); }

    // dt 동안 나올 만큼의 입자를 만든다(초당 개수 × dt, 소수 부분은 확률로 처리).
    void EmitFire(const glm::vec3 &position, float scale, float dt);  // 불꽃과 연기.
    void EmitEmbers(const glm::vec3 &center, float dt);               // 바람에 날리는 불티.
    void EmitDust(const glm::vec3 &position, float perSecond, float dt); // 발굽 뒤 흙먼지.
    void BurstDust(const glm::vec3 &position, int count, float power);   // 착지·바위 파괴 때 한꺼번에.

private:
    float Random();                       // 0~1.
    float Random(float low, float high);
    int CountFor(float perSecond, float dt);
    void Add(const Particle &particle);

    std::vector<Particle> particles;
    unsigned int seed = 20261116u;
};
