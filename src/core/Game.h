// [공통] 게임 진행: 입력 → 갱신 → 그리기와 상태 전환(Title/Playing/Result).
// 실습 틀의 main 루프(PollEvents → dt → InputProcess → Update → Draw)를 클래스로 옮긴 것이다.
// 상태 전환 규칙: 마우스 좌클릭은 Title/Result에서는 버튼, Playing에서는 점프로 해석한다.
// 규칙 자체(점프·능력·충돌)는 logic/Simulation에 있고, 여기서는 입력과 화면만 연결한다.
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

    // 검증용(--autoplay): 스스로 시작하고 플레이한다.
    void SetAutoPlay(bool enabled) { autoPlay = enabled; }
    // 지금 그려진 화면(백 버퍼)을 24비트 BMP로 저장한다. 버퍼를 교체하기 전에 호출한다.
    bool SaveScreenshot(const std::string &path) const;

private:
    void StartGame();       // 모든 상태를 초기화하고 Playing으로 전환(재시작 포함).
    void PollInput();       // A/D 조향처럼 누르고 있는 키.
    void UpdatePlaying(float dt);
    glm::vec3 CameraTarget() const;
    void Draw();
    void UpdateWindowTitle();

    GLFWwindow *window = nullptr;
    GameState state;
    Camera camera;
    Renderer renderer;
    std::string lastTitle;
    bool autoPlay = false;
    float elapsed = 0.0f; // 실행 후 지난 시간(초).
};
