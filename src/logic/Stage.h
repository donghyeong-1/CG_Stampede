// 고정 배치 패턴의 스테이지 데이터와 상태 초기화.
// 각 장애물 앞에 필요한 동물(균열=가젤, 바위=들소)이 점프 범위에 들어오도록 배치한다.
// 구간(segment) k의 동물은 플레이어가 장애물 k개를 지난 뒤에 달리기 시작한다.
#pragma once
#include "core/GameState.h"

// 동물·장애물·플레이어 상태를 모두 초기 값으로 되돌린다(재시작에서도 호출). mode는 건드리지 않는다.
void BuildStage(GameState &state);

// 체크포인트(지나온 장애물 수) 위치에 탑승 상태로 동물 떼를 다시 배치한다. 체력·장애물 상태는 유지한다.
void SpawnHerd(GameState &state, int checkpoint);
