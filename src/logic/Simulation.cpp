#include "logic/Simulation.h"
#include "logic/Stage.h"

namespace
{
const float PI = 3.14159265f;
const float FALL_SPEED = 7.0f;

void Notify(GameState &state, const char *text, float seconds = 1.8f)
{
    state.notice = text;
    state.noticeLeft = seconds;
}

float HorizontalDistance(const glm::vec3 &a, const glm::vec3 &b)
{
    return glm::length(glm::vec2(a.x - b.x, a.z - b.z));
}

// 탑승하지 않은 동물이 이 장애물을 스스로 지날 수 있는가: 가젤은 균열을 뛰어넘고, 부서진 바위는 누구나 지난다.
bool CanPass(const Animal &animal, const Obstacle &obstacle)
{
    if (obstacle.consumed) { return true; }
    return obstacle.type == ObstacleType::Crack && animal.type == AnimalType::Gazelle;
}

void AdvanceLegs(Animal &animal, float distance)
{
    animal.legPhase = std::fmod(animal.legPhase + distance * 4.0f, 2.0f * PI);
}

void UpdateNpc(GameState &state, Animal &animal, float dt)
{
    if (animal.yOffset < 0.0f) // 균열에 빠진 동물은 계속 떨어진다.
    {
        animal.yOffset -= FALL_SPEED * dt;
        return;
    }
    if (animal.segment > state.player.checkpoint) // 아직 플레이어가 오지 않은 구간: 제자리 대기.
    {
        animal.waiting = true;
        return;
    }
    const float endZ = cfg::STAGE_LENGTH + 6.0f;
    float nextZ = std::min(animal.position.z + animal.speed * dt, endZ);
    for (const Obstacle &obstacle : state.obstacles)
    {
        if (CanPass(animal, obstacle)) { continue; }
        float front = obstacle.position.z - obstacle.halfSize.z;
        if (animal.position.z > front) { continue; } // 이미 지나갔다.
        float stopZ = front - animal.halfSize.z - cfg::NPC_WAIT_GAP;
        if (nextZ > stopZ) { nextZ = std::max(animal.position.z, stopZ); }
    }
    float moved = nextZ - animal.position.z;
    animal.position.z = nextZ;
    animal.waiting = moved < 1.0e-5f;
    AdvanceLegs(animal, moved);

    // 가젤은 균열 위를 포물선으로 뛰어넘는다(플레이어에게 가젤의 능력을 보여 주는 역할도 한다).
    animal.yOffset = 0.0f;
    if (animal.type == AnimalType::Gazelle)
    {
        for (const Obstacle &obstacle : state.obstacles)
        {
            if (obstacle.type != ObstacleType::Crack) { continue; }
            float span = obstacle.halfSize.z + cfg::NPC_HOP_MARGIN;
            float d = std::abs(animal.position.z - obstacle.position.z) / span;
            if (d < 1.0f) { animal.yOffset = cfg::LEAP_HEIGHT * (1.0f - d * d); }
        }
    }
}

void UpdateMounted(GameState &state, Animal &mount, float dt)
{
    PlayerState &player = state.player;
    float speed = mount.speed * (player.abilityActive ? cfg::ABILITY_SPEED_SCALE : 1.0f);
    float moved = speed * dt;
    mount.position.z += moved;
    mount.waiting = false;
    AdvanceLegs(mount, moved);

    float limit = cfg::CANYON_HALF_WIDTH - mount.halfSize.x;
    mount.position.x += player.steerInput * cfg::STEER_RIGHT_SIGN * cfg::STEER_SPEED * dt;
    mount.position.x = std::clamp(mount.position.x, -limit, limit);

    // 가젤 도약: 능력 지속 시간 동안 사인 곡선으로 떠오른다.
    mount.yOffset = 0.0f;
    if (player.abilityActive && mount.type == AnimalType::Gazelle)
    {
        float u = 1.0f - player.abilityTimeLeft / cfg::ABILITY_DURATION;
        mount.yOffset = cfg::LEAP_HEIGHT * std::sin(PI * std::clamp(u, 0.0f, 1.0f));
    }
    player.position = RiderPosition(mount);
}

void UpdateJump(GameState &state, float dt)
{
    PlayerState &player = state.player;
    player.jumpTime += dt;
    Animal *target = FindAnimal(state, player.jumpTargetId);
    if (target) { player.jumpEnd = RiderPosition(*target); } // 움직이는 대상의 현재 위치를 따라간다.

    float s = std::clamp(player.jumpTime / cfg::JUMP_TIME, 0.0f, 1.0f);
    player.position = glm::mix(player.jumpStart, player.jumpEnd, s);
    player.position.y += cfg::JUMP_HEIGHT * 4.0f * s * (1.0f - s); // 포물선.
    if (s < 1.0f) { return; }

    // 착지 판정: 착지 순간 대상의 실제 위치로 판단한다. 대상이 사라졌거나, 추락 중이거나,
    // 점프 시작점에서 너무 멀어졌으면 실패.
    player.isJumping = false;
    bool landed = target && target->yOffset > -0.5f &&
                  HorizontalDistance(player.jumpStart, target->position) <= cfg::JUMP_RANGE * cfg::JUMP_LAND_SLACK;
    if (!landed)
    {
        ApplyHurt(state, HurtCause::MissedJump);
        return;
    }
    player.mountedAnimalId = target->id;
    player.jumpTargetId = -1;
    target->segment = 0; // 한 번 탄 동물은 내린 뒤에도 계속 달린다.
    target->waiting = false;
    player.position = RiderPosition(*target);
}

void CheckObstacles(GameState &state, Animal &mount)
{
    PlayerState &player = state.player;
    for (Obstacle &obstacle : state.obstacles)
    {
        float front = obstacle.position.z - obstacle.halfSize.z;
        float back = obstacle.position.z + obstacle.halfSize.z;
        if (std::abs(mount.position.x - obstacle.position.x) > obstacle.halfSize.x + mount.halfSize.x) { continue; }

        if (obstacle.type == ObstacleType::Crack)
        {
            // 동물의 중심이 균열 위에 있을 때, 도약 중인 가젤만 무사하다.
            bool over = mount.position.z > front && mount.position.z < back;
            bool leaping = player.abilityActive && mount.type == AnimalType::Gazelle;
            if (over && !leaping) { ApplyHurt(state, HurtCause::Crack); return; }
        }
        else if (!obstacle.consumed)
        {
            bool touching = mount.position.z + mount.halfSize.z >= front && mount.position.z - mount.halfSize.z <= back;
            if (!touching) { continue; }
            if (player.abilityActive && mount.type == AnimalType::Bison)
            {
                obstacle.consumed = true; // 돌진으로 바위를 부순다.
                obstacle.breakTime = state.time;
                Notify(state, "바위 돌파!");
            }
            else
            {
                ApplyHurt(state, HurtCause::Rock);
                return;
            }
        }
    }
}

void UpdateCheckpoint(GameState &state, const Animal &mount)
{
    PlayerState &player = state.player;
    while (static_cast<size_t>(player.checkpoint) < state.obstacles.size())
    {
        const Obstacle &next = state.obstacles[static_cast<size_t>(player.checkpoint)];
        if (mount.position.z - mount.halfSize.z <= next.position.z + next.halfSize.z + 1.0f) { break; }
        ++player.checkpoint;
        if (next.type == ObstacleType::Crack) { Notify(state, "균열 통과!"); }
    }
}

void UpdateHurt(GameState &state, float dt)
{
    PlayerState &player = state.player;
    player.respawnTimer -= dt;
    if (player.hurtCause == HurtCause::Crack)
    {
        // 균열: 탄 동물과 함께 아래로 떨어진다.
        player.position.y -= FALL_SPEED * dt;
        if (Animal *mount = FindAnimal(state, player.mountedAnimalId)) { mount->yOffset -= FALL_SPEED * dt; }
    }
    else if (player.hurtCause == HurtCause::MissedJump)
    {
        player.position.y = std::max(0.0f, player.position.y - FALL_SPEED * dt); // 바닥에 떨어진다.
    }
    if (player.respawnTimer > 0.0f) { return; }

    player.respawnTimer = 0.0f;
    if (player.hp <= 0)
    {
        state.resultSuccess = false;
        state.mode = GameMode::Result;
        return;
    }
    SpawnHerd(state, player.checkpoint); // 가까운 안전 구간에서 탑승 상태로 복귀.
    player.invincibleLeft = cfg::INVINCIBLE_TIME;
}
} // namespace

