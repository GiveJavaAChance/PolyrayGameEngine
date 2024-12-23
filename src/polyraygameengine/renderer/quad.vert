#version 330 core

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 2) in mat3 instanceTransform;

uniform vec2 windowSize;
uniform mat3 cameraTransform;

out vec2 fragTexCoord;

void main() {
    vec3 transformedPosition = cameraTransform * instanceTransform * vec3(inPosition, 1.0);
    vec2 v = vec2(transformedPosition.x / windowSize.x * 2.0 - 1.0, transformedPosition.y / windowSize.y * 2.0 + 1.0);
    gl_Position = vec4(v, 0.0, 1.0);
    
    fragTexCoord = inUV;
}