#include "model/Models.h"

namespace
{
const glm::mat4 IDENTITY(1.0f);

// parent 좌표계의 pivot으로 옮긴 뒤 축을 중심으로 회전한 좌표계.
glm::mat4 Joint(const glm::mat4 &parent, const glm::vec3 &pivot, float angleDeg, const glm::vec3 &axis)
{
    return parent * glm::translate(IDENTITY, pivot) * glm::rotate(IDENTITY, glm::radians(angleDeg), axis);
}

// 피벗(어깨/엉덩이)에서 아래로 뻗은 팔다리. 양의 각도는 뒤쪽(-Z)으로, 음의 각도는 앞쪽(+Z)으로 흔든다.
void DrawLimb(Renderer &renderer, const glm::mat4 &body, const glm::vec3 &pivot, float length, float thickness,
              float angleDeg, const glm::vec3 &color, float highlight = 0.0f)
{
    glm::mat4 limb = Joint(body, pivot, angleDeg, {1, 0, 0});
    renderer.DrawCube(limb, {0, -length * 0.5f, 0}, {thickness, length, thickness}, color, highlight);
}

// 정수에서 0~1 난수 비슷한 값을 만든다(매 프레임 같은 모양을 그리기 위한 고정 무늬).
float Hash(int n)
{
    float s = std::sin(static_cast<float>(n) * 12.9898f) * 43758.5453f;
    return s - std::floor(s);
}

void DrawGazelle(Renderer &renderer, const glm::mat4 &model, float legPhase, float highlight)
{
    const glm::vec3 coat(0.86f, 0.62f, 0.3f), dark(0.35f, 0.2f, 0.1f), cream(0.95f, 0.9f, 0.8f), horn(0.2f, 0.16f, 0.14f);
    const float legLength = 0.75f, legThickness = 0.13f;
    const glm::vec3 bodySize(0.55f, 0.5f, 1.3f);
    const float bodyY = legLength + bodySize.y * 0.5f;

    renderer.DrawCube(model, {0, bodyY, 0}, bodySize, coat, highlight);
    for (float side : {-1.0f, 1.0f}) // 옆구리의 짙은 줄무늬.
    {
        renderer.DrawCube(model, {side * 0.28f, bodyY - 0.08f, 0}, {0.02f, 0.12f, 1.0f}, dark, highlight);
    }
    renderer.DrawCube(model, {0, bodyY + 0.02f, -0.66f}, {0.4f, 0.3f, 0.04f}, cream, highlight); // 흰 엉덩이.
    renderer.DrawCube(model, {0, bodyY + 0.12f, -0.72f}, {0.07f, 0.2f, 0.07f}, dark, highlight);  // 꼬리.

    // 목 → 머리 → 뿔: 목을 앞으로 기울이고, 머리는 다시 수평으로 되돌린다.
    glm::mat4 neck = Joint(model, {0, bodyY + 0.15f, 0.55f}, 35.0f, {1, 0, 0});
    renderer.DrawCube(neck, {0, 0.3f, 0}, {0.2f, 0.6f, 0.2f}, coat, highlight);
    glm::mat4 head = Joint(neck, {0, 0.62f, 0}, -35.0f, {1, 0, 0});
    renderer.DrawCube(head, {0, 0.02f, 0.1f}, {0.22f, 0.22f, 0.42f}, coat, highlight);
    renderer.DrawCube(head, {0, -0.02f, 0.3f}, {0.16f, 0.12f, 0.06f}, dark, highlight); // 코.
    for (float side : {-1.0f, 1.0f})
    {
        renderer.DrawCube(head, {side * 0.15f, 0.12f, -0.06f}, {0.1f, 0.12f, 0.04f}, coat, highlight); // 귀.
        glm::mat4 hornBase = Joint(head, {side * 0.07f, 0.12f, -0.02f}, -18.0f, {1, 0, 0});
        renderer.DrawCube(hornBase, {0, 0.24f, 0}, {0.045f, 0.48f, 0.045f}, horn, highlight); // 가늘고 긴 뿔.
    }

    // 달리기: 대각선 다리가 같은 위상.
    const float swing = std::sin(legPhase) * 38.0f;
    const float x = bodySize.x * 0.5f - legThickness * 0.5f, z = bodySize.z * 0.5f - legThickness * 0.5f;
    DrawLimb(renderer, model, {-x, legLength, z}, legLength, legThickness, swing, coat, highlight);
    DrawLimb(renderer, model, {x, legLength, z}, legLength, legThickness, -swing, coat, highlight);
    DrawLimb(renderer, model, {-x, legLength, -z}, legLength, legThickness, -swing, coat, highlight);
    DrawLimb(renderer, model, {x, legLength, -z}, legLength, legThickness, swing, coat, highlight);
}

void DrawBison(Renderer &renderer, const glm::mat4 &model, float legPhase, float highlight)
{
    const glm::vec3 coat(0.42f, 0.27f, 0.18f), mane(0.24f, 0.15f, 0.1f), horn(0.92f, 0.88f, 0.78f);
    const float legLength = 0.5f, legThickness = 0.26f;
    const glm::vec3 bodySize(1.0f, 0.8f, 1.8f);
    const float bodyY = legLength + bodySize.y * 0.5f;

    renderer.DrawCube(model, {0, bodyY, 0}, bodySize, coat, highlight);
    renderer.DrawCube(model, {0, bodyY + 0.45f, 0.52f}, {0.92f, 0.5f, 0.86f}, mane, highlight);   // 어깨 혹.
    renderer.DrawCube(model, {0, bodyY - 0.05f, 1.05f}, {0.62f, 0.62f, 0.5f}, mane, highlight);   // 낮고 큰 머리.
    renderer.DrawCube(model, {0, bodyY - 0.42f, 1.12f}, {0.24f, 0.24f, 0.2f}, mane, highlight);   // 턱수염.
    renderer.DrawCube(model, {0, bodyY - 0.12f, 1.31f}, {0.34f, 0.2f, 0.04f}, {0.12f, 0.08f, 0.06f}, highlight); // 코.
    for (float side : {-1.0f, 1.0f}) // 옆으로 뻗었다가 위로 꺾이는 뿔.
    {
        renderer.DrawCube(model, {side * 0.4f, bodyY + 0.2f, 1.05f}, {0.2f, 0.09f, 0.09f}, horn, highlight);
        renderer.DrawCube(model, {side * 0.47f, bodyY + 0.31f, 1.05f}, {0.08f, 0.2f, 0.08f}, horn, highlight);
    }
    renderer.DrawCube(model, {0, bodyY, -0.94f}, {0.08f, 0.5f, 0.08f}, mane, highlight); // 꼬리.

    const float swing = std::sin(legPhase) * 30.0f;
    const float x = bodySize.x * 0.5f - legThickness * 0.5f, z = bodySize.z * 0.5f - legThickness * 0.5f;
    DrawLimb(renderer, model, {-x, legLength, z}, legLength, legThickness, swing, mane, highlight);
    DrawLimb(renderer, model, {x, legLength, z}, legLength, legThickness, -swing, mane, highlight);
    DrawLimb(renderer, model, {-x, legLength, -z}, legLength, legThickness, -swing, coat, highlight);
    DrawLimb(renderer, model, {x, legLength, -z}, legLength, legThickness, swing, coat, highlight);
}
} // namespace

