#include "render/Shader.h"

GLuint Shader::Compile(GLenum type, const char *fileName)
{
    std::string text = ReadTextFile(fileName);
    if (text.empty()) { return 0; }
    const char *source = text.c_str();
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE)
    {
        char log[4096]{};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << fileName << ": " << log << '\n';
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool Shader::Load(const char *vertexFile, const char *fragmentFile)
{
    GLuint vertexShader = Compile(GL_VERTEX_SHADER, vertexFile);
    GLuint fragmentShader = Compile(GL_FRAGMENT_SHADER, fragmentFile);
    if (!vertexShader || !fragmentShader)
    {
        if (vertexShader) { glDeleteShader(vertexShader); }
        if (fragmentShader) { glDeleteShader(fragmentShader); }
        return false;
    }
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    GLint success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE)
    {
        char log[4096]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "Program link failed (" << vertexFile << ", " << fragmentFile << "): " << log << '\n';
        glDeleteProgram(program);
        program = 0;
        return false;
    }
    return true;
}

void Shader::Destroy()
{
    if (program) { glDeleteProgram(program); }
    program = 0;
    locations.clear();
}

GLint Shader::Location(const char *name)
{
    auto found = locations.find(name);
    if (found != locations.end()) { return found->second; }
    GLint location = glGetUniformLocation(program, name);
    if (location < 0) { std::cerr << "uniform not found (or optimized out): " << name << '\n'; }
    locations[name] = location;
    return location;
}
