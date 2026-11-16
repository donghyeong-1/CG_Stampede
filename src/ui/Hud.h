// 타이틀·HUD·결과 화면과 버튼 배치. 글자는 Renderer::DrawLabel(한글 가능)로 그린다.
// 배치와 글자 크기는 프레임버퍼 높이 비율 기준이며, 버튼은 그리기와 클릭 판정이 같은 함수(ButtonRect)를 쓴다.
#pragma once
#include "core/GameState.h"
#include "render/Renderer.h"

enum class UiButton { None, Start, Restart, Quit };

// 현재 모드에서 마우스(프레임버퍼 픽셀)가 가리키는 버튼. 없으면 None.
UiButton HitButton(GameMode mode, float mouseX, float mouseY, int framebufferWidth, int framebufferHeight);

void DrawTitle(Renderer &renderer, const GameState &state);
void DrawPlayingHud(Renderer &renderer, const GameState &state);
void DrawResult(Renderer &renderer, const GameState &state);
