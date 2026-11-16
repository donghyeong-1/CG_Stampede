#version 330 core
layout(location = 0) in vec3 vPosition; // 단위 사각형 (0,0)~(1,1).
layout(location = 2) in vec2 vUV;
uniform mat4 uViewProj;
uniform vec3 uCenter;      // 입자 중심(월드).
uniform float uSize;       // 한 변 길이(월드).
uniform vec3 uCameraRight; // 카메라의 오른쪽·위쪽 방향(월드). 이 두 축으로 펼치면 항상 카메라를 향한다(빌보드).
uniform vec3 uCameraUp;
out vec2 fragUV;

void main()
{
    vec2 corner = (vPosition.xy - 0.5) * uSize;
    vec3 world = uCenter + uCameraRight * corner.x + uCameraUp * corner.y;
    fragUV = vUV;
    gl_Position = uViewProj * vec4(world, 1.0);
}
