#version 420

#append "shaders/Camera3D.glsl";

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec3 TANGENT;
layout(location = 3) in vec2 TEXCOORD;

#optional TANGENT
#optional TEXCOORD

layout(location = 4) in uint matIdx;
layout(location = 5) in mat4 instanceTransform;

#instancefrom 4

out vec2 uv;
out vec3 pos;
out flat uint materialIdx;
out mat3 tbn;

void main() {
    vec3 p = (instanceTransform * vec4(POSITION, 1.0)).xyz;
    gl_Position = projection * (cameraTransform * vec4(p - cameraPos, 1.0));

    uv = TEXCOORD;
    pos = p;

    materialIdx = matIdx;

    vec3 bitangent = cross(NORMAL, TANGENT);
    mat3 M = mat3(instanceTransform);
    vec3 T = normalize(M * TANGENT);
    vec3 B = normalize(M * bitangent);
    vec3 N = normalize(transpose(inverse(M)) * NORMAL);
    tbn = mat3(T, B, N);
}
