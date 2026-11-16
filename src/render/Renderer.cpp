#include "render/Renderer.h"
#include "render/Texture.h"

bool Renderer::Init()
{
    if (!phong.Load("shaders/phong.vert", "shaders/phong.frag")) { return false; }
    if (!ui.Load("shaders/ui.vert", "shaders/ui.frag")) { return false; }
    cube.Create(MakeCubeVertices());
    floorPlane.Create(MakeFloorVertices());
    quad.Create(MakeQuadVertices());
    floorTexture = LoadBmpTexture("assets/textures/ground.bmp");
    if (!floorTexture) { floorTexture = MakeCheckerTexture(256, 8, {0.85f, 0.7f, 0.5f}, {0.7f, 0.55f, 0.38f}); }
    return true;
}

void Renderer::Shutdown()
{
    cube.Destroy();
    floorPlane.Destroy();
    quad.Destroy();
    if (floorTexture) { glDeleteTextures(1, &floorTexture); }
    floorTexture = 0;
    textCache.Destroy();
    phong.Destroy();
    ui.Destroy();
}

void Renderer::BeginScene(const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &eye,
                          const DirLight &light, int framebufferWidth, int framebufferHeight)
{
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glDisable(GL_BLEND);
    const glm::vec3 sky(0.62f, 0.38f, 0.26f); // 산불 연기가 낀 하늘색. 안개 색으로도 쓴다.
    glClearColor(sky.r, sky.g, sky.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    phong.Use();
    phong.SetMat4("uViewProj", projection * view);
    phong.SetVec3("uViewPos", eye);
    phong.SetVec3("uLightDir", light.directionToLight);
    phong.SetVec3("uLightColor", light.color);
    phong.SetFloat("uAmbient", light.ambient);
    phong.SetFloat("uShininess", 32.0f);
    phong.SetFloat("uSpecular", 0.25f);
    phong.SetInt("uTexture", 0);
    phong.SetVec3("uFogColor", sky);
    phong.SetVec2("uUvOffset", glm::vec2(0.0f));
    phong.SetInt("uUnlit", 0);
}

void Renderer::DrawCube(const glm::mat4 &parent, const glm::vec3 &position, const glm::vec3 &size,
                        const glm::vec3 &color, float highlight, float alpha)
{
    glm::mat4 model = parent * glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), size);
    phong.SetMat4("uModel", model);
    phong.SetMat3("uNormalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
    phong.SetVec3("uColor", color);
    phong.SetFloat("uHighlight", highlight);
    phong.SetFloat("uAlpha", alpha);
    phong.SetInt("uUseTexture", 0);
    phong.SetVec2("uUvScale", glm::vec2(1.0f));
    cube.Draw();
}

void Renderer::DrawGlowCube(const glm::mat4 &parent, const glm::vec3 &position, const glm::vec3 &size,
                            const glm::vec3 &color)
{
    phong.SetInt("uUnlit", 1);
    DrawCube(parent, position, size, color);
    phong.SetInt("uUnlit", 0);
}

void Renderer::DrawFloor(float halfWidth, float zStart, float zEnd, float tileMeters)
{
    float length = zEnd - zStart;
    if (length <= 0.0f) { return; }
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, zStart)) *
                      glm::scale(glm::mat4(1.0f), glm::vec3(halfWidth, 1.0f, length));
    // 메시 UV는 (0~1, 0~1). 월드 길이 / 타일 길이만큼 반복하고, 시작 z만큼 밀어 조각 사이 무늬를 잇는다.
    glm::vec2 uvRepeat(2.0f * halfWidth / tileMeters, length / tileMeters);
    phong.SetMat4("uModel", model);
    phong.SetMat3("uNormalMatrix", glm::mat3(1.0f));
    phong.SetVec3("uColor", glm::vec3(1.0f));
    phong.SetFloat("uHighlight", 0.0f);
    phong.SetFloat("uAlpha", 1.0f);
    phong.SetInt("uUseTexture", 1);
    phong.SetVec2("uUvScale", uvRepeat);
    phong.SetVec2("uUvOffset", glm::vec2(0.0f, zStart / tileMeters));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    floorPlane.Draw();
    phong.SetVec2("uUvOffset", glm::vec2(0.0f));
}

void Renderer::BeginUi(int framebufferWidth, int framebufferHeight)
{
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ui.Use();
    // 픽셀 좌표(좌상단 원점) → NDC. bottom/top을 뒤집어 y가 아래로 증가하게 한다.
    ui.SetMat4("uProjection", glm::ortho(0.0f, static_cast<float>(framebufferWidth),
                                         static_cast<float>(framebufferHeight), 0.0f, -1.0f, 1.0f));
}

void Renderer::DrawRect(float x, float y, float width, float height, const glm::vec4 &rgba)
{
    ui.SetInt("uUseTexture", 0);
    ui.SetVec4("uRect", glm::vec4(x, y, width, height));
    ui.SetVec4("uColor", rgba);
    quad.Draw();
}

float Renderer::DrawLabel(const std::string &text, float x, float y, float pixelHeight, const glm::vec4 &rgba,
                          TextAlign align)
{
    if (text.empty()) { return 0.0f; }
    const TextTexture &label = textCache.Get(text, static_cast<int>(std::lround(pixelHeight)));
    if (!label.texture) { return 0.0f; }
    float width = static_cast<float>(label.width), height = static_cast<float>(label.height);
    if (align == TextAlign::Center) { x -= width * 0.5f; }
    if (align == TextAlign::Right) { x -= width; }
    ui.SetInt("uUseTexture", 1);
    ui.SetInt("uTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, label.texture);
    ui.SetVec4("uRect", glm::vec4(std::floor(x), std::floor(y), width, height)); // 픽셀에 맞춰 글자를 또렷하게.
    ui.SetVec4("uColor", rgba);
    quad.Draw();
    return width;
}

void Renderer::EndUi()
{
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}
