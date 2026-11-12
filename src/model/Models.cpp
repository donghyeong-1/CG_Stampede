#include "model/Models.h"

namespace
{
// 달리기: 대각선 다리가 같은 위상으로 움직인다. 피벗(어깨/엉덩이)에서 x축 회전.
void DrawLeg(Renderer &renderer, const glm::mat4 &body, const glm::vec3 &pivot, float length,
             float thickness, float angleDeg, const glm::vec3 &color)
{
    glm::mat4 leg = body * glm::translate(glm::mat4(1.0f), pivot) *
                    glm::rotate(glm::mat4(1.0f), glm::radians(angleDeg), glm::vec3(1, 0, 0));
    renderer.DrawCube(leg, {0, -length * 0.5f, 0}, {thickness, length, thickness}, color);
}
} // namespace

void DrawAnimal(Renderer &renderer, const Animal &animal, bool hovered)
{
    const bool gazelle = animal.type == AnimalType::Gazelle;
    const glm::vec3 bodyColor = gazelle ? glm::vec3(0.85f, 0.65f, 0.3f) : glm::vec3(0.4f, 0.25f, 0.18f);
    const glm::vec3 legColor = gazelle ? glm::vec3(0.7f, 0.5f, 0.22f) : glm::vec3(0.3f, 0.18f, 0.12f);
    const float highlight = hovered ? 1.0f : 0.0f;

    // 가젤: 날씬하고 다리가 길다 / 들소: 넓은 몸통, 짧고 굵은 다리, 어깨 혹.
    const float legLength = gazelle ? 0.75f : 0.5f;
    const float legThickness = gazelle ? 0.14f : 0.26f;
    const glm::vec3 bodySize = gazelle ? glm::vec3(0.55f, 0.5f, 1.3f) : glm::vec3(1.0f, 0.8f, 1.8f);
    const float bodyY = legLength + bodySize.y * 0.5f;

    glm::mat4 model = glm::translate(glm::mat4(1.0f), animal.position);
    renderer.DrawCube(model, {0, bodyY, 0}, bodySize, bodyColor, highlight);
    renderer.DrawCube(model, {0, bodyY + bodySize.y * 0.35f, bodySize.z * 0.5f + 0.15f},
                      gazelle ? glm::vec3(0.3f, 0.35f, 0.5f) : glm::vec3(0.6f, 0.6f, 0.6f), bodyColor, highlight);
    if (!gazelle)
    {
        renderer.DrawCube(model, {0, bodyY + bodySize.y * 0.5f + 0.15f, bodySize.z * 0.25f}, {0.7f, 0.3f, 0.6f},
                          bodyColor, highlight); // 혹.
    }

    const float swing = std::sin(animal.legPhase) * 35.0f;
    const float x = bodySize.x * 0.5f - legThickness * 0.5f;
    const float z = bodySize.z * 0.5f - legThickness * 0.5f;
    DrawLeg(renderer, model, {-x, legLength, z}, legLength, legThickness, swing, legColor);
    DrawLeg(renderer, model, {x, legLength, z}, legLength, legThickness, -swing, legColor);
    DrawLeg(renderer, model, {-x, legLength, -z}, legLength, legThickness, -swing, legColor);
    DrawLeg(renderer, model, {x, legLength, -z}, legLength, legThickness, swing, legColor);
}

void DrawMonkey(Renderer &renderer, const glm::vec3 &position, float legPhase)
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    const glm::vec3 fur(0.55f, 0.35f, 0.2f);
    renderer.DrawCube(model, {0, 0.3f, 0}, {0.3f, 0.35f, 0.25f}, fur);
    renderer.DrawCube(model, {0, 0.6f, 0.05f}, {0.26f, 0.24f, 0.24f}, fur);
    renderer.DrawCube(model, {0, 0.58f, 0.18f}, {0.14f, 0.12f, 0.06f}, {0.9f, 0.75f, 0.6f});
    renderer.DrawCube(model, {0, 0.25f, -0.25f}, {0.06f, 0.06f, 0.35f}, fur); // 꼬리.
    const float swing = std::sin(legPhase) * 20.0f;
    DrawLeg(renderer, model, {-0.08f, 0.15f, 0}, 0.15f, 0.08f, swing, fur);
    DrawLeg(renderer, model, {0.08f, 0.15f, 0}, 0.15f, 0.08f, -swing, fur);
}

void DrawObstacle(Renderer &renderer, const Obstacle &obstacle)
{
    glm::mat4 identity(1.0f);
    if (obstacle.type == ObstacleType::Crack)
    {
        // 균열: 바닥 위에 얇게 깔린 어두운 면. TODO(장수현): 깊이감 있는 표현.
        renderer.DrawCube(identity, {obstacle.position.x, 0.02f, obstacle.position.z},
                          {obstacle.halfSize.x * 2, 0.04f, obstacle.halfSize.z * 2}, {0.05f, 0.03f, 0.03f});
    }
    else
    {
        renderer.DrawCube(identity, {obstacle.position.x, obstacle.halfSize.y, obstacle.position.z},
                          obstacle.halfSize * 2.0f, {0.45f, 0.45f, 0.5f});
    }
}

void DrawExitMarker(Renderer &renderer, float z, float halfWidth)
{
    glm::mat4 identity(1.0f);
    const glm::vec3 color(0.3f, 0.9f, 0.4f);
    renderer.DrawCube(identity, {-halfWidth, 2.0f, z}, {0.6f, 4.0f, 0.6f}, color);
    renderer.DrawCube(identity, {halfWidth, 2.0f, z}, {0.6f, 4.0f, 0.6f}, color);
    renderer.DrawCube(identity, {0, 4.0f, z}, {halfWidth * 2 + 0.6f, 0.6f, 0.6f}, color);
}

void DrawCanyonWalls(Renderer &renderer, float halfWidth, float length)
{
    glm::mat4 identity(1.0f);
    const glm::vec3 color(0.55f, 0.32f, 0.22f);
    for (float side : {-1.0f, 1.0f})
    {
        renderer.DrawCube(identity, {side * (halfWidth + 1.5f), 3.0f, length * 0.5f}, {3.0f, 6.0f, length + 10.0f}, color);
    }
}