void ApplyHurt(GameState &state, HurtCause cause)
{
    PlayerState &player = state.player;
    if (player.respawnTimer > 0.0f || player.invincibleLeft > 0.0f) { return; }
    player.hp = std::max(0, player.hp - 1);
    player.respawnTimer = cfg::RESPAWN_DELAY;
    player.hurtCause = cause;
    player.isJumping = false;
    player.abilityActive = false;
    player.abilityTimeLeft = 0.0f;
    switch (cause)
    {
    case HurtCause::MissedJump: Notify(state, "착지 실패!  체력 -1"); break;
    case HurtCause::Crack: Notify(state, "균열에 빠졌다!  체력 -1"); break;
    case HurtCause::Rock: Notify(state, "바위에 부딪혔다!  체력 -1"); break;
    default: break;
    }
}

bool StartJump(GameState &state, int targetAnimalId)
{
    PlayerState &player = state.player;
    if (state.mode != GameMode::Playing || player.isJumping || player.respawnTimer > 0.0f) { return false; }
    const Animal *mount = FindAnimal(state, player.mountedAnimalId);
    const Animal *target = FindAnimal(state, targetAnimalId);
    if (!mount || !target || target->id == mount->id) { return false; }
    if (glm::distance(target->position, player.position) > cfg::JUMP_RANGE) { return false; }

    player.isJumping = true;
    player.jumpTargetId = target->id;
    player.jumpTime = 0.0f;
    player.jumpStart = player.position;
    player.jumpEnd = RiderPosition(*target);
    player.mountedAnimalId = -1;
    player.abilityActive = false; // 뛰어내리면 능력은 끊긴다.
    player.abilityTimeLeft = 0.0f;
    return true;
}

