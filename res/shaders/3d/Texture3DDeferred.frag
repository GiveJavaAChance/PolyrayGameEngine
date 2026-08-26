#version 420

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

layout(location = 0) out vec3 gAlbedo;
layout(location = 1) out vec4 gNormalRoughness;
layout(location = 2) out vec4 gMetallicF0;

void main() {
    gAlbedo = texture(albedoMap, uv).rgb;
    vec3 normal = normalize(tbn * (texture(normalMap, uv).rgb * 2.0 - 1.0)) * 0.5 + 0.5;
    vec2 roughnessMetallic = texture(roughnessMetallicMap, uv).rg;
    gNormalRoughness = vec4(normal, roughnessMetallic.x);
    gMetallicF0 = vec4(roughnessMetallic.y, F0);
}
