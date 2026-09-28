#version 430

#append "shaders/ACESTonemap.glsl";
#append "shaders/GammaCorrect.glsl";
#append "shaders/Dither.glsl";

in vec2 uv;

out vec4 fragColor;

uniform float ditherStrength = 1.0;

layout(binding = 0) uniform sampler2D screenTexture;

void main() {
    vec4 col = texture(screenTexture, uv);
    col.rgb = ACESTonemap(col.rgb);
    col.rgb = gammaCorrect(col.rgb, 1.0);
    fragColor = ditherColor(col, ditherStrength);
}