bool UseAbility(GameState &state)
{
    PlayerState &player = state.player;
    if (state.mode != GameMode::Playing || player.isJumping || player.respawnTimer > 0.0f) { return false; }
    const Animal *mount = FindAnimal(state, player.mountedAnimalId);
    if (!mount) { return false; }
    if (player.abilityCooldownLeft > 0.0f)
    {
        Notify(state, "능력 재사용 대기 중", 0.8f);
        return false;
    }
    player.abilityActive = true;
    player.abilityTimeLeft = cfg::ABILITY_DURATION;
    player.abilityCooldownLeft = cfg::ABILITY_COOLDOWN;
    Notify(state, mount->type == AnimalType::Gazelle ? "도약!" : "돌진!", 0.8f);
    return true;
}

void StepSimulation(GameState &state, float dt)
{
    if (state.mode != GameMode::Playing) { return; }
    PlayerState &player = state.player;
    state.time += dt;
    state.noticeLeft = std::max(0.0f, state.noticeLeft - dt);
    player.abilityCooldownLeft = std::max(0.0f, player.abilityCooldownLeft - dt);
    player.invincibleLeft = std::max(0.0f, player.invincibleLeft - dt);

    bool hurting = player.respawnTimer > 0.0f;
    for (Animal &animal : state.animals)
    {
        if (animal.id != player.mountedAnimalId) { UpdateNpc(state, animal, dt); }
    }
    if (hurting)
    {
        UpdateHurt(state, dt);
        return;
    }
    if (player.hp <= 0) // 디버그 키 등으로 체력이 0이 된 경우.
    {
        state.resultSuccess = false;
        state.mode = GameMode::Result;
        return;
    }

    if (player.isJumping)
    {
        UpdateJump(state, dt);
    }
    else if (Animal *mount = FindAnimal(state, player.mountedAnimalId))
    {
        UpdateMounted(state, *mount, dt);
        CheckObstacles(state, *mount);
        if (player.respawnTimer <= 0.0f) { UpdateCheckpoint(state, *mount); }
    }

    if (player.abilityActive)
    {
        player.abilityTimeLeft -= dt;
        if (player.abilityTimeLeft <= 0.0f)
        {
            player.abilityActive = false;
            player.abilityTimeLeft = 0.0f;
        }
    }

    player.progress = std::clamp(player.position.z / cfg::STAGE_LENGTH, 0.0f, 1.0f);
    if (player.respawnTimer <= 0.0f && !player.isJumping && player.position.z >= cfg::STAGE_LENGTH)
    {
        state.resultSuccess = true;
        state.mode = GameMode::Result;
    }
}
