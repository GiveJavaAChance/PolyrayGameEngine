layout(std140, binding = CAM2D_IDX) uniform Camera2DBuffer {
    mat3 cameraTransform;
    mat3 projection;
};