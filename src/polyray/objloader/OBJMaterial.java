package polyray.objloader;

import polyray.Material;
import polyray.Vector3f;

public class OBJMaterial {

    public String name;

    public Vector3f Kd;         // Diffuse color
    public Vector3f Ks;         // Specular color
    public Vector3f Ke;         // Emissive color
    public Vector3f Ka;         // Ambient color
    public float Ns;            // Shininess (specular exponent)
    public float Ni;            // Index of refraction
    public float d;             // Dissolve factor (opacity)
    public int illum;           // Illumination model
    public String mapKd;        // Diffuse texture map
    public String mapNs;        // Specular texture map
    public String mapRefl;      // Reflection map
    public String mapBump;      // Bump map

    public OBJMaterial(String name) {
        this.name = name;
        this.Kd = new Vector3f(1.0f, 1.0f, 1.0f);
        this.Ks = new Vector3f(0.0f, 0.0f, 0.0f);
        this.Ke = new Vector3f(0.0f, 0.0f, 0.0f);
        this.Ka = new Vector3f(1.0f, 1.0f, 1.0f);
        this.Ns = 0.0f;
        this.Ni = 1.0f;
        this.d = 1.0f;
        this.illum = 2;
    }

    public static void toPBR(OBJMaterial objMat, Material mat) {
        mat.setRoughness(Math.max(1.0f - (objMat.Ns / 1000.0f), 0.02f));
        float KsStrength = (objMat.Ks.x + objMat.Ks.y + objMat.Ks.z) / 3.0f;
        float metallic = (objMat.illum >= 3 || KsStrength > 0.5f) ? 1.0f : 0.0f;
        mat.setMetallic(metallic);
        if (metallic > 0.5f) {
            mat.setF0(objMat.Ks);
        } else {
            float f0 = (objMat.Ni - 1.0f) / (objMat.Ni + 1.0f);
            f0 *= f0;
            mat.setF0(new Vector3f(f0, f0, f0));
        }
    }
}
