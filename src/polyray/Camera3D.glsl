layout(std140, binding = CAM3D_IDX) uniform Camera3DBuffer {
    mat4 viewMatrix;
    mat4 projection;
};