#version 420

#append "Camera3D.glsl";

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uvCoords;
layout(location = 3) in mat4 instanceTransform;

out vec2 uv;
out vec3 norm;
out vec3 pos;

void main() {
    vec3 p = (instanceTransform * vec4(position, 1.0)).xyz;
    gl_Position = projection * (viewMatrix * vec4(p, 1.0));
    mat3 normalMatrix = transpose(inverse(mat3(instanceTransform)));
    norm = normalize(normalMatrix * normal);
    uv = uvCoords;
    pos = p;
}
