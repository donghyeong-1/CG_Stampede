#include "core/Game.h"
#include "logic/AutoPlay.h"
#include "logic/Picking.h"
#include "logic/Simulation.h"
#include "logic/Stage.h"
#include "model/Models.h"
#include "ui/Hud.h"

namespace
{
// 바닥·절벽을 그리는 z 범위. 불 위치 계산도 같은 범위를 쓴다.
const float WORLD_Z_START = -30.0f;
const float WORLD_Z_END = cfg::STAGE_LENGTH + 40.0f;
} // namespace

bool Game::Init(GLFWwindow *w)
{
    window = w;
    if (!renderer.Init()) { return false; }
    glfwGetFramebufferSize(window, &state.framebufferWidth, &state.framebufferHeight);
    fireSpots = CanyonFireSpots(cfg::CANYON_HALF_WIDTH, WORLD_Z_START, WORLD_Z_END);
    BuildStage(state); // 타이틀 뒤에 보일 장면을 미리 구성.
    camera.Reset(CameraTarget());
    state.mode = GameMode::Title;
    return true;
}

void Game::Shutdown()
{
    renderer.Shutdown();
}

void Game::Frame(float dt)
{
    elapsed += dt;
    PollInput();
    if (autoPlay)
    {
        if (state.mode == GameMode::Title && elapsed > 1.0f) { StartGame(); }
        AutoPlayStep(state);
    }
    if (state.mode == GameMode::Playing) { UpdatePlaying(dt); }
    UpdateEffects(dt);
    Draw();
    UpdateWindowTitle();
}

// ── 입력 ──
void Game::OnKey(int key, int action)
{
    if (action != GLFW_PRESS) { return; }
    if (key == GLFW_KEY_ESCAPE) { glfwSetWindowShouldClose(window, GLFW_TRUE); return; }
    switch (state.mode)
    {
    case GameMode::Title:
        if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) { StartGame(); }
        if (key == GLFW_KEY_Q) { glfwSetWindowShouldClose(window, GLFW_TRUE); }
        break;
    case GameMode::Playing:
        if (key == GLFW_KEY_SPACE) { UseAbility(state); }
        if (key == GLFW_KEY_R) { StartGame(); }
        if (key == GLFW_KEY_K) { ApplyHurt(state, HurtCause::MissedJump); } // 디버그: 피해·복귀·실패 전환 확인.
        break;
    case GameMode::Result:
        if (key == GLFW_KEY_R) { StartGame(); }
        if (key == GLFW_KEY_Q) { glfwSetWindowShouldClose(window, GLFW_TRUE); }
        break;
    }
}

void Game::OnMouseButton(int button, int action)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) { return; }
    if (state.mode == GameMode::Playing)
    {
        StartJump(state, state.hoveredAnimalId); // 조준한 동물이 없으면 아무 일도 일어나지 않는다.
        return;
    }
    UiButton hit = HitButton(state.mode, state.mouseX, state.mouseY, state.framebufferWidth, state.framebufferHeight);
    if (hit == UiButton::Start || hit == UiButton::Restart) { StartGame(); }
    if (hit == UiButton::Quit) { glfwSetWindowShouldClose(window, GLFW_TRUE); }
}

void Game::OnCursorPos(double x, double y)
{
    // GLFW 커서 좌표는 창 좌표. 프레임버퍼 크기와 다를 수 있으므로(DPI) 비율로 변환한다.
    int windowWidth = 1, windowHeight = 1;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    state.mouseX = static_cast<float>(x * state.framebufferWidth / std::max(windowWidth, 1));
    state.mouseY = static_cast<float>(y * state.framebufferHeight / std::max(windowHeight, 1));
}

void Game::OnFramebufferResize(int width, int height)
{
    state.framebufferWidth = std::max(width, 1);
    state.framebufferHeight = std::max(height, 1);
}

void Game::PollInput()
{
    float steer = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) { steer -= 1.0f; }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) { steer += 1.0f; }
    state.player.steerInput = steer;
}

// ── 갱신 ──
void Game::StartGame()
{
    BuildStage(state);
    camera.Reset(CameraTarget());
    state.mode = GameMode::Playing;
}

