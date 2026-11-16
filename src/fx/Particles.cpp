#include "fx/Particles.h"

namespace
{
const size_t MAX_PARTICLES = 3000;
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

void ParticleSystem::Add(const Particle &particle)
{
    if (particles.size() < MAX_PARTICLES) { particles.push_back(particle); }
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
    // 반투명(연기·먼지)을 먼저, 빛나는 입자(불꽃·불티)를 나중에 그린다.
    for (int pass = 0; pass < 2; ++pass)
    {
        bool additive = pass == 1;
        renderer.BeginParticles(additive);
        for (const Particle &p : particles)
        {
            if (p.additive != additive) { continue; }
            float t = 1.0f - p.life / p.maxLife; // 0(막 생김) → 1(사라짐).
            glm::vec4 color = glm::mix(p.colorStart, p.colorEnd, t);
            color.a *= std::min(1.0f, t * 8.0f); // 갑자기 나타나지 않게 처음에 살짝 서서히.
            renderer.DrawParticle(p.position, glm::mix(p.sizeStart, p.sizeEnd, t), color);
        }
    }
    renderer.EndParticles();
}

void ParticleSystem::EmitFire(const glm::vec3 &position, float scale, float dt)
{
    // 불꽃: 노란 중심에서 시작해 위로 오르며 붉어지고 작아진다.
    for (int i = CountFor(38.0f, dt); i > 0; --i)
    {
        Particle p;
        p.position = position + glm::vec3(Random(-0.3f, 0.3f), Random(0.0f, 0.3f), Random(-0.3f, 0.3f)) * scale;
        p.velocity = glm::vec3(Random(-0.3f, 0.3f), Random(1.4f, 2.8f), Random(-0.3f, 0.3f)) * scale;
        p.life = p.maxLife = Random(0.45f, 0.9f);
        p.sizeStart = Random(0.7f, 1.1f) * scale;
        p.sizeEnd = 0.12f * scale;
        p.colorStart = {1.0f, 0.8f, 0.3f, 0.9f};
        p.colorEnd = {0.9f, 0.12f, 0.02f, 0.0f};
        p.lift = 1.5f;
        p.additive = true;
        Add(p);
    }
    // 연기: 불꽃 위에서 천천히 커지며 바람에 흘러간다.
    for (int i = CountFor(5.0f, dt); i > 0; --i)
    {
        Particle p;
        p.position = position + glm::vec3(Random(-0.2f, 0.2f), Random(0.6f, 1.2f), Random(-0.2f, 0.2f)) * scale;
        p.velocity = glm::vec3(Random(-0.2f, 0.2f), Random(1.0f, 1.8f), Random(-0.2f, 0.2f)) + WIND;
        p.life = p.maxLife = Random(2.0f, 3.2f);
        p.sizeStart = 0.5f * scale;
        p.sizeEnd = Random(1.8f, 2.6f) * scale;
        p.colorStart = {0.06f, 0.05f, 0.05f, 0.5f};
        p.colorEnd = {0.2f, 0.17f, 0.17f, 0.0f};
        p.drag = 0.2f;
        Add(p);
    }
}

void ParticleSystem::EmitEmbers(const glm::vec3 &center, float dt)
{
    for (int i = CountFor(14.0f, dt); i > 0; --i)
    {
        Particle p;
        p.position = center + glm::vec3(Random(-7.0f, 7.0f), Random(0.5f, 7.0f), Random(-6.0f, 34.0f));
        p.velocity = glm::vec3(Random(-0.5f, 0.5f), Random(0.3f, 1.4f), Random(-0.5f, 0.5f)) + WIND * 2.0f;
        p.life = p.maxLife = Random(1.8f, 3.5f);
        p.sizeStart = Random(0.07f, 0.13f);
        p.sizeEnd = 0.03f;
        p.colorStart = {1.0f, 0.65f, 0.2f, 1.0f};
        p.colorEnd = {1.0f, 0.2f, 0.0f, 0.0f};
        p.additive = true;
        Add(p);
    }
}

void ParticleSystem::EmitDust(const glm::vec3 &position, float perSecond, float dt)
{
    for (int i = CountFor(perSecond, dt); i > 0; --i)
    {
        Particle p;
        p.position = position + glm::vec3(Random(-0.3f, 0.3f), Random(0.05f, 0.2f), Random(-0.2f, 0.2f));
        p.velocity = glm::vec3(Random(-0.6f, 0.6f), Random(0.5f, 1.2f), Random(-1.6f, -0.4f)); // 뒤쪽 위로 피어오른다.
        p.life = p.maxLife = Random(0.8f, 1.5f);
        p.sizeStart = Random(0.3f, 0.5f);
        p.sizeEnd = Random(1.2f, 1.9f);
        p.colorStart = {0.96f, 0.88f, 0.74f, 0.6f};
        p.colorEnd = {0.92f, 0.82f, 0.68f, 0.0f};
        p.lift = -0.6f;
        p.drag = 1.5f;
        Add(p);
    }
}

void ParticleSystem::BurstDust(const glm::vec3 &position, int count, float power)
{
    for (int i = 0; i < count; ++i)
    {
        float angle = Random(0.0f, 6.2831853f);
        Particle p;
        p.position = position + glm::vec3(std::cos(angle), 0.0f, std::sin(angle)) * Random(0.0f, 0.4f) * power;
        p.position.y = Random(0.05f, 0.4f * power);
        p.velocity = glm::vec3(std::cos(angle) * Random(1.0f, 3.0f), Random(0.6f, 2.2f), std::sin(angle) * Random(1.0f, 3.0f)) * power;
        p.life = p.maxLife = Random(0.6f, 1.3f);
        p.sizeStart = Random(0.3f, 0.5f) * power;
        p.sizeEnd = Random(1.0f, 1.6f) * power;
        p.colorStart = {0.96f, 0.88f, 0.74f, 0.7f};
        p.colorEnd = {0.92f, 0.82f, 0.68f, 0.0f};
        p.lift = -0.8f;
        p.drag = 2.0f;
        Add(p);
    }
}
