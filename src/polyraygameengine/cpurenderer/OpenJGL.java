package polyraygameengine.cpurenderer;

import java.util.ArrayList;
import polyraygameengine.misc.Rotator;
import polyraygameengine.misc.Vector3f;

public class OpenJGL {

    public static final int PRIOAXIS_X = 0, PRIOAXIS_Y = 1, PRIOAXIS_Z = 2;
    public float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 0.0f, cameraDist = 100.0f, minRendDist = 1.0f, renderDist = 1000.0f;
    private float preCameraDepthZ;
    public int prioAxis = 2;
    private Vector3f ang = new Vector3f(0.0f, 0.0f, 0.0f);
    private final double[] rotationMatrix = new double[6];
    public ArrayList<Triangle> triangles = new ArrayList<>();
    private Vector3f translate = new Vector3f();

    private static final Rotator rot = new Rotator();

    public OpenJGL() {
        updateMatrix();
    }

    public void addTriangle(Triangle t) {
        triangles.add(t);
    }

    public void setup(float minRendDist, float renderDist) {
        if (minRendDist < 0.1f) {
            minRendDist = 0.1f;
        }
        if (renderDist < 0.1f) {
            renderDist = 0.1f;
        }
        this.minRendDist = minRendDist;
        this.renderDist = renderDist;
    }

    public void setCameraPosition(float x, float y, float z, float dist, float windowW, float windowH) {
        this.cameraX = x;
        this.cameraY = y;
        this.cameraZ = z;
        this.cameraDist = dist;
        this.preCameraDepthZ = dist - z;
    }

    public static OpenJGL createDefaultRenderer(CPURenderer r, float dist, float FOV) {
        OpenJGL u = new OpenJGL();
        u.setup(0.1f, 1000.0f);
        u.setCameraPosition(-r.width / 2, -r.height / 2, (r.width - dist) / FOV, r.width / FOV, r.width, r.height);
        u.prioritizeAxis(0);
        return u;
    }

    public void rotate(Vector3f ang) {
        this.ang = Vector3f.add(this.ang, ang);
        updateMatrix();
    }

    public void rotateX(double ang) {
        this.ang = new Vector3f(this.ang.x + (float) ang, this.ang.y, this.ang.z);
        updateMatrixX();
    }

    public void rotateY(double ang) {
        this.ang = new Vector3f(this.ang.x, this.ang.y + (float) ang, this.ang.z);
        updateMatrixY();
    }

    public void rotateZ(double ang) {
        this.ang = new Vector3f(this.ang.x, this.ang.y, this.ang.z + (float) ang);
        updateMatrixZ();
    }

    public void setAngle(Vector3f ang) {
        this.ang = ang;
        updateMatrix();
    }

    private void updateMatrixX() {
        rotationMatrix[0] = Math.sin(ang.x);
        rotationMatrix[1] = Math.cos(ang.x);
    }

    private void updateMatrixY() {
        rotationMatrix[2] = Math.sin(ang.y);
        rotationMatrix[3] = Math.cos(ang.y);
    }

    private void updateMatrixZ() {
        rotationMatrix[4] = Math.sin(ang.z);
        rotationMatrix[5] = Math.cos(ang.z);
    }

    private void updateMatrix() {
        rotationMatrix[0] = Math.sin(ang.x);
        rotationMatrix[1] = Math.cos(ang.x);
        rotationMatrix[2] = Math.sin(ang.y);
        rotationMatrix[3] = Math.cos(ang.y);
        rotationMatrix[4] = Math.sin(ang.z);
        rotationMatrix[5] = Math.cos(ang.z);
    }

    public void setTranslate(Vector3f translate) {
        this.translate = translate;
    }

    public void prioritizeAxis(int axis) {
        prioAxis = axis;
    }

