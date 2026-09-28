#version 430

uniform int layers;

in vec2 uv;

uniform float intensity = 1.0;
uniform bool useLensDirt = false;

layout(binding = 0) uniform sampler2D bloomSampler;
layout(binding = 1) uniform sampler2D screenTexture;
layout(binding = 2) uniform sampler2D lensDirtTexture;

out vec4 fragColor;

void main() {
    vec4 col = texture(screenTexture, uv);

    vec3 bloom = texture(bloomSampler, uv).rgb / float(layers - 1);

    vec3 ld = vec3(1.0);
    if (useLensDirt) {
        ld = texture(lensDirtTexture, uv).rgb;
    }
    bloom *= intensity * ld;

    float bloomAlpha = mix(max(max(bloom.r, bloom.g), bloom.b), 1.0, col.a);
    bloom /= bloomAlpha;

    col.rgb += bloom * bloomAlpha;
    col.a = col.a + bloomAlpha * (1.0 - col.a);

    fragColor = col;
}