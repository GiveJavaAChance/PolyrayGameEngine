#version 430

#append "shaders/ACESTonemap.glsl";
#append "shaders/GammaCorrect.glsl";
#append "shaders/Dither.glsl";

in vec2 uv;

out vec4 fragColor;

uniform float ditherStrength = 1.0;

layout(binding = 0) uniform sampler2D screenTexture;

void main() {
    vec3 col = texture(screenTexture, uv).rgb;
    col = ACESTonemap(col);
    col = gammaCorrect(col, 1.0);
    fragColor = vec4(ditherColor(col, ditherStrength), 1.0);
}