    public void render(CPURenderer r) {
        for (Triangle t : triangles) {
            Vector3f ra = applyRotation(Vector3f.add(t.a, translate));
            Vector3f rb = applyRotation(Vector3f.add(t.b, translate));
            Vector3f rc = applyRotation(Vector3f.add(t.c, translate));

            Vector3f ro = null;

            float aDepth = preCameraDepthZ - ra.z;
            float bDepth = preCameraDepthZ - rb.z;
            float cDepth = preCameraDepthZ - rc.z;
            
            if(aDepth > renderDist && bDepth > renderDist && cDepth > renderDist) {
                continue;
            }
            
            boolean za = aDepth > minRendDist;
            boolean zb = bDepth > minRendDist;
            boolean zc = cDepth > minRendDist;
            int caseID = (za ? 4 : 0) | (zb ? 2 : 0) | (zc ? 1 : 0);
            boolean ca = false;
            switch (caseID) {
                case 0 -> {
                    // 000 - All vertices are outside (skip)
                    continue;
                }
                case 1 -> {

                    // 001 - Only C is inside
                    ra = tryProject(ra, rc); // Clip edge AB
                    rb = tryProject(rb, rc); // Clip edge BC
                }
                case 2 -> {
                    // 010 - Only B is inside
                    ra = tryProject(ra, rb); // Clip edge AB
                    rc = tryProject(rc, rb); // Clip edge BC
                }
                case 3 -> {// 011 - B and C are inside
                    Vector3f tmp = tryProject(ra, rb); // Clip edge AB
                    ro = tryProject(ra, rc);
                    ra = tmp;
                    ca = false;
                }
                case 4 -> {
                    // 100 - Only A is inside
                    rb = tryProject(rb, ra); // Clip edge AB
                    rc = tryProject(rc, ra); // Clip edge AC
                }

                case 5 -> {
                    // 101 - A and C are inside
                    Vector3f tmp = tryProject(rb, ra); // Clip edge AB
                    ro = tryProject(rb, rc);
                    rb = tmp;
                    ca = true;
                }

                case 6 -> {
                    // 110 - A and B are inside
                    Vector3f tmp = tryProject(rc, ra); // Clip edge AC
                    ro = tryProject(rc, rb);
                    rc = tmp;
                    ca = true;
                }

                case 7 -> {
                    // 111 - All vertices are inside (no clipping needed)
                }
            }
            Vector3f a = project2D(ra);
            Vector3f b = project2D(rb);
            Vector3f c = project2D(rc);
            if (t instanceof TextureTriangle tex) {
                r.fillTexturedTriangle((int) a.x, (int) a.y, a.z, tex.u0, tex.v0, (int) b.x, (int) b.y, b.z, tex.u1, tex.v1, (int) c.x, (int) c.y, c.z, tex.u2, tex.v2, tex.texture);
                if (ro == null) {
                    continue;
                }
                Vector3f o = project2D(ro);
                if (ca) {
                    r.fillTexturedTriangle((int) o.x, (int) o.y, o.z, tex.u0, tex.v0, (int) c.x, (int) c.y, c.z, tex.u1, tex.v1, (int) b.x, (int) b.y, b.z, tex.u2, tex.v2, tex.texture);
                } else {
                    r.fillTexturedTriangle((int) o.x, (int) o.y, o.z, tex.u0, tex.v0, (int) a.x, (int) a.y, a.z, tex.u1, tex.v1, (int) c.x, (int) c.y, c.z, tex.u2, tex.v2, tex.texture);
                }
            } else {
                r.fillTriangle((int) a.x, (int) a.y, a.z, (int) b.x, (int) b.y, b.z, (int) c.x, (int) c.y, c.z, t.color);
                if (ro == null) {
                    continue;
                }
                Vector3f o = project2D(ro);
                if (ca) {
                    r.fillTriangle((int) o.x, (int) o.y, o.z, (int) c.x, (int) c.y, c.z, (int) b.x, (int) b.y, b.z, t.color);
                } else {
                    r.fillTriangle((int) o.x, (int) o.y, o.z, (int) a.x, (int) a.y, a.z, (int) c.x, (int) c.y, c.z, t.color);
                }
            }
        }
    }

