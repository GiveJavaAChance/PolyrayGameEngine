#version 330 core

in vec2 fragTexCoord;

uniform sampler2D textureSampler;

out vec4 fragColor;

void main() {
    vec4 texColor = texture(textureSampler, fragTexCoord);
    fragColor = texColor;
}