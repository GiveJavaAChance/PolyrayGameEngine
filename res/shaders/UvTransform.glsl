struct UvTransform {
    vec2 offset;
    vec2 scale;
    float rotation;
};

vec2 transformUv(vec2 uv, UvTransform tx) {
    float s = sin(tx.rotation);
    float c = cos(tx.rotation);
    mat2 rotationMat = mat2(
        c, -s,
        s, c
    );
    return rotationMat * uv * tx.scale + tx.offset;
}