#include "logic/Stage.h"

namespace
{
struct Spawn
{
    AnimalType type;
    float x, z, speed;
    int segment;
};

// 배치 규칙:
//  - 구간 0(균열 앞): 시작은 들소. 균열 전에 가젤로 갈아타야 한다.
//  - 구간 1(균열~바위): 균열 건너편에서 기다리다가 플레이어가 건너오면 달린다. 들소로 갈아타야 한다.
//  - 구간 2·3: 같은 패턴을 한 번 더(균열 → 바위). 구간 4는 출구까지 함께 달리는 동물.
//  - 다음 구간의 동물은 장애물 너머로 점프해 건너뛸 수 없도록 사거리(JUMP_RANGE) 밖에 둔다.
const Spawn HERD[]{
    {AnimalType::Gazelle, -2.2f, 6.0f, 2.9f, 0},
    {AnimalType::Bison, 2.2f, 8.0f, 2.8f, 0},
    {AnimalType::Gazelle, 1.0f, 12.0f, 2.9f, 0},
    {AnimalType::Bison, -1.5f, 15.0f, 2.8f, 0},
    {AnimalType::Gazelle, -2.8f, 18.0f, 3.0f, 0},

    {AnimalType::Bison, -2.0f, 34.0f, 2.8f, 1},
    {AnimalType::Gazelle, 2.5f, 35.5f, 2.9f, 1},
    {AnimalType::Bison, 1.2f, 37.5f, 2.8f, 1},

    {AnimalType::Gazelle, -1.5f, 52.0f, 2.9f, 2},
    {AnimalType::Bison, 2.0f, 54.0f, 2.8f, 2},
    {AnimalType::Gazelle, 2.6f, 56.5f, 2.9f, 2},

    {AnimalType::Bison, 1.8f, 73.0f, 2.8f, 3},
    {AnimalType::Gazelle, -2.4f, 75.0f, 2.9f, 3},
    {AnimalType::Bison, -0.8f, 77.5f, 2.8f, 3},

    {AnimalType::Gazelle, -1.5f, 92.0f, 3.0f, 4},
    {AnimalType::Bison, 2.0f, 94.0f, 2.9f, 4},
};

Animal MakeAnimal(int id, AnimalType type, float x, float z, float speed, int segment)
{
    Animal animal;
    animal.id = id;
    animal.type = type;
    animal.position = {x, 0.0f, z};
    animal.speed = speed;
    animal.segment = segment;
    animal.halfSize = type == AnimalType::Gazelle ? glm::vec3(0.4f, 0.7f, 0.8f) : glm::vec3(0.55f, 0.85f, 1.1f);
    return animal;
}
} // namespace

void SpawnHerd(GameState &state, int checkpoint)
{
    state.animals.clear();
    state.hoveredAnimalId = -1;

    // 복귀 시 타고 있을 동물: 시작은 들소, 균열 뒤는 가젤(균열을 넘은 동물), 바위 뒤는 들소.
    AnimalType mountType = AnimalType::Bison;
    float mountZ = 2.0f;
    if (checkpoint > 0 && static_cast<size_t>(checkpoint) <= state.obstacles.size())
    {
        const Obstacle &passed = state.obstacles[static_cast<size_t>(checkpoint) - 1];
        mountType = passed.type == ObstacleType::Crack ? AnimalType::Gazelle : AnimalType::Bison;
        mountZ = passed.position.z + passed.halfSize.z + cfg::CHECKPOINT_MARGIN;
    }
    int nextId = 0;
    state.animals.push_back(MakeAnimal(nextId++, mountType, 0.0f, mountZ, 3.0f, 0));
    for (const Spawn &spawn : HERD)
    {
        if (spawn.segment < checkpoint) { continue; } // 이미 지나온 구간의 동물은 다시 만들지 않는다.
        state.animals.push_back(MakeAnimal(nextId++, spawn.type, spawn.x, spawn.z, spawn.speed, spawn.segment));
    }

    PlayerState &player = state.player;
    player.mountedAnimalId = state.animals[0].id;
    player.isJumping = false;
    player.jumpTargetId = -1;
    player.jumpTime = 0.0f;
    player.abilityActive = false;
    player.abilityTimeLeft = 0.0f;
    player.abilityCooldownLeft = 0.0f;
    player.respawnTimer = 0.0f;
    player.hurtCause = HurtCause::None;
    player.checkpoint = checkpoint;
    player.position = RiderPosition(state.animals[0]);
}

void BuildStage(GameState &state)
{
    state.obstacles.clear();
    state.resultSuccess = false;
    state.time = 0.0f;
    state.notice.clear();
    state.noticeLeft = 0.0f;

    // 장애물은 z 오름차순으로 넣는다(체크포인트 번호 = 지나온 장애물 수). 균열과 바위가 번갈아 나온다.
    for (float z : {25.0f, 45.0f, 65.0f, 85.0f})
    {
        Obstacle obstacle;
        bool crack = state.obstacles.size() % 2 == 0;
        obstacle.type = crack ? ObstacleType::Crack : ObstacleType::Rock;
        obstacle.position = {0.0f, 0.0f, z};
        // 균열은 폭 2m의 틈, 바위는 협곡 폭 전체를 막는 장벽.
        obstacle.halfSize = crack ? glm::vec3(cfg::CANYON_HALF_WIDTH, 0.1f, 1.0f) : glm::vec3(cfg::CANYON_HALF_WIDTH, 1.2f, 0.8f);
        state.obstacles.push_back(obstacle);
    }

    state.player = PlayerState{};
    SpawnHerd(state, 0);
}
