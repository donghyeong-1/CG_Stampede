#include "fx/Particles.h"

namespace
{
const size_t MAX_PARTICLES = 1500;
const glm::vec3 WIND(0.5f, 0.0f, 0.3f); // 연기와 불티가 흘러가는 방향.
} // namespace

float ParticleSystem::Random()
{
    seed = seed * 1664525u + 1013904223u; // 선형 합동 난수.
    return static_cast<float>((seed >> 8) & 0xFFFFu) / 65535.0f;
}

float ParticleSystem::Random(float low, float high)
{
    return low + (high - low) * Random();
}

int ParticleSystem::CountFor(float perSecond, float dt)
{
    float expected = perSecond * dt;
    int count = static_cast<int>(expected);
    if (Random() < expected - static_cast<float>(count)) { ++count; }
    return count;
}

void ParticleSystem::Add(Particle particle)
{
    if (particles.size() >= MAX_PARTICLES) { return; }
    // 조각마다 다른 축·속도로 돌게 해서 블록이 구르는 느낌을 준다.
    particle.spinAxis = glm::normalize(glm::vec3(Random(-1.0f, 1.0f), Random(-1.0f, 1.0f), Random(-1.0f, 1.0f)) + glm::vec3(0.01f));
    particle.spinSpeed = Random(120.0f, 420.0f);
    particles.push_back(particle);
}

void ParticleSystem::Update(float dt)
{
    for (Particle &p : particles)
    {
        p.life -= dt;
        p.velocity.y += p.lift * dt;
        p.velocity *= std::max(0.0f, 1.0f - p.drag * dt);
        p.position += p.velocity * dt;
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle &p) { return p.life <= 0.0f; }),
                    particles.end());
}

void ParticleSystem::Draw(Renderer &renderer) const
{
    const glm::mat4 identity(1.0f);
    for (const Particle &p : particles)
    {
        float t = 1.0f - p.life / p.maxLife; // 0(막 생김) → 1(사라짐).
        // 반투명 대신 크기로 나타나고 사라진다: 처음에 빠르게 커지고 마지막 30% 동안 0으로 줄어든다.
        float fade = std::min(std::min(1.0f, t * 10.0f), std::min(1.0f, (1.0f - t) / 0.3f));
        float size = glm::mix(p.sizeStart, p.sizeEnd, t) * fade;
        if (size <= 0.005f) { continue; }
        glm::vec3 color = glm::mix(p.colorStart, p.colorEnd, t);
        glm::mat4 model = glm::translate(identity, p.position) *
                          glm::rotate(identity, glm::radians(p.spinSpeed * (p.maxLife - p.life)), p.spinAxis);
        if (p.glow) { renderer.DrawGlowCube(model, glm::vec3(0.0f), glm::vec3(size), color); }
        else { renderer.DrawCube(model, glm::vec3(0.0f), glm::vec3(size), color); }
    }
}

void ParticleSystem::EmitFire(const glm::vec3 &position, float scale, float dt)
{
    // 불꽃: 노란 블록이 위로 오르며 주황 → 빨강으로 바뀌고 작아진다.
    for (int i = CountFor(16.0f, dt); i > 0; --i)
    {
        Particle p;
        p.position = position + glm::vec3(Random(-0.3f, 0.3f), Random(0.0f, 0.3f), Random(-0.3f, 0.3f)) * scale;
        p.velocity = glm::vec3(Random(-0.3f, 0.3f), Random(1.4f, 2.6f), Random(-0.3f, 0.3f)) * scale;
        p.life = p.maxLife = Random(0.5f, 0.95f);
        p.sizeStart = Random(0.22f, 0.36f) * scale;
        p.sizeEnd = 0.08f * scale;
        p.colorStart = {1.0f, 0.86f, 0.3f};
        p.colorEnd = {0.85f, 0.12f, 0.03f};
        p.lift = 1.2f;
        p.glow = true;
        Add(p);
    }
    // 연기: 검은 블록이 불꽃 위에서 천천히 커지며 바람에 흘러간다.
    for (int i = CountFor(2.5f, dt); i > 0; --i)
    {
        Particle p;
        p.position = position + glm::vec3(Random(-0.2f, 0.2f), Random(0.7f, 1.3f), Random(-0.2f, 0.2f)) * scale;
        p.velocity = glm::vec3(Random(-0.2f, 0.2f), Random(1.0f, 1.7f), Random(-0.2f, 0.2f)) + WIND;
        p.life = p.maxLife = Random(1.6f, 2.6f);
        p.sizeStart = 0.16f * scale;
        p.sizeEnd = Random(0.3f, 0.45f) * scale;
        p.colorStart = {0.1f, 0.08f, 0.08f};
        p.colorEnd = {0.3f, 0.27f, 0.26f};
        p.drag = 0.2f;
        Add(p);
    }
}

