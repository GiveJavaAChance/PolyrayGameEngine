float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}

// This is not the best, but it works to some extent
vec4 ditherColor(vec4 color) {
    float thresholdR = fract(color.r * 255.0);
    float thresholdG = fract(color.g * 255.0);
    float thresholdB = fract(color.b * 255.0);

    float seedR = hash(gl_FragCoord.x + gl_FragCoord.y * 1.1763);
    float seedG = hash(gl_FragCoord.x + gl_FragCoord.y * 2.8765);
    float seedB = hash(gl_FragCoord.x + gl_FragCoord.y * 3.2906);

    if (seedR > thresholdR) {
        color.r = min(color.r + 1.0 / 255.0, 1.0);
    }
    if (seedG > thresholdG) {
        color.g = min(color.g + 1.0 / 255.0, 1.0);
    }
    if (seedB > thresholdB) {
        color.b = min(color.b + 1.0 / 255.0, 1.0);
    }
    return color;
}