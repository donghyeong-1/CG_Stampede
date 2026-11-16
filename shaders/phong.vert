#version 330 core
layout(location = 0) in vec3 vPosition; // 로컬 정점.
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUV;
uniform mat4 uModel;
uniform mat4 uViewProj;      // projection * view (C++에서 계산).
uniform mat3 uNormalMatrix;  // transpose(inverse(mat3(uModel))): 비균등 크기에서도 법선 유지.
out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragUV;

void main()
{
    vec4 world = uModel * vec4(vPosition, 1.0);
    fragPosition = world.xyz;
    fragNormal = uNormalMatrix * vNormal;
    fragUV = vUV;
    gl_Position = uViewProj * world;
}
