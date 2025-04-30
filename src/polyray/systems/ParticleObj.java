package polyray.systems;

import polyray.Vector3d;
import polyray.modular.Instance;
import polyray.physics.PhysicsObject;

public class ParticleObj<T extends Instance> extends PhysicsObject {
    
    public int time;
    
    public T instance;
    
    public ParticleObj(int time, Vector3d pos, float friction, float restitution) {
        super(pos, friction, restitution);
        this.time = time;
    }
    
    public void tick() {
        
    }
    
    public void updateInstance() {
    }
}
