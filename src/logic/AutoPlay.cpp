#include "logic/AutoPlay.h"
#include "logic/Camera.h"
#include "logic/Picking.h"
#include "logic/Simulation.h"
#include "logic/Stage.h"

void AutoPlayStep(GameState &state)
{
    PlayerState &player = state.player;
    if (state.mode != GameMode::Playing || player.isJumping || player.respawnTimer > 0.0f) { return; }
    const Animal *mount = FindAnimal(state, player.mountedAnimalId);
    const Obstacle *next = NextObstacle(state);
    if (!mount || !next) { return; }

    AnimalType needed = next->type == ObstacleType::Crack ? AnimalType::Gazelle : AnimalType::Bison;
    if (mount->type != needed)
    {
        // 필요한 종류 중 사거리 안에서 가장 가까운 동물로 갈아탄다.
        int best = -1;
        float bestDistance = cfg::JUMP_RANGE;
        for (const Animal &animal : state.animals)
        {
            if (animal.type != needed || animal.id == mount->id || animal.yOffset < 0.0f) { continue; }
            if (animal.position.z > next->position.z) { continue; } // 장애물 너머의 동물은 고르지 않는다.
            float distance = glm::distance(animal.position, player.position);
            if (distance < bestDistance)
            {
                bestDistance = distance;
                best = animal.id;
            }
        }
        if (best >= 0) { StartJump(state, best); }
        return;
    }
    if (player.abilityActive || player.abilityCooldownLeft > 0.0f) { return; }
    float front = next->position.z - next->halfSize.z;
    float gap = next->type == ObstacleType::Crack ? front - mount->position.z
                                                  : front - (mount->position.z + mount->halfSize.z);
    float trigger = next->type == ObstacleType::Crack ? 1.2f : 2.0f;
    if (gap < trigger) { UseAbility(state); }
}