void DrawAnimal(Renderer &renderer, const Animal &animal, bool hovered, bool boosted)
{
    if (animal.yOffset < -6.0f) { return; } // 균열 아래로 떨어져 보이지 않는다.
    float highlight = hovered ? 1.0f : (boosted ? 0.55f : 0.0f);
    glm::mat4 model = glm::translate(IDENTITY, animal.position + glm::vec3(0.0f, animal.yOffset, 0.0f));
    if (boosted && animal.type == AnimalType::Bison)
    {
        model = model * glm::rotate(IDENTITY, glm::radians(6.0f), glm::vec3(1, 0, 0)); // 돌진: 머리를 숙인다.
    }
    float legPhase = animal.waiting ? 0.0f : animal.legPhase;
    if (animal.type == AnimalType::Gazelle) { DrawGazelle(renderer, model, legPhase, highlight); }
    else { DrawBison(renderer, model, legPhase, highlight); }
}

void DrawMonkey(Renderer &renderer, const glm::vec3 &position, float time, bool jumping)
{
    const glm::vec3 fur(0.5f, 0.3f, 0.16f), skin(0.93f, 0.76f, 0.6f), shirt(0.85f, 0.2f, 0.15f);
    glm::mat4 model = glm::translate(IDENTITY, position);
    renderer.DrawCube(model, {0, 0.24f, 0}, {0.3f, 0.36f, 0.24f}, shirt);        // 몸통(눈에 띄는 빨간 조끼).
    renderer.DrawCube(model, {0, 0.55f, 0.03f}, {0.3f, 0.27f, 0.27f}, fur);       // 머리.
    renderer.DrawCube(model, {0, 0.52f, 0.17f}, {0.2f, 0.16f, 0.04f}, skin);      // 얼굴.
    for (float side : {-1.0f, 1.0f})
    {
        renderer.DrawCube(model, {side * 0.18f, 0.57f, 0.02f}, {0.07f, 0.11f, 0.05f}, skin); // 귀.
    }
    renderer.DrawCube(model, {0, 0.14f, -0.24f}, {0.06f, 0.06f, 0.28f}, fur);     // 꼬리.
    renderer.DrawCube(model, {0, 0.3f, -0.36f}, {0.06f, 0.32f, 0.06f}, fur);

    // 탑승 중: 팔다리를 앞으로 뻗어 동물을 붙잡는다. 점프 중: 팔을 들고 다리를 흔든다.
    float wobble = std::sin(time * 12.0f) * 8.0f;
    float arm = jumping ? -150.0f : -55.0f + wobble;
    float leg = jumping ? std::sin(time * 18.0f) * 30.0f : -70.0f;
    for (float side : {-1.0f, 1.0f})
    {
        DrawLimb(renderer, model, {side * 0.19f, 0.38f, 0}, 0.28f, 0.08f, arm, fur);
        DrawLimb(renderer, model, {side * 0.09f, 0.1f, 0}, 0.24f, 0.09f, jumping ? leg * side : leg, fur);
    }
}

