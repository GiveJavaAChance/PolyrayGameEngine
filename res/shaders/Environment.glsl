layout(std140, binding = ENV_IDX) uniform EnvironmentBuffer {
    vec3 ambientColor;
};

vec3 getAmbientColor(vec3 normal) {
    return mix(vec3(0.3), ambientColor, normal.y * 0.25 + 0.5);
}