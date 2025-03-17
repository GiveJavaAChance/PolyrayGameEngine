package polyray;

public class Material {

    private final ShaderProgram shader;

    public Material(int cameraTransformBinding, int environmentBinding) {
        ShaderPreprocessor proc = ShaderPreprocessor.fromLocalFiles("Texture3D.vert", "Texture3D.frag");
        proc.appendAll();
        proc.setInt("CAM3D_IDX", cameraTransformBinding);
        proc.setInt("ENV_IDX", environmentBinding);
        this.shader = proc.createProgram("texture3d", 0);
    }

    public Material(ShaderProgram shader) {
        this.shader = shader;
    }

    public void setMetallic(float metallic) {
        shader.use();
        shader.setUniform("metallic", metallic);
        shader.unuse();
    }

    public void setRoughness(float roughness) {
        shader.use();
        shader.setUniform("roughness", roughness);
        shader.unuse();
    }

    public void setF0(Vector3f F0) {
        shader.use();
        shader.setUniform("F0", F0.x, F0.y, F0.z);
        shader.unuse();
    }

    public ShaderProgram getShader() {
        return this.shader;
    }
}
