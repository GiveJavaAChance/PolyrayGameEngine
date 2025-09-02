#version 460

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uvCoords;

layout(std430, binding = STR_IDX) buffer PackedStringBuffer {
    uint packedData[];
};

#append "Camera2D.glsl";

out vec3 uv;
out vec4 color;

void main() {
    uint base = gl_BaseInstance;
    uint instance = gl_InstanceID;

    uint posPacked = packedData[base];
    ivec2 pos = ivec2(
        int((posPacked >> 16) & 0xFFFF) - 32768,
        int(posPacked & 0xFFFF) - 32768
    );
    uint col = packedData[base + 1];
    color = vec4(float((col >> 16u) & 0xFFu), float((col >> 8u) & 0xFFu), float(col & 0xFFu), float((col >> 24u) & 0xFFu)) / 255.0;

    uint charIndex = instance / 4;
    uint charOffset = instance & 3u;

    uint packedChars = packedData[base + INTS_PER_STRING + charIndex];
    uint charID = (packedChars >> ((3u - charOffset) << 3u)) & 0xFFu;

    vec2 p = position + pos + vec2(float(instance) * 11.0, 0.0);

    gl_Position = vec4((projection * cameraTransform * vec3(p, 1.0)).xy, 0.0, 1.0);
    uv = vec3(uvCoords, charID);
}