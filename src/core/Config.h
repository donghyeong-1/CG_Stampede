// [공통] 튜닝 상수 모음. 수치는 제안서 5장 "정할 값"에 해당하며 플레이 테스트로 조정한다.
// 수치를 바꾸면 `Stampede.exe --selftest`로 스테이지가 여전히 클리어 가능한지 확인한다.
#pragma once

namespace cfg
{
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

// 협곡: +Z 방향으로 진행, 폭은 x ∈ [-CANYON_HALF_WIDTH, +CANYON_HALF_WIDTH].
// 카메라가 +Z를 바라볼 때 화면 오른쪽은 월드 -X이므로 D(오른쪽 조향)는 -X로 이동한다.
constexpr float CANYON_HALF_WIDTH = 4.0f;
constexpr float STAGE_LENGTH = 100.0f; // 이 z에 도달하면 출구 도착.
constexpr float STEER_SPEED = 4.0f;
constexpr float STEER_RIGHT_SIGN = -1.0f;

constexpr int START_HP = 3;

// 점프: 조준한 동물까지 JUMP_TIME 동안 포물선 이동, 착지 순간 동물의 실제 위치로 판정한다.
constexpr float JUMP_RANGE = 7.0f;
constexpr float JUMP_TIME = 0.6f;
constexpr float JUMP_HEIGHT = 2.0f;
constexpr float JUMP_LAND_SLACK = 1.3f; // 착지 순간 대상이 (사거리 × 이 값)보다 멀어졌으면 실패.

// 능력: 가젤 도약(균열) / 들소 돌진(바위). 지속 시간 동안 빨라진다.
constexpr float ABILITY_COOLDOWN = 3.0f;
constexpr float ABILITY_DURATION = 1.0f;
constexpr float ABILITY_SPEED_SCALE = 2.0f;
constexpr float LEAP_HEIGHT = 1.6f; // 가젤 도약 최고 높이.

// 탑승하지 않은 동물: 넘을 수 없는 장애물 앞 이 거리에서 멈춰 기다린다.
constexpr float NPC_WAIT_GAP = 4.0f;
constexpr float NPC_HOP_MARGIN = 1.5f; // 가젤이 균열 앞뒤 이 거리부터 뛰어오른다.

// 피해와 복귀.
constexpr float RESPAWN_DELAY = 0.9f;     // 피해 연출 후 체크포인트로 복귀하기까지.
constexpr float INVINCIBLE_TIME = 1.5f;   // 복귀 직후 무적 시간.
constexpr float CHECKPOINT_MARGIN = 3.0f; // 장애물 뒤 이 거리에 복귀 지점을 둔다.

// 카메라
constexpr float CAMERA_FOV_DEG = 45.0f;
constexpr float CAMERA_FOLLOW_SPEED = 6.0f;

// 바닥 텍스처 한 장이 덮는 길이(m).
constexpr float FLOOR_TILE_METERS = 4.0f;
} // namespace cfg
