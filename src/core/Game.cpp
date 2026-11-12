#include "core/Game.h"
#include "logic/Picking.h"
#include "logic/Stage.h"
#include "model/Models.h"
#include "ui/Hud.h"

bool Game::Init(GLFWwindow *w)
{
    window = w;
    if (!renderer.Init()) { return false; }
    glfwGetFramebufferSize(window, &state.framebufferWidth, &state.framebufferHeight);
    BuildStage(state); // 타이틀 뒤에 보일 장면을 미리 구성.
    camera.Reset(state.player.position);
    state.mode = GameMode::Title;
    return true;
}

void Game::Shutdown()
{
    renderer.Shutdown();
}

void Game::Frame(float dt)
{
    PollInput();
    if (state.mode == GameMode::Playing) { UpdatePlaying(dt); }
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
        break;
    case GameMode::Playing:
        if (key == GLFW_KEY_SPACE) { TryUseAbility(); }
        if (key == GLFW_KEY_R) { StartGame(); }
        if (key == GLFW_KEY_K) { --state.player.hp; } // 디버그: 체력 감소로 실패 전환 확인.
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
    if (state.mode == GameMode::Playing) { TryJump(); return; }
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
    camera.Reset(state.player.position);
    state.mode = GameMode::Playing;
}

void Game::FinishGame(bool success)
{
    state.resultSuccess = success;
    state.mode = GameMode::Result;
}

void Game::UpdateAnimals(float dt)
{
    const float endZ = cfg::STAGE_LENGTH + 3.0f;
    for (Animal &animal : state.animals)
    {
        animal.position.z = std::min(animal.position.z + animal.speed * dt, endZ);
        animal.legPhase = std::fmod(animal.legPhase + animal.speed * 4.0f * dt, 2.0f * 3.14159265f);
    }
}

void Game::UpdatePlaying(float dt)
{
    state.time += dt;
    UpdateAnimals(dt);

    // 탑승 중인 동물을 A/D로 조향한다. 점프 중 처리는 TryJump 구현 때 추가한다.
    if (Animal *mounted = FindAnimal(state, state.player.mountedAnimalId))
    {
        float limit = cfg::CANYON_HALF_WIDTH - mounted->halfSize.x;
        mounted->position.x += state.player.steerInput * cfg::STEER_RIGHT_SIGN * cfg::STEER_SPEED * dt;
        mounted->position.x = std::clamp(mounted->position.x, -limit, limit);
        state.player.position = mounted->position + glm::vec3(0.0f, mounted->halfSize.y * 2.0f, 0.0f);
    }

    state.player.abilityCooldownLeft = std::max(0.0f, state.player.abilityCooldownLeft - dt);
    state.player.progress = std::clamp(state.player.position.z / cfg::STAGE_LENGTH, 0.0f, 1.0f);

    camera.Follow(state.player.position, dt);
    Ray ray = camera.ScreenRay(state.mouseX, state.mouseY, state.framebufferWidth, state.framebufferHeight);
    state.hoveredAnimalId = PickAnimal(state, ray);

    if (state.player.hp <= 0) { FinishGame(false); }
    else if (state.player.position.z >= cfg::STAGE_LENGTH) { FinishGame(true); }
}

void Game::TryJump()
{
    // TODO(김동형): hoveredAnimalId를 대상으로 JUMP_TIME 동안 포물선 이동 → 착지 시 실제 동물 위치로 탑승 판정.
    if (state.hoveredAnimalId >= 0) { std::cerr << "jump target: " << state.hoveredAnimalId << " (TODO)\n"; }
}

void Game::TryUseAbility()
{
    // TODO(김동형): 쿨다운 확인 후 가젤 도약/들소 돌진 발동, 장애물 충돌 판정과 피해 1회 적용.
    std::cerr << "ability (TODO)\n";
}

// ── 그리기 ──
void Game::Draw()
{
    renderer.BeginScene(camera.View(), camera.Projection(state.framebufferWidth, state.framebufferHeight),
                        camera.Eye(), LightForProgress(state.player.progress),
                        state.framebufferWidth, state.framebufferHeight);
    renderer.DrawFloor(cfg::CANYON_HALF_WIDTH, cfg::STAGE_LENGTH + 10.0f,
                       {cfg::CANYON_HALF_WIDTH, (cfg::STAGE_LENGTH + 10.0f) / 2.0f}); // 2m마다 한 번 반복.
    DrawCanyonWalls(renderer, cfg::CANYON_HALF_WIDTH, cfg::STAGE_LENGTH);
    DrawExitMarker(renderer, cfg::STAGE_LENGTH, cfg::CANYON_HALF_WIDTH);
    for (const Obstacle &obstacle : state.obstacles) { DrawObstacle(renderer, obstacle); }
    for (const Animal &animal : state.animals)
    {
        DrawAnimal(renderer, animal, animal.id == state.hoveredAnimalId);
    }
    DrawMonkey(renderer, state.player.position, state.time * 12.0f);

    renderer.BeginUi(state.framebufferWidth, state.framebufferHeight);
    if (state.mode == GameMode::Title) { DrawTitle(renderer, state); }
    if (state.mode == GameMode::Playing) { DrawPlayingHud(renderer, state); }
    if (state.mode == GameMode::Result) { DrawResult(renderer, state); }
    renderer.EndUi();
}

void Game::UpdateWindowTitle()
{
    std::string title = "STAMPEDE";
    if (state.mode == GameMode::Title) { title += " | Enter / click: Start"; }
    if (state.mode == GameMode::Playing)
    {
        const Animal *mounted = FindAnimal(state, state.player.mountedAnimalId);
        title += " | HP " + std::to_string(state.player.hp);
        title += mounted ? (mounted->type == AnimalType::Gazelle ? " | Mount: Gazelle" : " | Mount: Bison") : " | Mount: -";
        title += state.player.abilityCooldownLeft > 0.0f ? " | Ability: Cooldown" : " | Ability: Ready";
    }
    if (state.mode == GameMode::Result)
    {
        title += state.resultSuccess ? " | CLEAR!  R: Restart  Q: Quit" : " | FAILED  R: Restart  Q: Quit";
    }
    if (title != lastTitle)
    {
        glfwSetWindowTitle(window, title.c_str());
        lastTitle = title;
    }
}