void ParticleSystem::EmitEmbers(const glm::vec3 &center, float dt)
{
    for (int i = CountFor(10.0f, dt); i > 0; --i)
    {
        Particle p;
        p.position = center + glm::vec3(Random(-7.0f, 7.0f), Random(0.5f, 7.0f), Random(-6.0f, 34.0f));
        p.velocity = glm::vec3(Random(-0.5f, 0.5f), Random(0.3f, 1.4f), Random(-0.5f, 0.5f)) + WIND * 2.0f;
        p.life = p.maxLife = Random(1.8f, 3.2f);
        p.sizeStart = Random(0.05f, 0.09f);
        p.sizeEnd = 0.03f;
        p.colorStart = {1.0f, 0.7f, 0.25f};
        p.colorEnd = {1.0f, 0.2f, 0.0f};
        p.glow = true;
        Add(p);
    }
}

void ParticleSystem::EmitDust(const glm::vec3 &position, float perSecond, float dt)
{
    for (int i = CountFor(perSecond, dt); i > 0; --i)
    {
        Particle p;
        p.position = position + glm::vec3(Random(-0.3f, 0.3f), Random(0.05f, 0.15f), Random(-0.2f, 0.2f));
        p.velocity = glm::vec3(Random(-0.8f, 0.8f), Random(1.0f, 2.2f), Random(-1.8f, -0.4f)); // 뒤쪽 위로 튄다.
        p.life = p.maxLife = Random(0.45f, 0.85f);
        p.sizeStart = Random(0.1f, 0.17f);
        p.sizeEnd = Random(0.14f, 0.22f);
        p.colorStart = {0.97f, 0.9f, 0.76f}; // 바닥보다 밝은 색이어야 눈에 띈다.
        p.colorEnd = {0.86f, 0.74f, 0.58f};
        p.lift = -4.5f; // 흙 조각은 튀었다가 다시 떨어진다.
        p.drag = 0.8f;
        Add(p);
    }
}

void ParticleSystem::BurstDust(const glm::vec3 &position, int count, float power)
{
    for (int i = 0; i < count; ++i)
    {
        float angle = Random(0.0f, 6.2831853f);
        glm::vec3 outward(std::cos(angle), 0.0f, std::sin(angle));
        Particle p;
        p.position = position + outward * Random(0.0f, 0.4f) * power;
        p.position.y = Random(0.05f, 0.3f * power);
        p.velocity = (outward * Random(1.0f, 3.2f) + glm::vec3(0.0f, Random(1.5f, 3.6f), 0.0f)) * power;
        p.life = p.maxLife = Random(0.5f, 1.1f);
        p.sizeStart = Random(0.12f, 0.2f) * power;
        p.sizeEnd = Random(0.16f, 0.26f) * power;
        p.colorStart = {0.97f, 0.9f, 0.76f};
        p.colorEnd = {0.84f, 0.72f, 0.56f};
        p.lift = -6.0f;
        p.drag = 1.0f;
        Add(p);
    }
}
