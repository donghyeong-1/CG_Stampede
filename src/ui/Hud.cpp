#include "ui/Hud.h"

namespace
{
struct Rect
{
    float x, y, w, h;
    bool Contains(float px, float py) const { return px >= x && px <= x + w && py >= y && py <= y + h; }
};

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

void DrawButton(Renderer &renderer, const GameState &state, UiButton button, const glm::vec3 &color)
{
    Rect r = ButtonRect(button, state.framebufferWidth, state.framebufferHeight);
    bool hover = r.Contains(state.mouseX, state.mouseY);
    glm::vec3 c = hover ? color * 1.25f : color;
    renderer.DrawRect(r.x, r.y, r.w, r.h, {c, 0.9f});
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
    renderer.DrawRect(0, 0, w, h, {0.05f, 0.03f, 0.03f, 0.55f});            // 화면 어둡게(반투명).
    renderer.DrawRect(w * 0.5f - h * 0.35f, h * 0.2f, h * 0.7f, h * 0.2f, {0.9f, 0.45f, 0.15f, 0.9f}); // 제목 자리.
    DrawButton(renderer, state, UiButton::Start, {0.2f, 0.65f, 0.3f});
}

void DrawPlayingHud(Renderer &renderer, const GameState &state)
{
    float h = static_cast<float>(state.framebufferHeight);
    float size = h * 0.05f, gap = h * 0.01f, x0 = h * 0.03f, y0 = h * 0.03f;
    for (int i = 0; i < cfg::START_HP; ++i) // 체력: 남은 칸은 빨강, 잃은 칸은 회색.
    {
        glm::vec4 color = i < state.player.hp ? glm::vec4(0.9f, 0.15f, 0.2f, 0.95f) : glm::vec4(0.3f, 0.3f, 0.3f, 0.6f);
        renderer.DrawRect(x0 + i * (size + gap), y0, size, size, color);
    }
    // 능력: 게이지가 차면 준비 완료(초록). 재사용 대기시간 비율로 채운다.
    float ready = 1.0f - std::clamp(state.player.abilityCooldownLeft / cfg::ABILITY_COOLDOWN, 0.0f, 1.0f);
    float barW = size * 3.0f + gap * 2.0f, barH = size * 0.4f, barY = y0 + size + gap * 2.0f;
    renderer.DrawRect(x0, barY, barW, barH, {0.1f, 0.1f, 0.1f, 0.6f});
    renderer.DrawRect(x0, barY, barW * ready, barH, ready >= 1.0f ? glm::vec4(0.2f, 0.85f, 0.3f, 0.95f) : glm::vec4(0.9f, 0.7f, 0.1f, 0.95f));
    // 조준 표시: 마우스 위치의 반투명 사각형. 조준한 동물이 있으면 노란색.
    float m = h * 0.03f;
    glm::vec4 aim = state.hoveredAnimalId >= 0 ? glm::vec4(1.0f, 0.9f, 0.2f, 0.5f) : glm::vec4(1.0f, 1.0f, 1.0f, 0.25f);
    renderer.DrawRect(state.mouseX - m * 0.5f, state.mouseY - m * 0.5f, m, m, aim);
}

void DrawResult(Renderer &renderer, const GameState &state)
{
    float w = static_cast<float>(state.framebufferWidth), h = static_cast<float>(state.framebufferHeight);
    glm::vec4 tint = state.resultSuccess ? glm::vec4(0.05f, 0.25f, 0.08f, 0.6f) : glm::vec4(0.3f, 0.04f, 0.04f, 0.6f);
    renderer.DrawRect(0, 0, w, h, tint);
    glm::vec4 banner = state.resultSuccess ? glm::vec4(0.3f, 0.9f, 0.4f, 0.9f) : glm::vec4(0.9f, 0.25f, 0.25f, 0.9f);
    renderer.DrawRect(w * 0.5f - h * 0.35f, h * 0.25f, h * 0.7f, h * 0.18f, banner); // 성공/실패 문구 자리.
    DrawButton(renderer, state, UiButton::Restart, {0.2f, 0.55f, 0.85f});
    DrawButton(renderer, state, UiButton::Quit, {0.6f, 0.2f, 0.2f});
}
