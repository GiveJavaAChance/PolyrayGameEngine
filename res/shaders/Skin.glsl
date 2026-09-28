layout(std430, binding = SKIN_JOINT_IDX) readonly buffer SkinJointBuffer {
    mat4 skinJoints[];
};

layout(std430, binding = SKIN_INS_IDX) readonly buffer SkinInstanceBuffer {
    uint skins[];
};