// ── 자체 검증 ──
namespace
{
const float DT = 1.0f / 120.0f;
int failures = 0;

void Check(bool condition, const char *name)
{
    std::cout << (condition ? "[PASS] " : "[FAIL] ") << name << '\n';
    if (!condition) { ++failures; }
}

GameState NewGame()
{
    GameState state;
    BuildStage(state);
    state.mode = GameMode::Playing;
    return state;
}

// until이 참이 되거나 시간이 다 될 때까지 진행한다. bot이면 자동 플레이.
template <typename Predicate> bool Run(GameState &state, float seconds, bool bot, Predicate until)
{
    for (float t = 0.0f; t < seconds; t += DT)
    {
        if (until(state)) { return true; }
        if (state.mode != GameMode::Playing) { return false; }
        if (bot) { AutoPlayStep(state); }
        StepSimulation(state, DT);
    }
    return until(state);
}

int FirstOfType(const GameState &state, AnimalType type, int exceptId)
{
    for (const Animal &animal : state.animals)
    {
        if (animal.type == type && animal.id != exceptId &&
            glm::distance(animal.position, state.player.position) <= cfg::JUMP_RANGE)
        {
            return animal.id;
        }
    }
    return -1;
}

void TestClear()
{
    GameState state = NewGame();
    bool finished = Run(state, 120.0f, true, [](const GameState &s) { return s.mode == GameMode::Result; });
    Check(finished && state.resultSuccess, "자동 플레이: 모든 장애물을 넘어 출구 도달");
    Check(state.player.hp == cfg::START_HP, "자동 플레이: 정상 능력 사용 시 피해 없음");
    Check(state.obstacles[1].consumed && state.obstacles[3].consumed, "들소 돌진: 바위가 부서짐");
}

void TestNoInputFails()
{
    GameState state = NewGame();
    int drops = 0, lastHp = state.player.hp;
    bool singleStep = true;
    for (float t = 0.0f; t < 120.0f && state.mode == GameMode::Playing; t += DT)
    {
        StepSimulation(state, DT);
        if (state.player.hp != lastHp)
        {
            if (lastHp - state.player.hp != 1) { singleStep = false; }
            ++drops;
            lastHp = state.player.hp;
        }
    }
    Check(state.mode == GameMode::Result && !state.resultSuccess, "입력 없음: 체력 0에서 실패 결과로 전환");
    Check(drops == cfg::START_HP && singleStep, "입력 없음: 균열마다 피해가 정확히 1씩 적용");
}

void TestJump()
{
    GameState state = NewGame();
    int target = FirstOfType(state, AnimalType::Gazelle, state.player.mountedAnimalId);
    Check(target >= 0 && StartJump(state, target), "점프: 사거리 안의 동물로 점프 시작");
    Check(!StartJump(state, target), "점프: 점프 중에는 다시 점프할 수 없음");
    Run(state, 2.0f, false, [](const GameState &s) { return !s.player.isJumping; });
    Check(state.player.mountedAnimalId == target && state.player.hp == cfg::START_HP, "점프: 착지 시 대상 동물에 탑승");

    GameState miss = NewGame();
    target = FirstOfType(miss, AnimalType::Gazelle, miss.player.mountedAnimalId);
    StartJump(miss, target);
    Run(miss, cfg::JUMP_TIME * 0.5f, false, [](const GameState &) { return false; });
    miss.animals.erase(std::remove_if(miss.animals.begin(), miss.animals.end(),
                                      [target](const Animal &a) { return a.id == target; }),
                       miss.animals.end()); // 착지 지점에 동물이 없는 상황.
    Run(miss, 1.0f, false, [](const GameState &s) { return !s.player.isJumping; });
    Check(miss.player.hp == cfg::START_HP - 1, "점프: 착지 시 동물이 없으면 체력 -1");
    Run(miss, 3.0f, false, [](const GameState &s) { return s.player.respawnTimer <= 0.0f; });
    Check(FindAnimal(miss, miss.player.mountedAnimalId) != nullptr && miss.player.invincibleLeft > 0.0f,
          "점프 실패 후: 체크포인트에서 탑승 상태로 복귀(무적 시간 적용)");

    GameState distant = NewGame();
    int farthest = distant.animals.back().id;
    Check(!StartJump(distant, farthest), "점프: 사거리 밖 동물로는 점프할 수 없음");
}

void TestAbilityMisuse()
{
    // 들소로 균열에 돌진(오사용): 피해 1회.
    GameState state = NewGame();
    Run(state, 30.0f, false, [](const GameState &s) { return s.player.position.z > 21.0f; });
    Check(UseAbility(state), "능력: 준비 상태에서 발동");
    Check(!UseAbility(state), "능력: 쿨다운 중에는 발동하지 않음");
    Run(state, 5.0f, false, [](const GameState &s) { return s.player.hp < cfg::START_HP; });
    Check(state.player.hp == cfg::START_HP - 1 && state.player.hurtCause == HurtCause::Crack,
          "오사용: 들소 돌진으로는 균열을 넘지 못하고 체력 -1");
    Run(state, cfg::RESPAWN_DELAY * 0.9f, false, [](const GameState &) { return false; });
    Check(state.player.hp == cfg::START_HP - 1, "오사용: 피해 연출 중 중복 피해 없음");

    // 가젤로 바위에 도약(오사용): 피해 1회, 바위는 그대로.
    GameState rock = NewGame();
    Run(rock, 60.0f, true, [](const GameState &s) { return s.player.checkpoint >= 1 && !s.player.isJumping; });
    Check(rock.player.checkpoint == 1 && rock.player.hp == cfg::START_HP, "가젤 도약: 균열을 넘어 체크포인트 1 도달");
    const Animal *mount = FindAnimal(rock, rock.player.mountedAnimalId);
    Check(mount && mount->type == AnimalType::Gazelle, "균열 통과 후 가젤에 탑승 중");
    Run(rock, 30.0f, false, [](const GameState &s) { return s.player.position.z > 41.0f; });
    rock.player.abilityCooldownLeft = 0.0f;
    UseAbility(rock);
    Run(rock, 5.0f, false, [](const GameState &s) { return s.player.hp < cfg::START_HP; });
    Check(rock.player.hp == cfg::START_HP - 1 && rock.player.hurtCause == HurtCause::Rock && !rock.obstacles[1].consumed,
          "오사용: 가젤 도약으로는 바위를 넘지 못하고 체력 -1");
    Run(rock, 3.0f, false, [](const GameState &s) { return s.player.respawnTimer <= 0.0f; });
    Check(rock.player.checkpoint == 1 && rock.player.position.z > 26.0f && rock.player.position.z < 40.0f,
          "바위 충돌 후: 균열 뒤 체크포인트에서 복귀");
}

void TestRestart()
{
    GameState state = NewGame();
    Run(state, 12.0f, false, [](const GameState &) { return false; });
    BuildStage(state);
    const PlayerState &p = state.player;
    Check(p.hp == cfg::START_HP && p.checkpoint == 0 && p.abilityCooldownLeft == 0.0f && !p.isJumping &&
              state.time == 0.0f && !state.obstacles[1].consumed && FindAnimal(state, p.mountedAnimalId) != nullptr,
          "재시작: 체력·체크포인트·쿨다운·장애물·시간이 모두 초기화");
}

void TestPicking()
{
    // 동물 중심을 화면에 투영한 점으로 레이를 쏘면 그 동물이 선택되어야 한다(창 크기·화면비가 달라도).
    const int sizes[][2]{{1280, 720}, {800, 600}, {1920, 1080}, {500, 900}};
    bool allMatch = true;
    for (const auto &size : sizes)
    {
        GameState state = NewGame();
        Camera camera;
        camera.Reset(state.player.position);
        glm::mat4 view = camera.View(), projection = camera.Projection(size[0], size[1]);
        for (const Animal &animal : state.animals)
        {
            if (animal.id == state.player.mountedAnimalId) { continue; }
            if (glm::distance(animal.position, state.player.position) > cfg::JUMP_RANGE) { continue; }
            glm::vec3 center = animal.position + glm::vec3(0.0f, animal.halfSize.y, 0.0f);
            glm::vec3 screen = glm::project(center, view, projection, glm::vec4(0, 0, size[0], size[1]));
            Ray ray = camera.ScreenRay(screen.x, static_cast<float>(size[1]) - screen.y, size[0], size[1]);
            if (PickAnimal(state, ray) != animal.id) { allMatch = false; }
        }
        Ray corner = camera.ScreenRay(1.0f, 1.0f, size[0], size[1]);
        if (PickAnimal(state, corner) != -1) { allMatch = false; }
    }
    Check(allMatch, "피킹: 창 크기 4종에서 조준한 동물과 선택된 동물이 일치");
}
} // namespace

int RunSelfTest()
{
    failures = 0;
    TestClear();
    TestNoInputFails();
    TestJump();
    TestAbilityMisuse();
    TestRestart();
    TestPicking();
    std::cout << (failures == 0 ? "SELFTEST OK" : "SELFTEST FAILED") << " (" << failures << " failed)\n";
    return failures;
}
