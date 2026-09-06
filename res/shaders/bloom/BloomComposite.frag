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

    vec3 accum = texture(bloomSampler, uv).rgb / float(layers - 1);

    vec3 ld = vec3(1.0);
    if (useLensDirt) {
        ld = texture(lensDirtTexture, uv).rgb;
    }
    accum *= intensity * ld;

    col.rgb += accum;

    fragColor = col;
}