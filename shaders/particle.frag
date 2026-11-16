#version 330 core
in vec2 fragUV;
layout(location = 0) out vec4 FragColor;
uniform vec4 uColor;

void main()
{
    // 텍스처 없이 중심에서 가장자리로 부드럽게 옅어지는 둥근 점을 만든다.
    float distanceFromCenter = length(fragUV - 0.5) * 2.0;
    float soft = 1.0 - smoothstep(0.0, 1.0, distanceFromCenter);
    FragColor = vec4(uColor.rgb, uColor.a * soft);
}
