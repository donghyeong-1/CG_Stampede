#include "ui/Hud.h"
#include <cstdio>

namespace
{
struct Rect
{
    float x, y, w, h;
    bool Contains(float px, float py) const { return px >= x && px <= x + w && py >= y && py <= y + h; }
};

const glm::vec4 WHITE(1.0f, 1.0f, 1.0f, 1.0f);
const glm::vec4 YELLOW(1.0f, 0.9f, 0.3f, 1.0f);
const glm::vec4 GREEN(0.5f, 1.0f, 0.55f, 1.0f);

// 창 높이에 비례해 크기를 정해 창 크기를 바꿔도 잘리지 않게 한다.
Rect ButtonRect(UiButton button, int width, int height)
{
    float unit = static_cast<float>(height);
    float w = unit * 0.34f, h = unit * 0.09f, y = unit * 0.62f;
    float cx = width * 0.5f;
    switch (button)
    {
    case UiButton::Start: return {cx - w * 0.5f, y, w, h};
    case UiButton::Restart: return {cx - w - unit * 0.02f, y, w, h};
    case UiButton::Quit: return {cx + unit * 0.02f, y, w, h};
    default: return {0, 0, 0, 0};
    }
}

// 그림자를 깔아 밝은 배경에서도 읽히게 한 글자. 그린 폭을 반환한다.
float Label(Renderer &renderer, const std::string &text, float x, float y, float size, const glm::vec4 &color,
            TextAlign align = TextAlign::Left)
{
    float offset = std::max(1.0f, size * 0.07f);
    renderer.DrawLabel(text, x + offset, y + offset, size, {0.0f, 0.0f, 0.0f, 0.75f * color.a}, align);
    return renderer.DrawLabel(text, x, y, size, color, align);
}

void DrawButton(Renderer &renderer, const GameState &state, UiButton button, const glm::vec3 &color, const char *text)
{
    Rect r = ButtonRect(button, state.framebufferWidth, state.framebufferHeight);
    bool hover = r.Contains(state.mouseX, state.mouseY);
    glm::vec3 c = hover ? color * 1.25f : color;
    renderer.DrawRect(r.x, r.y, r.w, r.h, {c, 0.9f});
    float size = r.h * 0.42f;
    Label(renderer, text, r.x + r.w * 0.5f, r.y + (r.h - size * 1.3f) * 0.5f, size, WHITE, TextAlign::Center);
}

std::string OneDecimal(float value)
{
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.1f", static_cast<double>(value));
    return buffer;
}
} // namespace

UiButton HitButton(GameMode mode, float mouseX, float mouseY, int framebufferWidth, int framebufferHeight)
{
    UiButton candidates[2]{UiButton::None, UiButton::None};
    if (mode == GameMode::Title) { candidates[0] = UiButton::Start; }
    if (mode == GameMode::Result) { candidates[0] = UiButton::Restart; candidates[1] = UiButton::Quit; }
    for (UiButton button : candidates)
    {
        if (button != UiButton::None && ButtonRect(button, framebufferWidth, framebufferHeight).Contains(mouseX, mouseY))
        {
            return button;
        }
    }
    return UiButton::None;
}

void DrawTitle(Renderer &renderer, const GameState &state)
{
    float w = static_cast<float>(state.framebufferWidth), h = static_cast<float>(state.framebufferHeight);
    float cx = w * 0.5f;
    renderer.DrawRect(0, 0, w, h, {0.05f, 0.03f, 0.03f, 0.6f}); // 화면 어둡게(반투명).
    Label(renderer, "STAMPEDE", cx, h * 0.1f, h * 0.15f, {1.0f, 0.55f, 0.15f, 1.0f}, TextAlign::Center);
    Label(renderer, "협곡 탈출", cx, h * 0.3f, h * 0.06f, WHITE, TextAlign::Center);
    Label(renderer, "산불을 피해 달리는 동물 떼의 등을 건너뛰어 협곡 출구까지 가자!", cx, h * 0.42f, h * 0.032f,
          {1.0f, 0.92f, 0.8f, 1.0f}, TextAlign::Center);
    Label(renderer, "균열은 가젤의 도약, 바위는 들소의 돌진으로 돌파하고 불붙은 통나무는 피한다", cx, h * 0.475f, h * 0.032f, YELLOW,
          TextAlign::Center);
    DrawButton(renderer, state, UiButton::Start, {0.2f, 0.65f, 0.3f}, "게임 시작 (Enter)");
    const char *controls[]{"마우스 이동: 조준    좌클릭: 조준한 동물로 점프", "A / D: 좌우 조향    Space: 탑승 동물의 능력",
                           "P: 일시정지    R: 다시 시작    Q / Esc: 종료"};
    float y = h * 0.76f;
    for (const char *line : controls)
    {
        Label(renderer, line, cx, y, h * 0.03f, {0.9f, 0.9f, 0.9f, 1.0f}, TextAlign::Center);
        y += h * 0.048f;
    }
}

