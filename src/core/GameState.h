// [공통·합의 필수] 로직(김동형)과 화면(장수현)이 공유하는 상태.
// 규칙: 로직이 값을 쓰고, 렌더러/HUD는 읽기만 한다. 필드 추가·변경은 docs/PROGRESS.md에 알린다.
#pragma once
#include "core/Config.h"
#include "core/Platform.h"

enum class GameMode { Title, Playing, Result };
enum class AnimalType { Gazelle, Bison };
enum class ObstacleType { Crack, Rock };

struct Animal
{
    int id = 0;
    AnimalType type = AnimalType::Gazelle;
    glm::vec3 position{0};  // 발바닥 중심(y=0이 바닥).
    glm::vec3 halfSize{0.4f, 0.7f, 0.9f}; // 충돌/피킹 AABB 반크기.
    float speed = 3.0f;     // +Z 자동 달리기 속도.
    float legPhase = 0.0f;  // 달리기 다리 위상(라디안).
};

struct Obstacle
{
    ObstacleType type = ObstacleType::Crack;
    glm::vec3 position{0};
    glm::vec3 halfSize{1.0f};
    bool consumed = false; // 피해 1회만 적용하기 위한 플래그.
};

struct PlayerState
{
    int hp = cfg::START_HP;
    int mountedAnimalId = -1;       // -1 = 공중(점프 중).
    bool isJumping = false;
    int jumpTargetId = -1;
    float jumpTime = 0.0f;
    float abilityCooldownLeft = 0.0f;
    bool abilityActive = false;
    float abilityTimeLeft = 0.0f;
    float progress = 0.0f;          // 0~1. 조명 전환에 사용.
    float steerInput = 0.0f;        // -1(왼쪽) ~ +1(오른쪽).
    glm::vec3 position{0};          // 원숭이 위치(탑승 중에는 동물 등 위).
};

struct GameState
{
    GameMode mode = GameMode::Title;
    bool resultSuccess = false;
    PlayerState player;
    std::vector<Animal> animals;
    std::vector<Obstacle> obstacles;
    int hoveredAnimalId = -1;       // 마우스로 조준한 동물(점프 대상 후보).
    float mouseX = 0.0f, mouseY = 0.0f; // 프레임버퍼 픽셀, 좌상단 원점.
    int framebufferWidth = cfg::WINDOW_WIDTH;
    int framebufferHeight = cfg::WINDOW_HEIGHT;
    float time = 0.0f;              // 진행 시간(초).
};

inline const Animal *FindAnimal(const GameState &state, int id)
{
    for (const Animal &animal : state.animals)
    {
        if (animal.id == id) { return &animal; }
    }
    return nullptr;
}
inline Animal *FindAnimal(GameState &state, int id)
{
    return const_cast<Animal *>(FindAnimal(static_cast<const GameState &>(state), id));
}
