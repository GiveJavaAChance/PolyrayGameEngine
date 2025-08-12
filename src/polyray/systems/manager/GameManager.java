package polyray.systems.manager;

import polyray.systems.FixedUpdate;

public class GameManager {

    public static final FixedUpdate physicsUpdate = new FixedUpdate(0.006d, 0.003d);
    
    static {
        physicsUpdate.addUpdate(ParticleManager::update);
    }
    
    public static final void update(double dt) {
        physicsUpdate.update(dt);
        ParticleManager.updateInstances();
    }
}
