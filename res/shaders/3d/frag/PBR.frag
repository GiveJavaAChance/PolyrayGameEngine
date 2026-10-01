#version 430

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : enable

#append "shaders/Constants.glsl";
#append "shaders/Environment.glsl";
#append "shaders/Camera3D.glsl";
#append "shaders/PBRLighting.glsl";

#append "shaders/PBRMaterial.glsl";

#append "shaders/Hash.glsl";

in vec2 uv;
in vec3 pos;
in flat uint materialIdx;
in mat3 tbn;

layout(std430, binding = MAT_INS_IDX) readonly buffer MaterialInstanceBuffer {
    PBRMaterial materials[];
};

#append "shaders/Light.glsl";
#append "shaders/Shadow.glsl";
#append "shaders/LightShadow.glsl";

layout(binding = AO_TEXTURE_IDX, r16f) uniform image2D ambientOcclusion;

out vec4 fragColor;

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
        normal = normalize(tbn * normal);
    }

    vec2 metallicRoughness = material.baseMetallicRougness;
    if(material.metallicRoughnessTexture != 0ul) {
        vec4 mr = texture(sampler2D(material.metallicRoughnessTexture), transformUv(uv, material.metallicRoughnessUvTransform));
        metallicRoughness *= mr.bg;
    }
    float metallic = metallicRoughness.x;
    float roughness = metallicRoughness.y;

    vec3 viewDir = normalize(cameraPos - pos);

    vec3 color = vec3(0.0);

    for(uint i = 0u; i < directionalLight3DCount; i++) {
        DirectionalLight3D light = directionalLights3D[i];
        float shadow = getDirectionalLightShadow(light, pos, normal);
        if(shadow == 0.0) {
            continue;
        }
        vec3 l = PBRLighting(normal, viewDir, light.dir, light.color * light.strength, albedo, roughness, metallic, material.F0);
        color += l * shadow;
    }
    for(uint i = 0u; i < spotLight3DCount; i++) {
        SpotLight3D light = spotLights3D[i];
        float shadow = getSpotLightShadow(light, pos, normal);
        if(shadow == 0.0) {
            continue;
        }
        vec3 l = PBRLighting(normal, viewDir, normalize(light.pos - pos), getSpotLightColor(light, pos), albedo, roughness, metallic, material.F0);
        color += l * shadow;
    }
    for(uint i = 0u; i < pointLight3DCount; i++) {
        PointLight3D light = pointLights3D[i];
        float shadow = getPointLightShadow(light, pos, normal);
        if(shadow == 0.0) {
            continue;
        }
        vec3 l = PBRLighting(normal, viewDir, normalize(light.pos - pos), getPointLightColor(light, pos), albedo, roughness, metallic, material.F0);
        color += l * shadow;
    }

    float ao = imageLoad(ambientOcclusion, ivec2(gl_FragCoord.xy)).r;

    color += getAmbientColor(normal) * albedo * ao;

    fragColor = vec4(color, 1.0);
}
