#include "logic/Picking.h"

bool RayIntersectsAabb(const Ray &ray, const glm::vec3 &boxMin, const glm::vec3 &boxMax, float &t)
{
    float tMin = 0.0f;
    float tMax = 1.0e9f;
    for (int axis = 0; axis < 3; ++axis)
    {
        if (std::abs(ray.direction[axis]) < 1.0e-8f)
        {
            // 축과 평행한 레이: 시작점이 슬랩 밖이면 교차하지 않는다.
            if (ray.origin[axis] < boxMin[axis] || ray.origin[axis] > boxMax[axis]) { return false; }
            continue;
        }
        float inverse = 1.0f / ray.direction[axis];
        float t0 = (boxMin[axis] - ray.origin[axis]) * inverse;
        float t1 = (boxMax[axis] - ray.origin[axis]) * inverse;
        if (t0 > t1) { std::swap(t0, t1); }
        tMin = std::max(tMin, t0);
        tMax = std::min(tMax, t1);
        if (tMin > tMax) { return false; }
    }
    t = tMin;
    return true;
}

void AnimalAabb(const Animal &animal, glm::vec3 &boxMin, glm::vec3 &boxMax)
{
    glm::vec3 center = animal.position + glm::vec3(0.0f, animal.halfSize.y, 0.0f);
    boxMin = center - animal.halfSize;
    boxMax = center + animal.halfSize;
}

int PickAnimal(const GameState &state, const Ray &ray)
{
    int best = -1;
    float bestT = 1.0e9f;
    for (const Animal &animal : state.animals)
    {
        if (animal.id == state.player.mountedAnimalId) { continue; }
        if (glm::distance(animal.position, state.player.position) > cfg::JUMP_RANGE) { continue; }
        glm::vec3 boxMin, boxMax;
        AnimalAabb(animal, boxMin, boxMax);
        float t = 0.0f;
        if (RayIntersectsAabb(ray, boxMin, boxMax, t) && t < bestT)
        {
            bestT = t;
            best = animal.id;
        }
    }
    return best;
}
