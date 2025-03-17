layout(std140, binding = ENV_IDX) uniform EnvBuffer {
    vec3 sunDir;
    vec3 sunColor;
    vec3 ambientColor;
};