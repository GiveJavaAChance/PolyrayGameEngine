#append "shaders/UvTransform.glsl";

struct PBRMaterial {
    vec4 baseColor;

    UvTransform baseColorUvTransform;
    uint64_t baseColorTexture;

    UvTransform normalUvTransform;
    uint64_t normalMapTexture;

    UvTransform metallicRoughnessUvTransform;
    uint64_t metallicRoughnessTexture;

    vec2 baseMetallicRougness;
    float alphaCutoff;
    uint doubleSided;

    vec3 F0;
};