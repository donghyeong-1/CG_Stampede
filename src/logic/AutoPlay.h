// 검증용 자동 플레이. 다음 장애물에 맞는 동물로 갈아타고 알맞은 때에 능력을 쓴다.
// `--selftest`(스테이지가 클리어 가능한지 확인)와 `--autoplay`(화면 확인)에서만 쓴다.
#pragma once
#include "core/GameState.h"

void AutoPlayStep(GameState &state);

// 창 없이 규칙을 검증한다(plan.md 8장 시나리오). 실패한 항목 수를 반환한다.
int RunSelfTest();
