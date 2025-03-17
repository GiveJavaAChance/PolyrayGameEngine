#append "Constants.glsl";

// Using Inigo quilez implementatioan of 2D noise and expanded to 3D as well

float ramp(float v) {
    return v * v * (3.0 - 2.0 * v);
}

float getVal2D(ivec2 v) {
    vec2 uv = 50.0 * fract(v * INVERSE_PI);
    return 2.0 * fract(uv.x * uv.y * (uv.x + uv.y)) - 1.0;
}

float noise2D(vec2 v) {
    ivec2 i = ivec2(floor(v));
    vec2 uv = vec2(ramp(v.x - i.x), ramp(v.y - i.y));
    float a = getVal2D(i);
    float b = getVal2D(i + ivec2(1, 0));
    float c = getVal2D(i + ivec2(0, 1));
    float d = getVal2D(i + ivec2(1));
    float x1 = mix(a, b, uv.x);
    float x2 = mix(c, d, uv.x);
    return mix(x1, x2, uv.y);
}

float getVal3D(ivec3 v) {
    vec3 uv = 50.0 * fract(vec3(v) * INVERSE_PI);
    return 2.0 * fract(uv.x * uv.y * uv.z * (uv.x + uv.y + uv.z)) - 1.0;
}

float noise3D(vec3 v) {
    ivec3 i = ivec3(floor(v));
    vec3 uv = vec3(ramp(v.x - i.x), ramp(v.y - i.y), ramp(v.z - i.z));
    float n000 = getVal3D(i);
    float n001 = getVal3D(i + ivec3(0, 0, 1));
    float n010 = getVal3D(i + ivec3(0, 1, 0));
    float n011 = getVal3D(i + ivec3(0, 1, 1));
    float n100 = getVal3D(i + ivec3(1, 0, 0));
    float n101 = getVal3D(i + ivec3(1, 0, 1));
    float n110 = getVal3D(i + ivec3(1, 1, 0));
    float n111 = getVal3D(i + ivec3(1, 1, 1));
    float x00 = mix(n000, n100, uv.x);
    float x01 = mix(n001, n101, uv.x);
    float x10 = mix(n010, n110, uv.x);
    float x11 = mix(n011, n111, uv.x);
    float y0 = mix(x00, x10, uv.y);
    float y1 = mix(x01, x11, uv.y);
    return mix(y0, y1, uv.z);
}