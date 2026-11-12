// [장수현] 타이틀·HUD·결과 화면과 버튼 배치. 글자 출력은 아직 없다(창 제목으로 임시 표시).
// 버튼 배치는 프레임버퍼 크기 비율 기준이며, 그리기와 클릭 판정이 같은 함수(ButtonRect)를 쓴다.
// TODO(장수현): 비트맵 폰트 글자 출력, 체력/능력 아이콘, 반투명 조준 표시 다듬기.
#pragma once
#include "core/GameState.h"
#include "render/Renderer.h"

enum class UiButton { None, Start, Restart, Quit };

// 현재 모드에서 마우스(프레임버퍼 픽셀)가 가리키는 버튼. 없으면 None.
UiButton HitButton(GameMode mode, float mouseX, float mouseY, int framebufferWidth, int framebufferHeight);

void DrawTitle(Renderer &renderer, const GameState &state);
void DrawPlayingHud(Renderer &renderer, const GameState &state);
void DrawResult(Renderer &renderer, const GameState &state);
