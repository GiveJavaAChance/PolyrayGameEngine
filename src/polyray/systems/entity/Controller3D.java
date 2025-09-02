package polyray.systems.entity;

import polyray.Vector3d;

public abstract class Controller3D {

    protected Vector3d pos, prevPos;

    public abstract void update(double dt);
}
