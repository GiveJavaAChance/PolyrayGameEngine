#version 420

#append "shaders/Camera3D.glsl";

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uvCoords;
layout(location = 4) in mat4 instanceTransform;

#instancefrom 4

out vec2 uv;
out vec3 pos;
out mat3 tbn;

void main() {
    vec3 p = (instanceTransform * vec4(position, 1.0)).xyz;
    gl_Position = projection * (cameraTransform * vec4(p - cameraPos, 1.0));

    vec3 bitangent = cross(normal, tangent);
    mat3 M = mat3(instanceTransform);
    vec3 T = normalize(M * tangent);
    vec3 B = normalize(M * bitangent);
    vec3 N = normalize(transpose(inverse(M)) * normal);
    tbn = mat3(T, B, N);

    uv = uvCoords;
    pos = p;
}
