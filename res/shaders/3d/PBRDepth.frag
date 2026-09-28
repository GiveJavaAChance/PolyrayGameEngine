#version 430

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : enable

#append "shaders/PBRMaterial.glsl";

in vec2 uv;
in flat uint materialIdx;

layout(std430, binding = MAT_INS_IDX) readonly buffer MaterialInstanceBuffer {
    PBRMaterial materials[];
};

out vec4 dummyColor;

void main() {
    PBRMaterial material = materials[materialIdx];
    vec4 baseColor = material.baseColor;
    if(material.baseColorTexture != 0ul) {
        vec2 transformedUv = transformUv(uv, material.baseColorUvTransform);

        baseColor *= texture(sampler2D(material.baseColorTexture), transformedUv);

        vec2 texCoord = transformedUv * vec2(textureSize(sampler2D(material.baseColorTexture), 0));

        vec2 dx = dFdx(texCoord);
        vec2 dy = dFdy(texCoord);

        float deltaMaxSqr = max(dot(dx, dx), dot(dy, dy));
        float mipLevel = max(0.0, 0.5 * log2(deltaMaxSqr));

        const float mipScale = 0.15;

        baseColor.a *= 1.0 + mipLevel * mipScale;
    }

    baseColor.a = (baseColor.a - material.alphaCutoff) / max(fwidth(baseColor.a), 0.0001) + 0.5;

    if(baseColor.a <= 0.0) {
        discard;
    }

    dummyColor = vec4(0.0, 0.0, 0.0, baseColor.a);
}