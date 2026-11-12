// [장수현] 기본 도형 조합 모델: 원숭이·가젤·들소와 달리기 모션.
// 몸통 행렬을 부모로 다리·머리를 계층 변환한다(실습 23 RobotModel/DrawRobot 방식).
// TODO(장수현): 실루엣 다듬기(뿔, 꼬리, 혹), 색 확정.
#pragma once
#include "core/GameState.h"
#include "render/Renderer.h"

void DrawAnimal(Renderer &renderer, const Animal &animal, bool hovered);
void DrawMonkey(Renderer &renderer, const glm::vec3 &position, float legPhase);
void DrawObstacle(Renderer &renderer, const Obstacle &obstacle);
void DrawExitMarker(Renderer &renderer, float z, float halfWidth);
void DrawCanyonWalls(Renderer &renderer, float halfWidth, float length);
