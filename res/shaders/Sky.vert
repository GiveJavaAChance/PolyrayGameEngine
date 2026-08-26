#version 420
#append "shaders/Camera3D.glsl";

layout(location = 0) in vec2 position;

out vec3 rayDir;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    vec4 clipSpace = vec4(position, -1.0, 1.0);
    vec4 viewSpace = inverseProjection * clipSpace;
    vec3 rayDirView = normalize(viewSpace.xyz / viewSpace.w);
    rayDir = -(mat3(inverseCameraTransform) * rayDirView);
}