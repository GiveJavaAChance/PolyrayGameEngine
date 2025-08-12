package polyray.physics;

public interface Collider3D {

    public boolean collide(Collider3D other, double dt);

    public void getBounds(float[] box);
}
