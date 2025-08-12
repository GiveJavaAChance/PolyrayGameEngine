package polyray.physics;

public interface Collider {

    public boolean collide(Collider other, double dt);

    public void getBounds(float[] box);
}