glm::vec3 Game::CameraTarget() const
{
    // 점프·도약·추락으로 높이가 바뀌어도 화면이 출렁이지 않도록 높이 변화는 조금만 따라간다.
    const float baseHeight = 1.3f;
    glm::vec3 target = state.player.position;
    target.y = baseHeight + (std::max(target.y, 0.0f) - baseHeight) * 0.25f;
    return target;
}

void Game::UpdatePlaying(float dt)
{
    // 규칙을 한 단계 진행하고, 그 사이에 일어난 일(착지·도약 끝·바위 파괴)에 먼지를 터뜨린다.
    const int mountedBefore = state.player.mountedAnimalId;
    const bool abilityBefore = state.player.abilityActive;
    int brokenBefore = 0, brokenAfter = 0;
    for (const Obstacle &obstacle : state.obstacles) { brokenBefore += obstacle.consumed ? 1 : 0; }

    StepSimulation(state, dt);

    const Animal *mount = FindAnimal(state, state.player.mountedAnimalId);
    if (mount && mountedBefore < 0) { effects.BurstDust(mount->position, 14, 0.8f); } // 점프 착지.
    if (mount && abilityBefore && !state.player.abilityActive && mount->type == AnimalType::Gazelle)
    {
        effects.BurstDust(mount->position, 18, 1.0f); // 가젤 도약 착지.
    }
    for (const Obstacle &obstacle : state.obstacles) { brokenAfter += obstacle.consumed ? 1 : 0; }
    if (brokenAfter > brokenBefore && mount)
    {
        effects.BurstDust(mount->position + glm::vec3(0.0f, 0.0f, 1.5f), 70, 1.8f); // 바위 파괴.
    }

    camera.Follow(CameraTarget(), dt);

    // 마우스 조준: 점프할 수 있을 때만 대상을 고른다.
    state.hoveredAnimalId = -1;
    if (state.mode == GameMode::Playing && !state.player.isJumping && state.player.respawnTimer <= 0.0f)
    {
        Ray ray = camera.ScreenRay(state.mouseX, state.mouseY, state.framebufferWidth, state.framebufferHeight);
        state.hoveredAnimalId = PickAnimal(state, ray);
    }
}

void Game::UpdateEffects(float dt)
{
    // 산불: 화면에 들어올 만한 범위의 나무에서만 불꽃·연기를 낸다. 타이틀·결과 화면에서도 계속 탄다.
    glm::vec3 focus = CameraTarget();
    for (const FireSpot &spot : fireSpots)
    {
        if (spot.position.z < focus.z - 14.0f || spot.position.z > focus.z + 48.0f) { continue; }
        effects.EmitFire(spot.position, spot.scale, dt);
    }
    effects.EmitEmbers(focus, dt);

    // 흙먼지: 땅을 딛고 달리는 동물의 뒷발 쪽에서 피어오른다. 능력으로 빨라진 동물은 더 많이.
    if (state.mode == GameMode::Playing)
    {
        for (const Animal &animal : state.animals)
        {
            if (animal.waiting || std::abs(animal.yOffset) > 0.05f) { continue; }
            if (animal.position.z < focus.z - 14.0f || animal.position.z > focus.z + 48.0f) { continue; }
            bool boosted = state.player.abilityActive && animal.id == state.player.mountedAnimalId;
            float rate = (animal.type == AnimalType::Bison ? 22.0f : 14.0f) * (boosted ? 4.0f : 1.0f);
            effects.EmitDust(animal.position - glm::vec3(0.0f, 0.0f, animal.halfSize.z * 0.8f), rate, dt);
        }
    }
    effects.Update(dt);
}