void DrawPlayingHud(Renderer &renderer, const GameState &state)
{
    const PlayerState &player = state.player;
    float w = static_cast<float>(state.framebufferWidth), h = static_cast<float>(state.framebufferHeight);
    float size = h * 0.045f, gap = h * 0.01f, x0 = h * 0.03f, y0 = h * 0.03f, text = h * 0.03f;

    // 피해를 받은 순간 화면 전체가 붉게 번쩍였다가 복귀할 때까지 옅어진다.
    if (player.respawnTimer > 0.0f)
    {
        float strength = std::clamp(player.respawnTimer / cfg::RESPAWN_DELAY, 0.0f, 1.0f);
        renderer.DrawRect(0, 0, w, h, {0.9f, 0.05f, 0.05f, 0.45f * strength});
    }

    // 왼쪽 위: 체력(남은 칸은 빨강, 잃은 칸은 회색), 탑승 동물, 능력 게이지.
    float labelWidth = Label(renderer, "체력", x0, y0 + (size - text * 1.3f) * 0.5f, text, WHITE) + gap;
    for (int i = 0; i < cfg::START_HP; ++i)
    {
        glm::vec4 color = i < player.hp ? glm::vec4(0.9f, 0.15f, 0.2f, 0.95f) : glm::vec4(0.3f, 0.3f, 0.3f, 0.6f);
        renderer.DrawRect(x0 + labelWidth + static_cast<float>(i) * (size + gap), y0, size, size, color);
    }
    const Animal *mount = FindAnimal(state, player.mountedAnimalId);
    float y = y0 + size + gap * 1.5f;
    std::string mountText = "탑승: 점프 중";
    if (mount) { mountText = mount->type == AnimalType::Gazelle ? "탑승: 가젤 (도약 → 균열)" : "탑승: 들소 (돌진 → 바위)"; }
    Label(renderer, mountText, x0, y, text, WHITE);
    y += text * 1.5f;

    // 능력: 게이지가 차면 준비 완료(초록). 재사용 대기시간 비율로 채운다.
    float ready = 1.0f - std::clamp(player.abilityCooldownLeft / cfg::ABILITY_COOLDOWN, 0.0f, 1.0f);
    float barW = h * 0.3f, barH = h * 0.018f;
    renderer.DrawRect(x0, y, barW, barH, {0.1f, 0.1f, 0.1f, 0.6f});
    renderer.DrawRect(x0, y, barW * ready, barH,
                      ready >= 1.0f ? glm::vec4(0.2f, 0.85f, 0.3f, 0.95f) : glm::vec4(0.9f, 0.7f, 0.1f, 0.95f));
    y += barH + gap * 0.5f;
    if (player.abilityActive) { Label(renderer, "능력 사용 중!", x0, y, text, YELLOW); }
    else if (ready >= 1.0f) { Label(renderer, "능력 준비 완료 [Space]", x0, y, text, GREEN); }
    else { Label(renderer, "재사용 대기 " + OneDecimal(player.abilityCooldownLeft) + "초", x0, y, text, {1.0f, 0.8f, 0.3f, 1.0f}); }

    // 위쪽 가운데: 출구까지의 진행 막대와 장애물 위치.
    float trackW = h * 0.6f, trackX = w * 0.5f - trackW * 0.5f, trackY = y0 + h * 0.01f, trackH = h * 0.012f;
    renderer.DrawRect(trackX, trackY, trackW, trackH, {0.1f, 0.1f, 0.1f, 0.6f});
    renderer.DrawRect(trackX, trackY, trackW * player.progress, trackH, {0.3f, 0.9f, 0.4f, 0.9f});
    for (const Obstacle &obstacle : state.obstacles)
    {
        bool passed = obstacle.type == ObstacleType::Rock ? obstacle.consumed
                                                           : player.position.z > obstacle.position.z + obstacle.halfSize.z;
        glm::vec4 color = passed ? glm::vec4(0.5f, 0.5f, 0.5f, 0.8f) : glm::vec4(1.0f, 0.5f, 0.15f, 1.0f);
        float markX = trackX + trackW * (obstacle.position.z / cfg::STAGE_LENGTH);
        renderer.DrawRect(markX - h * 0.004f, trackY - h * 0.008f, h * 0.008f, trackH + h * 0.016f, color);
    }
    Label(renderer, "출구", trackX + trackW + gap, trackY - text * 0.55f, text * 0.9f, GREEN);

    // 오른쪽 위: 다음 장애물과 필요한 동물. 알맞은 동물을 타고 있으면 초록색.
    if (const Obstacle *next = NextObstacle(state))
    {
        bool crack = next->type == ObstacleType::Crack;
        bool matched = mount && (mount->type == AnimalType::Gazelle) == crack;
        Label(renderer, crack ? "다음 장애물: 균열" : "다음 장애물: 바위", w - x0, y0 + text * 1.2f, text, WHITE, TextAlign::Right);
        const char *advice = crack ? (matched ? "가젤로 도약! [Space]" : "가젤로 갈아타자")
                                   : (matched ? "들소로 돌진! [Space]" : "들소로 갈아타자");
        Label(renderer, advice, w - x0, y0 + text * 2.7f, text, matched ? GREEN : YELLOW, TextAlign::Right);
    }
    else
    {
        Label(renderer, "출구까지 달려라!", w - x0, y0 + text * 1.2f, text, GREEN, TextAlign::Right);
    }

    // 가운데: 잠깐 뜨는 안내 문구(마지막 0.4초 동안 흐려진다).
    if (state.noticeLeft > 0.0f && !state.notice.empty())
    {
        float alpha = std::clamp(state.noticeLeft / 0.4f, 0.0f, 1.0f);
        Label(renderer, state.notice, w * 0.5f, h * 0.2f, h * 0.06f, {1.0f, 0.95f, 0.6f, alpha}, TextAlign::Center);
    }
    Label(renderer, "좌클릭: 점프   A/D: 조향(통나무 피하기)   Space: 능력   P: 일시정지   R: 다시 시작", w * 0.5f, h * 0.94f, h * 0.026f,
          {1.0f, 1.0f, 1.0f, 0.8f}, TextAlign::Center);

    // 조준 표시: 마우스 위치의 반투명 십자. 조준한 동물이 있으면 노란색.
    float m = h * 0.035f, t = std::max(2.0f, h * 0.005f);
    glm::vec4 aim = state.hoveredAnimalId >= 0 ? glm::vec4(1.0f, 0.9f, 0.2f, 0.8f) : glm::vec4(1.0f, 1.0f, 1.0f, 0.4f);
    renderer.DrawRect(state.mouseX - m * 0.5f, state.mouseY - t * 0.5f, m, t, aim);
    renderer.DrawRect(state.mouseX - t * 0.5f, state.mouseY - m * 0.5f, t, m, aim);
    if (state.hoveredAnimalId >= 0 && !player.isJumping)
    {
        Label(renderer, "클릭: 점프", state.mouseX + m * 0.6f, state.mouseY - text * 1.5f, text * 0.9f, YELLOW);
    }

    if (state.paused)
    {
        renderer.DrawRect(0, 0, w, h, {0.0f, 0.0f, 0.0f, 0.55f});
        Label(renderer, "일시정지", w * 0.5f, h * 0.36f, h * 0.11f, WHITE, TextAlign::Center);
        Label(renderer, "P: 계속하기    R: 다시 시작    Esc: 종료", w * 0.5f, h * 0.52f, h * 0.035f, YELLOW, TextAlign::Center);
    }
}

