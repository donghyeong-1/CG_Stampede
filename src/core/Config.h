// [공통] 튜닝 상수 모음. 수치는 제안서 5장 "정할 값"에 해당하며 플레이 테스트로 조정한다.
// TODO(김동형): 점프/능력 수치  TODO(장수현): 조명/UV 수치
#pragma once

namespace cfg
{
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

// 협곡: +Z 방향으로 진행, 폭은 x ∈ [-CANYON_HALF_WIDTH, +CANYON_HALF_WIDTH].
// 카메라가 +Z를 바라볼 때 화면 오른쪽은 월드 -X이므로 D(오른쪽 조향)는 -X로 이동한다.
constexpr float CANYON_HALF_WIDTH = 4.0f;
constexpr float STAGE_LENGTH = 60.0f; // 이 z에 도달하면 출구 도착.
constexpr float STEER_SPEED = 4.0f;
constexpr float STEER_RIGHT_SIGN = -1.0f;

constexpr int START_HP = 3;

// 점프·능력 (김동형이 확정)
constexpr float JUMP_RANGE = 7.0f;
constexpr float JUMP_TIME = 0.6f;
constexpr float JUMP_HEIGHT = 2.0f;
constexpr float ABILITY_COOLDOWN = 4.0f;
constexpr float ABILITY_DURATION = 0.8f;

// 카메라 (김동형이 확정)
constexpr float CAMERA_FOV_DEG = 45.0f;
constexpr float CAMERA_FOLLOW_SPEED = 6.0f;
} // namespace cfg
