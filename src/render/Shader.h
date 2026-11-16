// [장수현] 셰이더 래퍼. 실습의 filetobuf → MakeShader → make_shaderProgram 흐름을 클래스로 묶었다.
// 컴파일·링크 로그를 항상 출력한다(제안서의 셰이더 오류 대응).
#pragma once
#include "core/Platform.h"
#include <string>
#include <unordered_map>

class Shader
{
public:
    bool Load(const char *vertexFile, const char *fragmentFile);
    void Destroy();
    void Use() const { glUseProgram(program); }

    void SetInt(const char *name, int value) { glUniform1i(Location(name), value); }
    void SetFloat(const char *name, float value) { glUniform1f(Location(name), value); }
    void SetVec2(const char *name, const glm::vec2 &v) { glUniform2fv(Location(name), 1, glm::value_ptr(v)); }
    void SetVec3(const char *name, const glm::vec3 &v) { glUniform3fv(Location(name), 1, glm::value_ptr(v)); }
    void SetVec4(const char *name, const glm::vec4 &v) { glUniform4fv(Location(name), 1, glm::value_ptr(v)); }
    // GLM은 열 우선이므로 전치하지 않는다.
    void SetMat3(const char *name, const glm::mat3 &m) { glUniformMatrix3fv(Location(name), 1, GL_FALSE, glm::value_ptr(m)); }
    void SetMat4(const char *name, const glm::mat4 &m) { glUniformMatrix4fv(Location(name), 1, GL_FALSE, glm::value_ptr(m)); }

private:
    GLint Location(const char *name);
    static GLuint Compile(GLenum type, const char *fileName);

    GLuint program = 0;
    std::unordered_map<std::string, GLint> locations;
};
