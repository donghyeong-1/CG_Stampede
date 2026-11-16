#version 330 core
layout(location = 0) in vec3 vPosition; // 단위 사각형 (0,0)~(1,1).
uniform mat4 uProjection;               // 직교 투영: 픽셀 좌표, 좌상단 원점.
layout(location = 2) in vec2 vUV;
uniform vec4 uRect;                     // x, y, width, height (픽셀).
out vec2 fragUV;

void main()
{
    fragUV = vUV;
    vec2 pixel = uRect.xy + vPosition.xy * uRect.zw;
    gl_Position = uProjection * vec4(pixel, 0.0, 1.0);
}
