// STAMPEDE 협곡 탈출 — 진입점. 실습 틀(공통.cpp)의 초기화 → 메인 루프 → 정리 순서를 따른다.
#include "core/Game.h"

namespace
{
Game *GameFrom(GLFWwindow *window) { return static_cast<Game *>(glfwGetWindowUserPointer(window)); }
} // namespace

int main()
{
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
    glfwSetWindowUserPointer(window, &game);
    glfwSetKeyCallback(window, [](GLFWwindow *w, int key, int, int action, int) { GameFrom(w)->OnKey(key, action); });
    glfwSetMouseButtonCallback(window, [](GLFWwindow *w, int button, int action, int) { GameFrom(w)->OnMouseButton(button, action); });
    glfwSetCursorPosCallback(window, [](GLFWwindow *w, double x, double y) { GameFrom(w)->OnCursorPos(x, y); });
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *w, int width, int height) { GameFrom(w)->OnFramebufferResize(width, height); });

    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        double currentTime = glfwGetTime();
        float dt = static_cast<float>(std::min(currentTime - lastTime, 0.1)); // 긴 프레임 지연은 0.1초로 제한.
        lastTime = currentTime;
        game.Frame(dt);
        glfwSwapBuffers(window);
    }
    game.Shutdown(); // 현재 OpenGL 컨텍스트가 살아 있을 때 해제.
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
