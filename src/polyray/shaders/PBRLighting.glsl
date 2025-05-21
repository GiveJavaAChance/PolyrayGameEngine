uniform float metallic;
uniform float roughness;
uniform vec3 F0;

struct PointLight {
    vec3 pos;
    vec3 color;
    float distanceAttenuation;
};

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

vec3 PBRLighting(vec3 normal, vec3 viewDir, vec3 lightDir, vec3 lightColor, vec3 albedo) {
    vec3 H = normalize(viewDir + lightDir);
    vec3 F = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);
    
    float NV = max(dot(normal, viewDir), 0.0);
    float NL = max(dot(normal, lightDir), 0.0);
    
    float D = distributionGGX(normal, H, roughness);
    float G = geometrySchlickGGX(NV, roughness) *
              geometrySchlickGGX(NL, roughness);

    vec3 specular = D * G * F / max(NV * NL, 0.01) * 0.25;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 diffuse = kD * albedo / PI;
    return (diffuse + specular) * lightColor * NL;
}

vec3 PBRLighting(vec3 normal, vec3 viewDir, vec3 pos, PointLight light, vec3 albedo) {
    vec3 lightDir = light.pos - pos;
    float dist = length(lightDir);
    lightDir /= dist;
    vec3 lightColor = light.color / (1.0 + light.distanceAttenuation * dist * dist);
    return PBRLighting(normal, viewDir, lightDir, lightColor, albedo);
}