    private Vector3f tryProject(Vector3f pos, Vector3f to) {
        float t = preCameraDepthZ - pos.z;
        if (t < minRendDist) {
            if(preCameraDepthZ - to.z < minRendDist) {
                return null;
            }
            Vector3f delta = Vector3f.sub(pos, to);
            delta = Vector3f.div(delta, delta.z);
            float k = minRendDist - t;
            
            return Vector3f.sub(pos, Vector3f.mul(delta, k));
        }
        return pos;
    }

    private Vector3f project2D(Vector3f v) {
        float t = preCameraDepthZ - v.z;
        if (t > 0.001f) {
            t = cameraDist / t;
            float x1 = v.x * t - cameraX;
            float y1 = v.y * t - cameraY;
            return new Vector3f(x1, y1, -v.z);
        }
        return null;
    }

    public float calcZDepth(Vector3f v) {
        return preCameraDepthZ - applyRotation(v).z;
    }

    private Vector3f applyRotation(Vector3f v) {
        switch (prioAxis) {
            case 0 -> {
                // Y
                Vector3f out = new Vector3f(
                        (float) (rotationMatrix[3] * v.x + rotationMatrix[2] * v.z),
                        v.y,
                        (float) (rotationMatrix[3] * v.z - rotationMatrix[2] * v.x)
                );

                // Z
                float nx = (float) (rotationMatrix[5] * out.x - rotationMatrix[4] * out.y);
                out.y = (float) (rotationMatrix[4] * out.x + rotationMatrix[5] * out.y);
                out.x = nx;

                // X
                float ny = (float) (rotationMatrix[1] * out.y - rotationMatrix[0] * out.z);
                out.z = (float) (rotationMatrix[0] * out.y + rotationMatrix[1] * out.z);
                out.y = ny;
                return out;
            }

            case 1 -> {
                // Z
                Vector3f out = new Vector3f(
                        (float) (rotationMatrix[5] * v.x - rotationMatrix[4] * v.y),
                        (float) (rotationMatrix[4] * v.x + rotationMatrix[5] * v.y),
                        v.z
                );

                // X
                float ny = (float) (rotationMatrix[1] * out.y - rotationMatrix[0] * out.z);
                out.z = (float) (rotationMatrix[0] * out.y + rotationMatrix[1] * out.z);
                out.y = ny;
                // Y
                float nx = (float) (rotationMatrix[3] * out.x + rotationMatrix[2] * out.z);
                out.z = (float) (rotationMatrix[3] * out.z - rotationMatrix[2] * out.x);
                out.x = nx;
                return out;
            }

            case 2 -> {
                // X
                Vector3f out = new Vector3f(
                        v.x,
                        (float) (rotationMatrix[1] * v.y - rotationMatrix[0] * v.z),
                        (float) (rotationMatrix[0] * v.y + rotationMatrix[1] * v.z)
                );

                // Y
                float nx = (float) (rotationMatrix[3] * out.x + rotationMatrix[2] * out.z);
                out.z = (float) (rotationMatrix[3] * out.z - rotationMatrix[2] * out.x);
                out.x = nx;
                // Z
                nx = (float) (rotationMatrix[5] * out.x - rotationMatrix[4] * out.y);
                out.y = (float) (rotationMatrix[4] * out.x + rotationMatrix[5] * out.y);
                out.x = nx;
                return out;
            }
        }
        return new Vector3f();
    }

    public Vector3f calcInverseRot(float x, float y, float z) {
        return rot.calcInverseRot(new Vector3f(x, y, z), ang, prioAxis);
    }

    public Vector3f calcInverseRot(Vector3f v) {
        return rot.calcInverseRot(v, ang, prioAxis);
    }
}
