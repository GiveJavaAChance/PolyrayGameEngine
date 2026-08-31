vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    float x = 1.0 - cosTheta;
    float x2 = x * x;
    return F0 + (1.0 - F0) * x2 * x2 * x;
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

vec3 PBRLighting(vec3 normal, vec3 viewDir, vec3 lightDir, vec3 lightColor, vec3 albedo, float roughness, float metallic, vec3 F0) {
    vec3 H = normalize(viewDir + lightDir);
    vec3 F = fresnelSchlick(clamp(dot(H, viewDir), 0.0, 1.0), F0);
    
    float NV = max(dot(normal, viewDir), 0.0);
    float NL = max(dot(normal, lightDir), 0.0);
    
    float D = distributionGGX(normal, H, roughness);
    float G = geometrySchlickGGX(NV, roughness) *
              geometrySchlickGGX(NL, roughness);

    vec3 specular = D * G * F / max(NV * NL, 0.001) * 0.25;

    vec3 kD = (1.0 - F) * (1.0 - metallic);

    vec3 diffuse = kD * albedo / PI;
    return (diffuse + specular) * lightColor * NL;
}