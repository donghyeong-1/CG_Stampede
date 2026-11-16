#version 330 core
layout(location = 0) out vec4 FragColor;
uniform vec4 uColor; // RGBA. 알파 블렌딩으로 반투명 UI 지원.

void main()
{
    FragColor = uColor;
}
