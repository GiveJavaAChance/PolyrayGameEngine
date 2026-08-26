#define FilterSize 7

struct Shadow {
    mat4 lightSpaceTransform;
    uvec2 shadowMapOffset;
    uvec2 shadowMapSize;
};

layout(std430, binding = SHADOW3D_IDX) buffer ShadowCamera3DBuffer {
    Shadow shadows[];
};

layout(binding = 32) uniform sampler2DShadow shadowMapAtlas;

float SampleShadowMap(vec2 base_uv, float u, float v, vec2 shadowMapSizeInv, float depth, vec2 uvScale, vec2 uvOffset) {
    vec2 uv = base_uv + vec2(u, v) * shadowMapSizeInv;
    if(any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) {
        return 1.0;
    }
    return texture(shadowMapAtlas, vec3(uv * uvScale + uvOffset, depth));
}

float SampleShadowMapOptimizedPCF(uint shadowIdx, vec3 pos, vec3 normal, vec3 lightDir, float baseBias) {
    Shadow shadow = shadows[shadowIdx];
    vec4 lightSpacePos = shadow.lightSpaceTransform * vec4(pos, 1.0);
    vec3 shadowPos = lightSpacePos.xyz / lightSpacePos.w;
    shadowPos.xy = shadowPos.xy * 0.5 + 0.5;

    vec2 shadowAtlasSize = textureSize(shadowMapAtlas, 0);
    vec2 shadowMapSize = vec2(shadow.shadowMapSize);

    vec2 uvScale = shadowMapSize / shadowAtlasSize;
    vec2 uvOffset = vec2(shadow.shadowMapOffset) / shadowAtlasSize;

    float lightDepth = shadowPos.z;

    //const float bias = 0.003;
    float bias = baseBias / max(pow(dot(normal, lightDir), 1.4), 0.01);

    lightDepth += bias;

    vec2 uv = shadowPos.xy * shadowMapSize; // 1 unit - 1 texel

    vec2 shadowMapSizeInv = 1.0 / shadowMapSize;

    vec2 base_uv;
    base_uv = floor(uv + 0.5);

    float s = (uv.x + 0.5 - base_uv.x);
    float t = (uv.y + 0.5 - base_uv.y);

    base_uv -= vec2(0.5);
    base_uv *= shadowMapSizeInv;

    float sum = 0;

    #if FilterSize == 2
        if(any(lessThan(shadowPos.xy, vec2(0.0))) || any(greaterThan(shadowPos.xy, vec2(1.0)))) {
            return 1.0;
        }
        return texture(shadowMapAtlas, vec3(shadowPos.xy * uvScale + uvOffset, lightDepth));
    #elif FilterSize == 3

        float uw0 = (3 - 2 * s);
        float uw1 = (1 + 2 * s);

        float u0 = (2 - s) / uw0 - 1;
        float u1 = s / uw1 + 1;

        float vw0 = (3 - 2 * t);
        float vw1 = (1 + 2 * t);

        float v0 = (2 - t) / vw0 - 1;
        float v1 = t / vw1 + 1;

        sum += uw0 * vw0 * SampleShadowMap(base_uv, u0, v0, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw1 * vw0 * SampleShadowMap(base_uv, u1, v0, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw0 * vw1 * SampleShadowMap(base_uv, u0, v1, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw1 * vw1 * SampleShadowMap(base_uv, u1, v1, shadowMapSizeInv, lightDepth, uvScale, uvOffset);

        return sum * 1.0f / 16;

    #elif FilterSize == 5

        float uw0 = (4 - 3 * s);
        float uw1 = 7;
        float uw2 = (1 + 3 * s);

        float u0 = (3 - 2 * s) / uw0 - 2;
        float u1 = (3 + s) / uw1;
        float u2 = s / uw2 + 2;

        float vw0 = (4 - 3 * t);
        float vw1 = 7;
        float vw2 = (1 + 3 * t);

        float v0 = (3 - 2 * t) / vw0 - 2;
        float v1 = (3 + t) / vw1;
        float v2 = t / vw2 + 2;

        sum += uw0 * vw0 * SampleShadowMap(base_uv, u0, v0, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw1 * vw0 * SampleShadowMap(base_uv, u1, v0, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw2 * vw0 * SampleShadowMap(base_uv, u2, v0, shadowMapSizeInv, lightDepth, uvScale, uvOffset);

        sum += uw0 * vw1 * SampleShadowMap(base_uv, u0, v1, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw1 * vw1 * SampleShadowMap(base_uv, u1, v1, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw2 * vw1 * SampleShadowMap(base_uv, u2, v1, shadowMapSizeInv, lightDepth, uvScale, uvOffset);

        sum += uw0 * vw2 * SampleShadowMap(base_uv, u0, v2, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw1 * vw2 * SampleShadowMap(base_uv, u1, v2, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw2 * vw2 * SampleShadowMap(base_uv, u2, v2, shadowMapSizeInv, lightDepth, uvScale, uvOffset);

        return sum * 1.0f / 144;

    #else // FilterSize == 7

        float uw0 = (5 * s - 6);
        float uw1 = (11 * s - 28);
        float uw2 = -(11 * s + 17);
        float uw3 = -(5 * s + 1);

        float u0 = (4 * s - 5) / uw0 - 3;
        float u1 = (4 * s - 16) / uw1 - 1;
        float u2 = -(7 * s + 5) / uw2 + 1;
        float u3 = -s / uw3 + 3;

        float vw0 = (5 * t - 6);
        float vw1 = (11 * t - 28);
        float vw2 = -(11 * t + 17);
        float vw3 = -(5 * t + 1);

        float v0 = (4 * t - 5) / vw0 - 3;
        float v1 = (4 * t - 16) / vw1 - 1;
        float v2 = -(7 * t + 5) / vw2 + 1;
        float v3 = -t / vw3 + 3;

        sum += uw0 * vw0 * SampleShadowMap(base_uv, u0, v0, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw1 * vw0 * SampleShadowMap(base_uv, u1, v0, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw2 * vw0 * SampleShadowMap(base_uv, u2, v0, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw3 * vw0 * SampleShadowMap(base_uv, u3, v0, shadowMapSizeInv, lightDepth, uvScale, uvOffset);

        sum += uw0 * vw1 * SampleShadowMap(base_uv, u0, v1, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw1 * vw1 * SampleShadowMap(base_uv, u1, v1, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw2 * vw1 * SampleShadowMap(base_uv, u2, v1, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw3 * vw1 * SampleShadowMap(base_uv, u3, v1, shadowMapSizeInv, lightDepth, uvScale, uvOffset);

        sum += uw0 * vw2 * SampleShadowMap(base_uv, u0, v2, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw1 * vw2 * SampleShadowMap(base_uv, u1, v2, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw2 * vw2 * SampleShadowMap(base_uv, u2, v2, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw3 * vw2 * SampleShadowMap(base_uv, u3, v2, shadowMapSizeInv, lightDepth, uvScale, uvOffset);

        sum += uw0 * vw3 * SampleShadowMap(base_uv, u0, v3, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw1 * vw3 * SampleShadowMap(base_uv, u1, v3, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw2 * vw3 * SampleShadowMap(base_uv, u2, v3, shadowMapSizeInv, lightDepth, uvScale, uvOffset);
        sum += uw3 * vw3 * SampleShadowMap(base_uv, u3, v3, shadowMapSizeInv, lightDepth, uvScale, uvOffset);

        return sum * 1.0f / 2704;

    #endif
}