// ── 그리기 ──
void Game::Draw()
{
    const PlayerState &player = state.player;
    renderer.BeginScene(camera.View(), camera.Projection(state.framebufferWidth, state.framebufferHeight),
                        camera.Eye(), LightForProgress(player.progress),
                        state.framebufferWidth, state.framebufferHeight);

    // 바닥: 균열 자리를 비우고 조각으로 나눠 그린다. 절벽 안쪽 면이 들쭉날쭉하므로 조금 넓게 깐다.
    const float floorHalfWidth = cfg::CANYON_HALF_WIDTH + 0.7f;
    const float zStart = WORLD_Z_START, zEnd = WORLD_Z_END;
    float z = zStart;
    for (const Obstacle &obstacle : state.obstacles)
    {
        if (obstacle.type != ObstacleType::Crack) { continue; }
        renderer.DrawFloor(floorHalfWidth, z, obstacle.position.z - obstacle.halfSize.z, cfg::FLOOR_TILE_METERS);
        DrawCrackPit(renderer, obstacle, floorHalfWidth);
        z = obstacle.position.z + obstacle.halfSize.z;
    }
    renderer.DrawFloor(floorHalfWidth, z, zEnd, cfg::FLOOR_TILE_METERS);

    DrawCanyonWalls(renderer, cfg::CANYON_HALF_WIDTH, zStart, zEnd);
    DrawExitMarker(renderer, cfg::STAGE_LENGTH, cfg::CANYON_HALF_WIDTH);
    for (const Obstacle &obstacle : state.obstacles)
    {
        if (obstacle.type == ObstacleType::Rock) { DrawRockWall(renderer, obstacle, state.time); }
    }
    for (const Animal &animal : state.animals)
    {
        bool boosted = player.abilityActive && animal.id == player.mountedAnimalId;
        DrawAnimal(renderer, animal, animal.id == state.hoveredAnimalId, boosted);
    }
    // 복귀 직후 무적 시간에는 원숭이를 깜빡여 알린다.
    bool blinkHidden = player.invincibleLeft > 0.0f && std::fmod(player.invincibleLeft * 10.0f, 2.0f) < 1.0f;
    if (!blinkHidden && player.position.y > -6.0f)
    {
        DrawMonkey(renderer, player.position, elapsed, player.isJumping);
    }

    effects.Draw(renderer); // 반투명 입자는 불투명한 물체를 모두 그린 뒤에.

    renderer.BeginUi(state.framebufferWidth, state.framebufferHeight);
    if (state.mode == GameMode::Title) { DrawTitle(renderer, state); }
    if (state.mode == GameMode::Playing) { DrawPlayingHud(renderer, state); }
    if (state.mode == GameMode::Result) { DrawResult(renderer, state); }
    renderer.EndUi();
}

void Game::UpdateWindowTitle()
{
    std::string title = "STAMPEDE";
    if (state.mode == GameMode::Playing) { title += " | HP " + std::to_string(state.player.hp); }
    if (state.mode == GameMode::Result) { title += state.resultSuccess ? " | CLEAR" : " | FAILED"; }
    if (title != lastTitle)
    {
        glfwSetWindowTitle(window, title.c_str());
        lastTitle = title;
    }
}

bool Game::SaveScreenshot(const std::string &path) const
{
    int width = state.framebufferWidth, height = state.framebufferHeight;
    size_t rowSize = (static_cast<size_t>(width) * 3 + 3) / 4 * 4; // BMP 행은 4바이트 단위.
    std::vector<unsigned char> pixels(rowSize * static_cast<size_t>(height));
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels.data()); // BMP와 같은 순서(아래 행부터, BGR).

    unsigned char header[54]{};
    auto put32 = [&header](size_t offset, unsigned int value)
    {
        for (size_t i = 0; i < 4; ++i) { header[offset + i] = static_cast<unsigned char>((value >> (8 * i)) & 0xFFu); }
    };
    header[0] = 'B';
    header[1] = 'M';
    put32(2, static_cast<unsigned int>(sizeof(header) + pixels.size()));
    put32(10, sizeof(header));
    put32(14, 40);
    put32(18, static_cast<unsigned int>(width));
    put32(22, static_cast<unsigned int>(height));
    header[26] = 1;
    header[28] = 24;
    put32(34, static_cast<unsigned int>(pixels.size()));
    std::ofstream file(std::filesystem::u8path(path), std::ios::binary);
    if (!file) { return false; }
    file.write(reinterpret_cast<const char *>(header), sizeof(header));
    file.write(reinterpret_cast<const char *>(pixels.data()), static_cast<std::streamsize>(pixels.size()));
    return static_cast<bool>(file);
}
