#version 330 core
in vec2 fragUV;
layout(location = 0) out vec4 FragColor;
uniform vec4 uColor; // RGBA. 알파 블렌딩으로 반투명 UI 지원.
uniform int uUseTexture;    // 1이면 글자 텍스처(빨강 채널 = 불투명도)를 곱한다.
uniform sampler2D uTexture;

void main()
{
    float coverage = 1.0;
    if (uUseTexture == 1) { coverage = texture(uTexture, fragUV).r; }
    FragColor = vec4(uColor.rgb, uColor.a * coverage);
}
