// 게임 규칙: 동물 이동, 점프·탑승 판정, 능력, 장애물 충돌, 피해·체크포인트 복귀, 성공/실패 판정.
// GameState만 읽고 쓰며 OpenGL을 호출하지 않는다(창 없이 --selftest로 검증할 수 있다).
#pragma once
#include "core/GameState.h"

// Playing 상태에서 매 프레임 호출한다. 성공/실패가 결정되면 state.mode를 Result로 바꾼다.
void StepSimulation(GameState &state, float dt);

// 조준한 동물로 점프를 시작한다. 시작하지 못하면 false(대상 없음, 사거리 밖, 이미 점프 중 등).
bool StartJump(GameState &state, int targetAnimalId);

// 탑승 동물의 능력(가젤 도약 / 들소 돌진)을 발동한다. 쿨다운 중이면 false.
bool UseAbility(GameState &state);

// 피해 1회 적용. 무적·피해 연출 중에는 무시한다(같은 원인으로 중복 피해가 들어가지 않는다).
void ApplyHurt(GameState &state, HurtCause cause);
