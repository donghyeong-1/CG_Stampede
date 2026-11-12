// [공통] 플랫폼/라이브러리 포함 모음. 실습 14~24 틀의 include 구성을 한 곳으로 모았다.
// 필요한 라이브러리: GLFW, GLEW, OpenGL, GLM (모두 third_party/에 동봉).
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <GL/glew.h>
#include <GL/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

// 실행 파일 폴더 → 현재 작업 폴더 순으로 리소스(셰이더·텍스처)를 찾는다.
// 빌드 PC의 절대 경로에 의존하지 않으므로 다른 PC에서도 실행된다.
std::filesystem::path AssetPath(const char *name);
std::string ReadTextFile(const char *name);
