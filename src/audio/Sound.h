// 효과음. 소리 파일 없이 코드로 파형을 만들어(사인파·잡음) Windows의 PlaySound로 재생한다.
// PlaySound는 한 번에 한 소리만 낼 수 있어서, 새 효과음이 나오면 앞의 소리는 끊긴다(배경음은 없다).
#pragma once

enum class Sfx { Start, Jump, Land, Leap, Charge, RockBreak, LogBreak, Hurt, Clear, Fail, Pause, Count };

void PlaySfx(Sfx sfx);
void SetSoundMuted(bool muted);
