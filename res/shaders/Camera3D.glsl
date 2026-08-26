layout(std140, binding = CAM3D_IDX) uniform Camera3DBuffer {
    mat4 cameraTransform;
    mat4 inverseCameraTransform;
    mat4 projection;
    mat4 inverseProjection;
    vec3 cameraPos;
};