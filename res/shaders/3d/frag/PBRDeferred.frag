#version 430

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : enable

#append "shaders/PBRMaterial.glsl";

#append "shaders/Hash.glsl";

in vec2 uv;
in vec3 pos;
in flat uint materialIdx;
in mat3 tbn;

layout(std430, binding = MAT_INS_IDX) readonly buffer MaterialInstanceBuffer {
    PBRMaterial materials[];
};

layout(location = 0) out vec3 gAlbedo;
layout(location = 1) out vec4 gNormalRoughness;
layout(location = 2) out vec4 gMetallicF0;

void main() {
    PBRMaterial material = materials[materialIdx];
    vec4 baseColor = material.baseColor;
    if(material.baseColorTexture != 0ul) {
        baseColor *= texture(sampler2D(material.baseColorTexture), transformUv(uv, material.baseColorUvTransform));
    }
    vec3 albedo = baseColor.rgb;

    vec3 normal = tbn[2u];
    if(material.normalMapTexture != 0ul) {
        normal = texture(sampler2D(material.normalMapTexture), transformUv(uv, material.normalUvTransform)).rgb;
        normal = normal * 2.0 - 1.0;
        normal = tbn * normal;
    }
    normal = normalize(normal);

    vec2 metallicRoughness = material.baseMetallicRougness;
    if(material.metallicRoughnessTexture != 0ul) {
        vec4 mr = texture(sampler2D(material.metallicRoughnessTexture), transformUv(uv, material.metallicRoughnessUvTransform));
        metallicRoughness *= mr.bg;
    }
    float metallic = metallicRoughness.x;
    float roughness = metallicRoughness.y;

    gAlbedo = albedo;
    gNormalRoughness = vec4(normal * 0.5 + 0.5, roughness);
    gMetallicF0 = vec4(metallic, material.F0);
}
