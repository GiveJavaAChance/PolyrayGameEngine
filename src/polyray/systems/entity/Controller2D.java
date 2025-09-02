package polyray.systems.entity;

import polyray.Vector2d;

public abstract class Controller2D {

    protected Vector2d pos, prevPos;
    
    public abstract void update(double dt);
}
