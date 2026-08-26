struct DirectionalLight3D {
    vec3 color;
    float strength;
    vec3 dir;
    uint shadowIdx;
};

struct SpotLight3D {
    vec3 color;
    float strength;
    vec3 pos;
    float distanceAttenuation;
    vec3 dir;
    float spotCosAngle;
    uint shadowIdx;
};

struct PointLight3D {
    vec3 color;
    float strength;
    vec3 pos;
    float distanceAttenuation;
    uint shadowIdx[6u];
};

layout(std430, binding = DIRECTIONAL_LIGHT3D_IDX) buffer DirectionalLight3DBuffer {
    uint directionalLight3DCount;
    DirectionalLight3D directionalLights3D[];
};

layout(std430, binding = SPOT_LIGHT3D_IDX) buffer SpotLight3DBuffer {
    uint spotLight3DCount;
    SpotLight3D spotLights3D[];
};

layout(std430, binding = POINT_LIGHT3D_IDX) buffer PointLight3DBuffer {
    uint pointLight3DCount;
    PointLight3D pointLights3D[];
};

vec3 getSpotLightColor(SpotLight3D light, vec3 pos) {
    vec3 lightDir = light.pos - pos;
    float dist = length(lightDir);
    lightDir /= dist;
    float k = (dot(lightDir, light.dir) - light.spotCosAngle) / (1.0 - light.spotCosAngle);
    if(k < 0.0) {
        return vec3(0.0);
    }
    return light.color * light.strength * k / (1.0 + light.distanceAttenuation * dist * dist);
}

vec3 getPointLightColor(PointLight3D light, vec3 pos) {
    vec3 lightDir = light.pos - pos;
    float dist = length(lightDir);
    lightDir /= dist;
    return light.color * light.strength / (1.0 + light.distanceAttenuation * dist * dist);
}