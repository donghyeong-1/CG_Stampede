// [김동형] 마우스 피킹: 레이-AABB 교차로 사거리 안에서 가장 가까운 동물을 고른다.
#pragma once
#include "core/GameState.h"
#include "logic/Camera.h"

// slab 방법. 교차하면 true, 레이 시작점부터의 거리를 t에 반환한다.
bool RayIntersectsAabb(const Ray &ray, const glm::vec3 &boxMin, const glm::vec3 &boxMax, float &t);

// 동물의 충돌/피킹 AABB (발바닥 y=0 기준으로 halfSize만큼 위로 올린다).
void AnimalAabb(const Animal &animal, glm::vec3 &boxMin, glm::vec3 &boxMax);

// 탑승 중인 동물을 제외하고, 플레이어 기준 JUMP_RANGE 안에서 레이에 맞는 가장 가까운 동물 id. 없으면 -1.
int PickAnimal(const GameState &state, const Ray &ray);
