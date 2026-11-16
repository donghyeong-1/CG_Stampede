// [공통·합의 필수] 로직과 화면이 공유하는 상태.
// 규칙: 로직(src/logic)이 값을 쓰고, 렌더러/HUD는 읽기만 한다. 필드 추가·변경은 docs/PROGRESS.md에 알린다.
#pragma once
#include "core/Config.h"
#include "core/Platform.h"

enum class GameMode { Title, Playing, Result };
enum class AnimalType { Gazelle, Bison };
enum class ObstacleType { Crack, Rock };
enum class HurtCause { None, MissedJump, Crack, Rock };

struct Animal
{
    int id = 0;
    AnimalType type = AnimalType::Gazelle;
    glm::vec3 position{0};  // 발바닥 중심(y=0이 바닥).
    glm::vec3 halfSize{0.4f, 0.7f, 0.9f}; // 충돌/피킹 AABB 반크기.
    float speed = 3.0f;     // +Z 자동 달리기 속도.
    float legPhase = 0.0f;  // 달리기 다리 위상(라디안).
    int segment = 0;        // 속한 구간. 플레이어가 이 구간에 들어오기 전에는 제자리에서 기다린다.
    float yOffset = 0.0f;   // 도약(+)·추락(-) 높이. 그리기와 탑승 위치에 더한다.
    bool waiting = false;   // 멈춰 있는 중(다리 모션 정지).
};

struct Obstacle
{
    ObstacleType type = ObstacleType::Crack;
    glm::vec3 position{0};
    glm::vec3 halfSize{1.0f};
    bool consumed = false;   // 바위: 돌진으로 부서졌다(이후 통과 가능).
    float breakTime = -1.0f; // 부서진 시각(GameState::time). 파편 연출용.
};

struct PlayerState
{
    int hp = cfg::START_HP;
    int mountedAnimalId = -1;       // -1 = 공중(점프 중).
    bool isJumping = false;
    int jumpTargetId = -1;
    float jumpTime = 0.0f;
    glm::vec3 jumpStart{0};
    glm::vec3 jumpEnd{0};           // 대상의 최신 등 위치(대상이 사라지면 마지막 값 유지).
    float abilityCooldownLeft = 0.0f;
    bool abilityActive = false;
    float abilityTimeLeft = 0.0f;
    float invincibleLeft = 0.0f;    // 복귀 직후 무적.
    float respawnTimer = 0.0f;      // > 0 이면 피해 연출 중(조작·충돌 정지).
    HurtCause hurtCause = HurtCause::None;
    int checkpoint = 0;             // 지나온 장애물 수. 복귀 지점과 동물 구간 활성화에 쓴다.
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
    std::vector<Obstacle> obstacles; // z 오름차순.
    int hoveredAnimalId = -1;       // 마우스로 조준한 동물(점프 대상 후보).
    float mouseX = 0.0f, mouseY = 0.0f; // 프레임버퍼 픽셀, 좌상단 원점.
    int framebufferWidth = cfg::WINDOW_WIDTH;
    int framebufferHeight = cfg::WINDOW_HEIGHT;
    float time = 0.0f;              // 진행 시간(초).
    std::string notice;             // HUD 중앙에 잠깐 띄우는 안내 문구(UTF-8).
    float noticeLeft = 0.0f;
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

// 동물 등 높이(발바닥 기준). 모델(Models.cpp)의 몸통 윗면과 맞춘다.
inline float AnimalBackHeight(AnimalType type) { return type == AnimalType::Gazelle ? 1.25f : 1.3f; }

// 원숭이가 앉는 위치.
inline glm::vec3 RiderPosition(const Animal &animal)
{
    return animal.position + glm::vec3(0.0f, AnimalBackHeight(animal.type) + animal.yOffset, 0.0f);
}

// 아직 지나지 않은 다음 장애물. 없으면 nullptr.
inline const Obstacle *NextObstacle(const GameState &state)
{
    size_t index = static_cast<size_t>(std::max(state.player.checkpoint, 0));
    return index < state.obstacles.size() ? &state.obstacles[index] : nullptr;
}
