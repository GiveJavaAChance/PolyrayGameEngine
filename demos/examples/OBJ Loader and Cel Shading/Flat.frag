#version 420

#append "Constants.glsl";
#append "Environment.glsl";
#append "PBRLighting.glsl";
#append "Camera3D.glsl";
#append "ACESTonemap.glsl";
#append "GammaCorrect.glsl";
#append "Dither.glsl";

vec3 PBRLighting2(vec3 normal, vec3 viewDir, vec3 lightDir, vec3 lightColor, vec3 albedo) {
    vec3 H = normalize(viewDir + lightDir);
    vec3 F = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);

    float D = distributionGGX(normal, H, roughness);
    float G = geometrySchlickGGX(max(dot(normal, viewDir), 0.0), roughness) *
              geometrySchlickGGX(max(dot(normal, lightDir), 0.0), roughness);

    vec3 numerator = D * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(normal, lightDir), 0.0);

    // **Threshold the diffuse lighting**
    float lightThreshold = 0.1;
    NdotL = NdotL > lightThreshold ? 1.0 : 0.0;

    // **Threshold the specular reflection**
    float specularThreshold = 0.5;
    specular = length(specular) > specularThreshold ? specular : vec3(0.0);

    vec3 diffuse = kD * albedo / PI;

    // **Grazing-angle glow effect (reflection boost)**
    float grazingEffect = smoothstep(0.3, 0.4, (1.0 - NdotL) * pow(1.0 - max(dot(normal, viewDir), 0.0), 5.0));
    vec3 grazingGlow = ambientColor * grazingEffect; // Glow stronger at shallow angles

    return (diffuse * NdotL + specular) * lightColor + grazingGlow;
}

vec3 PBRLighting1(vec3 normal, vec3 viewDir, vec3 lightDir, vec3 lightColor, vec3 albedo) {
    vec3 H = normalize(viewDir + lightDir);
    vec3 F = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);

    float D = distributionGGX(normal, H, roughness);
    float G = geometrySchlickGGX(max(dot(normal, viewDir), 0.0), roughness) *
              geometrySchlickGGX(max(dot(normal, lightDir), 0.0), roughness);

    vec3 numerator = D * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(normal, lightDir), 0.0);

    float lightThreshold = 0.1;
    NdotL = NdotL > lightThreshold ? 1.0 : 0.0;

    float specularThreshold = 0.5;
    specular = length(specular) > specularThreshold ? specular : vec3(0.0);

    vec3 diffuse = kD * albedo / PI;
    return (diffuse * NdotL + specular) * lightColor;
}

vec3 saturate(vec3 color, float amt) {
    return mix(vec3(dot(color, vec3(0.2126, 0.7152, 0.0722))), color, amt);
}

in vec2 uv;
in vec3 norm;
in vec3 pos;

uniform sampler2D textureSampler;

out vec4 fragColor;

void main() {
    vec4 col = texture(textureSampler, uv);
    if(col.w < 0.9) {
        discard;
    }
    
    vec3 normal = normalize(norm);

    vec3 viewDir = normalize(cameraPos - pos);

    vec3 color = PBRLighting2(normal, viewDir, sunDir, sunColor * 5.0, col.rgb);

    color += ambientColor * col.rgb;
    
    color = ACESTonemap(color);
    color = gammaCorrect(color, 1.0);
    fragColor = vec4(color, col.w);
}
