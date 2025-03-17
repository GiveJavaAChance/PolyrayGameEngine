#version 330 core

in vec2 uv;

uniform sampler2D textureSampler;

out vec4 fragColor;

void main() {
    vec4 col = texture(textureSampler, uv);
    if(col.a < 0.01) {
        discard;
    }
    fragColor = col;
}