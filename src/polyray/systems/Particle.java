package polyray.systems;

import polyray.modular.Instance;

public abstract class Particle<T extends Instance> {

    public int time;

    public T instance;

    public Particle(int time) {
        this.time = time;
    }

    public abstract void tickUpdate();

    public abstract void frameUpdate(double dt);

    public abstract void physicsUpdate(double dt);

    public abstract void updateInstance();
}
