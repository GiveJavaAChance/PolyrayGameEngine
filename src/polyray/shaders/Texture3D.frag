#version 420

#append "Constants.glsl";
#append "Environment.glsl";
#append "Camera3D.glsl";
#append "PBRLighting.glsl";
#append "ACESTonemap.glsl";
#append "GammaCorrect.glsl";
#append "Dither.glsl";

in vec2 uv;
in vec3 norm;
in vec3 pos;

uniform sampler2D textureSampler;

out vec4 fragColor;

void main() {
    vec4 col = texture(textureSampler, uv);
    
    vec3 normal = normalize(norm);

    vec3 viewDir = normalize(cameraPos - pos);

    vec3 color = PBRLighting(normal, viewDir, sunDir, sunColor * 5.0, col.rgb);

    color += ambientColor * col.rgb;
    
    color = ACESTonemap(color);
    color = gammaCorrect(color, 1.0);
    fragColor = ditherColor(vec4(color, col.w));
}
