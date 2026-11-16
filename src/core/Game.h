// [공통] 게임 진행: 입력 → 갱신 → 그리기와 상태 전환(Title/Playing/Result).
// 실습 틀의 main 루프(PollEvents → dt → InputProcess → Update → Draw)를 클래스로 옮긴 것이다.
// 상태 전환 규칙: 마우스 좌클릭은 Title/Result에서는 버튼, Playing에서는 점프로 해석한다.
#pragma once
#include "core/GameState.h"
#include "logic/Camera.h"
#include "render/Renderer.h"

class Game
{
public:
    bool Init(GLFWwindow *window);
    void Shutdown();
    void Frame(float dt); // 입력 폴링 → Update → Draw.

    // GLFW 콜백에서 호출한다.
    void OnKey(int key, int action);
    void OnMouseButton(int button, int action);
    void OnCursorPos(double x, double y);
    void OnFramebufferResize(int width, int height);

private:
    void StartGame();       // 모든 상태를 초기화하고 Playing으로 전환(재시작 포함).
    void PollInput();       // A/D 조향처럼 누르고 있는 키.
    void UpdatePlaying(float dt);
    void UpdateAnimals(float dt);
    void TryJump();         // TODO(김동형): 점프·탑승 판정.
    void TryUseAbility();   // TODO(김동형): 가젤 도약 / 들소 돌진.
    void FinishGame(bool success);
    void Draw();
    void UpdateWindowTitle(); // 글자 UI가 생기기 전까지 상태를 창 제목에 표시.

    GLFWwindow *window = nullptr;
    GameState state;
    Camera camera;
    Renderer renderer;
    std::string lastTitle;
};
