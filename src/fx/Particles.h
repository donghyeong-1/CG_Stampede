// 블록 효과: 산불의 불꽃·연기·불티와 달리는 동물의 흙먼지를 작은 정육면체 조각으로 표현한다.
// 조각 하나는 큐브 한 개이며, 날아가면서 돌고 색이 바뀌고 작아지다가 사라진다(반투명·빌보드는 쓰지 않는다).
// 화면 연출 전용이다. 게임 규칙(logic/Simulation)에는 영향을 주지 않는다.
#pragma once
#include "core/Platform.h"
#include "render/Renderer.h"

struct Particle
{
    glm::vec3 position{0};
    glm::vec3 velocity{0};
    float life = 0.0f, maxLife = 1.0f; // 남은 수명 / 전체 수명(초).
    float sizeStart = 0.2f, sizeEnd = 0.2f; // 한 변 길이. 수명 끝에는 0으로 줄어들며 사라진다.
    glm::vec3 colorStart{1}, colorEnd{1};   // 수명에 따라 보간한다.
    float lift = 0.0f;                      // 위쪽 가속도(+면 떠오르고 -면 가라앉는다).
    float drag = 0.0f;                      // 속도 감쇠.
    glm::vec3 spinAxis{0, 1, 0};
    float spinSpeed = 0.0f;                 // 초당 회전 각도(도).
    bool glow = false;                      // true: 조명과 무관하게 스스로 빛난다(불꽃·불티).
};

class ParticleSystem
{
public:
    void Update(float dt);
    void Draw(Renderer &renderer) const; // 월드 그리기(BeginScene 이후) 중에 호출한다.
    void Clear() { particles.clear(); }

    // dt 동안 나올 만큼의 조각을 만든다(초당 개수 × dt, 소수 부분은 확률로 처리).
    void EmitFire(const glm::vec3 &position, float scale, float dt);     // 불꽃과 연기.
    void EmitEmbers(const glm::vec3 &center, float dt);                  // 바람에 날리는 불티.
    void EmitDust(const glm::vec3 &position, float perSecond, float dt); // 발굽 뒤 흙먼지.
    void BurstDust(const glm::vec3 &position, int count, float power);   // 착지·파괴 때 한꺼번에.

private:
    float Random();                       // 0~1.
    float Random(float low, float high);
    int CountFor(float perSecond, float dt);
    void Add(Particle particle);

    std::vector<Particle> particles;
    unsigned int seed = 20261116u;
};
