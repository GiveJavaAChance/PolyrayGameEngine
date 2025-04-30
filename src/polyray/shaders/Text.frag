#version 330 core

in vec3 uv;
in vec4 color;

uniform sampler2DArray textureSamplers;

out vec4 fragColor;

void main() {
    fragColor = texture(textureSamplers, uv) * color;
}