void DrawResult(Renderer &renderer, const GameState &state)
{
    float w = static_cast<float>(state.framebufferWidth), h = static_cast<float>(state.framebufferHeight);
    glm::vec4 tint = state.resultSuccess ? glm::vec4(0.05f, 0.25f, 0.08f, 0.6f) : glm::vec4(0.3f, 0.04f, 0.04f, 0.6f);
    renderer.DrawRect(0, 0, w, h, tint);
    glm::vec4 color = state.resultSuccess ? GREEN : glm::vec4(1.0f, 0.45f, 0.4f, 1.0f);
    Label(renderer, state.resultSuccess ? "탈출 성공!" : "탈출 실패", w * 0.5f, h * 0.2f, h * 0.13f, color, TextAlign::Center);
    std::string detail = state.resultSuccess
                             ? "기록 " + OneDecimal(state.time) + "초    남은 체력 " + std::to_string(state.player.hp)
                             : "체력이 모두 떨어졌다. 장애물에 맞는 동물로 갈아타자!";
    Label(renderer, detail, w * 0.5f, h * 0.41f, h * 0.04f, WHITE, TextAlign::Center);
    if (state.bestTime > 0.0f)
    {
        std::string best = std::string(state.newRecord ? "신기록!  최고 기록 " : "최고 기록 ") + OneDecimal(state.bestTime) + "초";
        Label(renderer, best, w * 0.5f, h * 0.49f, h * 0.035f, state.newRecord ? YELLOW : WHITE, TextAlign::Center);
    }
    DrawButton(renderer, state, UiButton::Restart, {0.2f, 0.55f, 0.85f}, "다시 시작 (R)");
    DrawButton(renderer, state, UiButton::Quit, {0.6f, 0.2f, 0.2f}, "종료 (Q)");
}
