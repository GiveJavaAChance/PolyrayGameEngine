float getDirectionalLightShadow(DirectionalLight3D light, vec3 pos, vec3 normal) {
    if(light.shadowIdx == 0xFFFFFFFFu) {
        return 1.0;
    }
    return SampleShadowMapOptimizedPCF(light.shadowIdx, pos, normal, light.dir, 0.002);
}

float getSpotLightShadow(SpotLight3D light, vec3 pos, vec3 normal) {
    vec3 lightDir = normalize(light.pos - pos);
    if(dot(lightDir, light.dir) < light.spotCosAngle) {
        return 0.0;
    }
    if(light.shadowIdx == 0xFFFFFFFFu) {
        return 1.0;
    }
    return SampleShadowMapOptimizedPCF(light.shadowIdx, pos, normal, lightDir, 0.0001);
}

float getPointLightShadow(PointLight3D light, vec3 pos, vec3 normal) {
    vec3 lightDir = normalize(light.pos - pos);
    if(light.shadowIdx[0u] == 0xFFFFFFFFu) {
        return 1.0;
    }
    vec3 dir = pos - light.pos;
    vec3 aDir = abs(dir);

    uint shadowFace;

    if(aDir.x > aDir.y && aDir.x > aDir.z) {
        shadowFace = dir.x < 0.0 ? 0u : 1u;
    } else if(aDir.y > aDir.z) {
        shadowFace = dir.y < 0.0 ? 2u : 3u;
    } else {
        shadowFace = dir.z < 0.0 ? 4u : 5u;
    }
    return SampleShadowMapOptimizedPCF(light.shadowIdx[shadowFace], pos, normal, lightDir, 0.0001);
}