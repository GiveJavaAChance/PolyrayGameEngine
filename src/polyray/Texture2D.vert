#version 330 core

#append "Camera2D.glsl";

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uvCoords;

layout(location = 2) in mat3 instanceTransform;

out vec2 uv;

void main() {
    vec2 p = (instanceTransform * vec3(inPosition, 1.0)).xy;
    gl_Position = vec4((cameraTransform * vec3(v, 1.0)).xy, 0.0, 1.0);
    uv = uvCoords;
}