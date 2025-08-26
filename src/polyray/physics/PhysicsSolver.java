package polyray.physics;

import java.util.ArrayList;

public abstract class PhysicsSolver {

    private final double H;
    private double timeAccu;

    private final ArrayList<PhysicsObject3D> objects;

    public PhysicsSolver(double internalDt) {
        this.H = internalDt;
        this.timeAccu = 0.0d;
        this.objects = new ArrayList<>();
    }
    
    public int addObject(PhysicsObject3D o) {
        this.objects.add(o);
        return this.objects.size() - 1;
    }
    
    public void removeObject(PhysicsObject3D o) {
        this.objects.remove(o);
    }
    
    public void removeObject(int idx) {
        this.objects.remove(idx);
    }

    public void update(double dt) {
        this.timeAccu += dt;
        while (this.timeAccu >= H) {
            this.timeAccu -= H;
            updateGeneral(H);
            for (PhysicsObject3D obj : objects) {
                updateObject(obj, H);
            }
        }
    }
    
    public abstract void updateObject(PhysicsObject3D obj, double dt);
    public abstract void updateGeneral(double dt);
}
