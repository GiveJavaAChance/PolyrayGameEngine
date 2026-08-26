#version 430
                                      
#append "shaders/Environment.glsl";

#append "shaders/Light.glsl";

in vec3 rayDir;

out vec4 color;

float directionalIntensity(vec3 dir, vec3 lightDir) {
    float dot = max(dot(dir, lightDir), 0.0);
    float thresh = 0.999;
    float solid = min(max(dot - thresh, 0.0) / (1.0 - thresh) * 10.0, 1.0);
    return pow(dot, 100.0) + solid * 5.0;
}

void main() {
    vec3 dir = normalize(rayDir);
    vec3 col = ambientColor * min(1.0 - dir.y * 0.5, 1.0);
    col = mix(col, vec3(0.3), clamp(-dir.y * 50.0, 0.0, 1.0));
    for(uint i = 0u; i < directionalLight3DCount; i++) {
        DirectionalLight3D light = directionalLights3D[i];
        col += directionalIntensity(dir, light.dir) * light.color * light.strength;
    }
    color = vec4(col, 1.0);
}