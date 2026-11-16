// 기본 도형(상자) 조합 모델: 원숭이·가젤·들소·장애물·협곡과 달리기 모션.
// 몸통 행렬을 부모로 다리·목·머리를 계층 변환한다(실습 23 RobotModel/DrawRobot 방식).
#pragma once
#include "core/GameState.h"
#include "render/Renderer.h"

// hovered: 마우스 조준 강조. boosted: 능력(도약·돌진) 사용 중.
void DrawAnimal(Renderer &renderer, const Animal &animal, bool hovered, bool boosted);
// position은 앉은 자리(동물 등 위). jumping이면 팔을 들고 다리를 편다.
void DrawMonkey(Renderer &renderer, const glm::vec3 &position, float time, bool jumping);
// 바위 장벽. 부서진 뒤에는 time 기준으로 파편이 튀고 사라진다.
void DrawRockWall(Renderer &renderer, const Obstacle &obstacle, float time);
// 균열: 바닥이 끊긴 자리의 절벽 면과 아래쪽 불빛.
void DrawCrackPit(Renderer &renderer, const Obstacle &obstacle, float halfWidth);
void DrawExitMarker(Renderer &renderer, float z, float halfWidth);
void DrawCanyonWalls(Renderer &renderer, float halfWidth, float zStart, float zEnd);
