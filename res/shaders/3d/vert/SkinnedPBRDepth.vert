#version 430

#append "shaders/Camera3D.glsl";
#append "shaders/Skin.glsl";

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec3 TANGENT;
layout(location = 3) in vec2 TEXCOORD;
layout(location = 4) in uvec4 JOINTS;
layout(location = 5) in vec4 WEIGHTS;

#optional TANGENT
#optional TEXCOORD

layout(location = 6) in uint matIdx;
layout(location = 7) in uint skinIdx;
layout(location = 8) in mat4 instanceTransform;

#instancefrom 6

void main() {
    uint jointOffset = skins[skinIdx];

    mat4 skinMatrix = instanceTransform * (
        WEIGHTS.x * skinJoints[jointOffset + JOINTS.x]
      + WEIGHTS.y * skinJoints[jointOffset + JOINTS.y]
      + WEIGHTS.z * skinJoints[jointOffset + JOINTS.z]
      + WEIGHTS.w * skinJoints[jointOffset + JOINTS.w]
    );

    vec3 p = (skinMatrix * vec4(POSITION, 1.0)).xyz;
    gl_Position = projection * (cameraTransform * vec4(p - cameraPos, 1.0));
}