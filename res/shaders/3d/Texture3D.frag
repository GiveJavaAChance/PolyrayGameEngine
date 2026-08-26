#version 430

#append "shaders/Constants.glsl";
#append "shaders/Environment.glsl";
#append "shaders/Camera3D.glsl";
#append "shaders/PBRLighting.glsl";

in vec2 uv;
in vec3 pos;
in mat3 tbn;

layout(binding = 0) uniform sampler2D albedoMap;
layout(binding = 1) uniform sampler2D normalMap;
layout(binding = 2) uniform sampler2D roughnessMetallicMap;

uniform vec3 F0;

#append "shaders/Light.glsl";
#append "shaders/Shadow.glsl";

layout(binding = AO_TEXTURE_IDX, r16f) uniform image2D ambientOcclusion;

out vec4 fragColor;

void main() {
    vec3 albedo = texture(albedoMap, uv).rgb;
    vec3 normal = texture(normalMap, uv).rgb;
    vec2 roughnessMetallic = texture(roughnessMetallicMap, uv).rg;

    normal = normal * 2.0 - 1.0;
    normal = normalize(tbn * normal);

    vec3 viewDir = normalize(cameraPos - pos);

    vec3 color = vec3(0.0);

    for(uint i = 0u; i < directionalLight3DCount; i++) {
        DirectionalLight3D light = directionalLights3D[i];
        vec3 l = PBRLighting(normal, viewDir, light.dir, light.color * light.strength, albedo, roughnessMetallic.x, roughnessMetallic.y, F0);
        if(light.shadowIdx != 0xFFFFFFFFu) {
            l *= SampleShadowMapOptimizedPCF(light.shadowIdx, pos, normal, light.dir, 0.002);
        }
        color += l;
    }
    for(uint i = 0u; i < spotLight3DCount; i++) {
        SpotLight3D light = spotLights3D[i];
        vec3 l = PBRLighting(normal, viewDir, normalize(light.pos - pos), getSpotLightColor(light, pos), albedo, roughnessMetallic.x, roughnessMetallic.y, F0);
        if(light.shadowIdx != 0xFFFFFFFFu) {
            l *= SampleShadowMapOptimizedPCF(light.shadowIdx, pos, normal, light.dir, 0.0001);
        }
        color += l;
    }

    float ao = imageLoad(ambientOcclusion, ivec2(gl_FragCoord.xy)).r;

    color += getAmbientColor(normal) * albedo * ao;

    fragColor = vec4(color, 1.0);
}