void DrawRockWall(Renderer &renderer, const Obstacle &obstacle, float time)
{
    const int count = 7;
    float elapsed = obstacle.consumed ? time - obstacle.breakTime : 0.0f;
    if (obstacle.consumed && elapsed > 1.4f) { return; }
    float width = obstacle.halfSize.x * 2.0f / count;
    for (int i = 0; i < count; ++i)
    {
        float x = obstacle.position.x - obstacle.halfSize.x + (static_cast<float>(i) + 0.5f) * width;
        float height = obstacle.halfSize.y * (1.4f + Hash(i * 3 + 1) * 0.9f);
        glm::vec3 size(width * (1.05f + Hash(i * 3 + 2) * 0.3f), height, obstacle.halfSize.z * (1.6f + Hash(i * 3) * 0.6f));
        glm::vec3 center(x, height * 0.5f, obstacle.position.z);
        float gray = 0.4f + Hash(i + 20) * 0.14f;
        float yaw = (Hash(i + 40) - 0.5f) * 40.0f, spin = 0.0f;
        if (obstacle.consumed)
        {
            // 부서진 바위: 진행 방향으로 튕겨 나가며 회전하고, 중력으로 떨어지면서 작아진다.
            glm::vec3 velocity((Hash(i + 60) - 0.5f) * 7.0f, 4.0f + Hash(i + 80) * 3.0f, 5.0f + Hash(i + 100) * 5.0f);
            center += velocity * elapsed + glm::vec3(0.0f, -0.5f * 9.8f * elapsed * elapsed, 0.0f);
            size *= std::max(0.0f, 1.0f - elapsed / 1.4f);
            spin = elapsed * (200.0f + Hash(i + 120) * 300.0f);
        }
        glm::mat4 block = Joint(IDENTITY, center, yaw, {0, 1, 0}) * glm::rotate(IDENTITY, glm::radians(spin), glm::vec3(1, 0, 0));
        renderer.DrawCube(block, {0, 0, 0}, size, {gray, gray * 0.97f, gray * 1.05f});
    }
}

