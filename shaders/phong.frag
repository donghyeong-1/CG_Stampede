#version 330 core
in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragUV;
layout(location = 0) out vec4 FragColor;

uniform vec3 uColor;
uniform float uAlpha;
uniform float uHighlight;     // 0~1: 마우스로 조준한 동물 강조.
uniform vec3 uLightDir;       // 표면 → 광원 방향(정규화 전).
uniform vec3 uLightColor;
uniform float uAmbient;
uniform vec3 uViewPos;
uniform float uShininess;
uniform float uSpecular;
uniform int uUseTexture;
uniform sampler2D uTexture;
uniform vec2 uUvScale;        // 바닥 UV 반복 횟수.

void main()
{
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uViewPos - fragPosition);
    vec3 R = reflect(-L, N);
    vec3 base = uColor;
    if (uUseTexture == 1) { base *= texture(uTexture, fragUV * uUvScale).rgb; }
    float diffuse = max(dot(N, L), 0.0);
    float specular = pow(max(dot(V, R), 0.0), uShininess) * uSpecular;
    vec3 lit = base * (uAmbient + diffuse * uLightColor) + specular * uLightColor;
    lit = mix(lit, vec3(1.0, 0.95, 0.35), uHighlight * 0.45);
    FragColor = vec4(lit, uAlpha);
}
