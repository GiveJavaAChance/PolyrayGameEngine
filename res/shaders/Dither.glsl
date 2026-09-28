#append "shaders/Hash.glsl";

vec4 ditherColor(vec4 color, float strength) {
    return color + hash42(gl_FragCoord.xy) * strength / 255.0;
}