void DrawCrackPit(Renderer &renderer, const Obstacle &obstacle, float halfWidth)
{
    const glm::vec3 cliff(0.3f, 0.17f, 0.12f);
    const float depth = 5.0f, width = halfWidth * 2.0f;
    float front = obstacle.position.z - obstacle.halfSize.z, back = obstacle.position.z + obstacle.halfSize.z;
    // 바닥과 겹쳐 깜빡이지 않도록 절벽 윗면을 바닥보다 살짝 낮춘다.
    renderer.DrawCube(IDENTITY, {0, -depth * 0.5f - 0.01f, front - 0.5f}, {width, depth, 1.0f}, cliff);
    renderer.DrawCube(IDENTITY, {0, -depth * 0.5f - 0.01f, back + 0.5f}, {width, depth, 1.0f}, cliff);
    // 바닥 깊은 곳의 불빛: 조명과 무관하게 빛난다.
    renderer.DrawGlowCube(IDENTITY, {0, -depth + 0.4f, obstacle.position.z}, {width, 0.2f, obstacle.halfSize.z * 2.0f},
                          {0.95f, 0.38f, 0.08f});
    renderer.DrawGlowCube(IDENTITY, {0, -depth + 0.55f, obstacle.position.z}, {width, 0.1f, obstacle.halfSize.z * 0.7f},
                          {1.0f, 0.75f, 0.25f});
}

void DrawExitMarker(Renderer &renderer, float z, float halfWidth)
{
    const glm::vec3 color(0.3f, 0.9f, 0.4f);
    renderer.DrawCube(IDENTITY, {-halfWidth, 2.5f, z}, {0.7f, 5.0f, 0.7f}, color);
    renderer.DrawCube(IDENTITY, {halfWidth, 2.5f, z}, {0.7f, 5.0f, 0.7f}, color);
    renderer.DrawCube(IDENTITY, {0, 5.0f, z}, {halfWidth * 2 + 0.7f, 0.7f, 0.7f}, color);
    renderer.DrawGlowCube(IDENTITY, {0, 0.02f, z}, {halfWidth * 2, 0.04f, 0.4f}, {0.85f, 1.0f, 0.85f}); // 결승선.
}

void DrawCanyonWalls(Renderer &renderer, float halfWidth, float zStart, float zEnd)
{
    // 크기·색이 조금씩 다른 암석 덩어리를 두 층으로 쌓아 절벽을 만든다. 안쪽 면은 바닥 가장자리 밖에 둔다.
    const float step = 5.0f;
    int index = 0;
    for (float z = zStart; z < zEnd; z += step, ++index)
    {
        for (int sideIndex = 0; sideIndex < 2; ++sideIndex)
        {
            float side = sideIndex == 0 ? -1.0f : 1.0f;
            int seed = index * 2 + sideIndex;
            float inset = Hash(seed) * 0.6f, height = 2.2f + Hash(seed + 300) * 1.6f, depth = 4.0f;
            float tone = 0.9f + Hash(seed + 600) * 0.25f;
            glm::vec3 color = glm::vec3(0.56f, 0.32f, 0.21f) * tone;
            float inner = halfWidth + inset; // 절벽 안쪽 면의 |x|.
            // 아래쪽은 균열 깊이까지 내려가 구덩이의 옆면이 된다.
            renderer.DrawCube(IDENTITY, {side * (inner + depth * 0.5f), (height - 5.0f) * 0.5f, z + step * 0.5f},
                              {depth, height + 5.0f, step}, color);
            float upper = 1.0f + Hash(seed + 900) * 1.6f; // 뒤로 물러난 위층. 카메라(높이 약 9m)를 가리지 않게 낮게 둔다.
            renderer.DrawCube(IDENTITY, {side * (inner + 1.2f + depth * 0.5f), height + upper * 0.5f, z + step * 0.5f},
                              {depth, upper, step}, color * 0.9f);
        }
    }
}
