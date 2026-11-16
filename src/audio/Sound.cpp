#include "audio/Sound.h"
#include "core/Platform.h"
#include <mmsystem.h>

namespace
{
const unsigned int SAMPLE_RATE = 22050u;
const float RATE = 22050.0f;
const float TWO_PI = 6.2831853f;
bool muted = false;
std::vector<char> waves[static_cast<size_t>(Sfx::Count)]; // 메모리에 만든 WAV 파일(재생 중에도 살아 있어야 한다).

unsigned int noiseSeed = 12345u;
float Noise() // -1 ~ 1.
{
    noiseSeed = noiseSeed * 1664525u + 1013904223u;
    return static_cast<float>((noiseSeed >> 8) & 0xFFFFu) / 32767.5f - 1.0f;
}

// 한 음: 주파수가 from → to로 미끄러지고, 소리 크기는 점점 줄어든다. noise는 잡음 비율(0~1).
void AddTone(std::vector<float> &samples, float start, float duration, float from, float to, float volume, float noise = 0.0f)
{
    size_t first = static_cast<size_t>(start * RATE), count = static_cast<size_t>(duration * RATE);
    if (samples.size() < first + count) { samples.resize(first + count, 0.0f); }
    float phase = 0.0f;
    for (size_t i = 0; i < count; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(count);
        phase += TWO_PI * (from + (to - from) * t) / RATE;
        float envelope = (1.0f - t) * std::min(1.0f, t * 40.0f); // 딱 소리(클릭) 방지용 짧은 시작 구간.
        float wave = std::sin(phase) * (1.0f - noise) + Noise() * noise;
        samples[first + i] += wave * envelope * volume;
    }
}

std::vector<float> Synthesize(Sfx sfx)
{
    std::vector<float> s;
    switch (sfx)
    {
    case Sfx::Start: AddTone(s, 0.0f, 0.1f, 520, 520, 0.5f); AddTone(s, 0.09f, 0.18f, 780, 780, 0.5f); break;
    case Sfx::Jump: AddTone(s, 0.0f, 0.18f, 380, 820, 0.5f); break;
    case Sfx::Land: AddTone(s, 0.0f, 0.14f, 150, 60, 0.8f, 0.3f); break;
    case Sfx::Leap: AddTone(s, 0.0f, 0.35f, 300, 950, 0.45f); AddTone(s, 0.0f, 0.3f, 600, 1200, 0.15f, 0.6f); break;
    case Sfx::Charge: AddTone(s, 0.0f, 0.45f, 70, 130, 0.8f, 0.35f); break;
    case Sfx::RockBreak: AddTone(s, 0.0f, 0.6f, 90, 40, 0.9f, 0.75f); AddTone(s, 0.05f, 0.3f, 200, 80, 0.4f, 0.5f); break;
    case Sfx::LogBreak: AddTone(s, 0.0f, 0.3f, 220, 90, 0.7f, 0.6f); break;
    case Sfx::Hurt: AddTone(s, 0.0f, 0.16f, 520, 300, 0.6f, 0.15f); AddTone(s, 0.14f, 0.28f, 300, 120, 0.6f, 0.15f); break;
    case Sfx::Clear: // 도-미-솔-높은 도.
        AddTone(s, 0.0f, 0.16f, 523, 523, 0.45f); AddTone(s, 0.14f, 0.16f, 659, 659, 0.45f);
        AddTone(s, 0.28f, 0.16f, 784, 784, 0.45f); AddTone(s, 0.42f, 0.5f, 1047, 1047, 0.5f);
        break;
    case Sfx::Fail: AddTone(s, 0.0f, 0.25f, 392, 370, 0.5f); AddTone(s, 0.22f, 0.25f, 330, 311, 0.5f); AddTone(s, 0.44f, 0.6f, 262, 196, 0.55f); break;
    case Sfx::Pause: AddTone(s, 0.0f, 0.08f, 660, 660, 0.4f); break;
    default: break;
    }
    return s;
}

void Put(std::vector<char> &out, unsigned int value, int bytes)
{
    for (int i = 0; i < bytes; ++i) { out.push_back(static_cast<char>((value >> (8 * i)) & 0xFFu)); }
}

// 16비트 모노 PCM WAV(RIFF) 형식으로 포장한다.
std::vector<char> MakeWav(const std::vector<float> &samples)
{
    std::vector<char> wav;
    unsigned int dataBytes = static_cast<unsigned int>(samples.size() * 2);
    wav.insert(wav.end(), {'R', 'I', 'F', 'F'});
    Put(wav, 36 + dataBytes, 4);
    wav.insert(wav.end(), {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '});
    Put(wav, 16, 4);              // fmt 청크 크기.
    Put(wav, 1, 2);               // PCM.
    Put(wav, 1, 2);               // 채널 1개.
    Put(wav, SAMPLE_RATE, 4);
    Put(wav, SAMPLE_RATE * 2, 4); // 초당 바이트.
    Put(wav, 2, 2);               // 샘플 하나의 바이트.
    Put(wav, 16, 2);              // 비트 수.
    wav.insert(wav.end(), {'d', 'a', 't', 'a'});
    Put(wav, dataBytes, 4);
    for (float sample : samples)
    {
        int value = static_cast<int>(std::clamp(sample, -1.0f, 1.0f) * 32000.0f);
        Put(wav, static_cast<unsigned int>(value) & 0xFFFFu, 2);
    }
    return wav;
}
} // namespace

void SetSoundMuted(bool value)
{
    muted = value;
}

void PlaySfx(Sfx sfx)
{
    if (muted || sfx == Sfx::Count) { return; }
    std::vector<char> &wav = waves[static_cast<size_t>(sfx)];
    if (wav.empty()) { wav = MakeWav(Synthesize(sfx)); } // 처음 쓸 때 한 번만 만든다.
    PlaySoundA(wav.data(), nullptr, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
}
