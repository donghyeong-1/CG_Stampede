// STAMPEDE 협곡 탈출 — 진입점. 실습 틀(공통.cpp)의 초기화 → 메인 루프 → 정리 순서를 따른다.
#include "core/Game.h"
#include "logic/AutoPlay.h"
#include <cstdlib>

namespace
{
Game *GameFrom(GLFWwindow *window) { return static_cast<Game *>(glfwGetWindowUserPointer(window)); }
} // namespace

// 실행 옵션(검증용. 옵션 없이 실행하면 일반 게임):
//   --selftest             창 없이 규칙을 검증하고 종료한다(실패 수가 종료 코드).
//   --autoplay             자동으로 시작해 플레이한다.
//   --shot <초> <파일.bmp>  실행 후 해당 시각의 화면을 저장한다(여러 번 지정 가능).
//   --exit-after <초>      해당 시각에 종료한다.
int main(int argc, char **argv)
{
    struct Shot { double time; std::string path; bool done; };
    std::vector<Shot> shots;
    bool autoPlay = false;
    double exitAfter = -1.0;
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--selftest") { return RunSelfTest(); }
        if (arg == "--autoplay") { autoPlay = true; }
        if (arg == "--shot" && i + 2 < argc) { shots.push_back({std::atof(argv[i + 1]), argv[i + 2], false}); i += 2; }
        if (arg == "--exit-after" && i + 1 < argc) { exitAfter = std::atof(argv[++i]); }
    }

    if (!glfwInit())
    {
        std::cerr << "GLFW initialization failed\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(cfg::WINDOW_WIDTH, cfg::WINDOW_HEIGHT, "STAMPEDE", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // OpenGL 함수를 호출하기 전에 현재 컨텍스트 지정.
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "GLEW initialization failed\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    glfwSwapInterval(1);

    Game game;
    if (!game.Init(window))
    {
        std::cerr << "Game initialization failed\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    game.SetAutoPlay(autoPlay);
    glfwSetWindowUserPointer(window, &game);
    glfwSetKeyCallback(window, [](GLFWwindow *w, int key, int, int action, int) { GameFrom(w)->OnKey(key, action); });
    glfwSetMouseButtonCallback(window, [](GLFWwindow *w, int button, int action, int) { GameFrom(w)->OnMouseButton(button, action); });
    glfwSetCursorPosCallback(window, [](GLFWwindow *w, double x, double y) { GameFrom(w)->OnCursorPos(x, y); });
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *w, int width, int height) { GameFrom(w)->OnFramebufferResize(width, height); });

    double lastTime = glfwGetTime();
    const double startTime = lastTime;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        double currentTime = glfwGetTime();
        float dt = static_cast<float>(std::min(currentTime - lastTime, 0.1)); // 긴 프레임 지연은 0.1초로 제한.
        lastTime = currentTime;
        game.Frame(dt);
        for (Shot &shot : shots)
        {
            if (!shot.done && currentTime - startTime >= shot.time)
            {
                shot.done = true;
                std::cout << (game.SaveScreenshot(shot.path) ? "saved " : "failed to save ") << shot.path << std::endl;
            }
        }
        if (exitAfter >= 0.0 && currentTime - startTime >= exitAfter) { glfwSetWindowShouldClose(window, GLFW_TRUE); }
        glfwSwapBuffers(window);
    }
    game.Shutdown(); // 현재 OpenGL 컨텍스트가 살아 있을 때 해제.
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
