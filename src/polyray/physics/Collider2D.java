package polyray.physics;

public interface Collider2D {

    public boolean collide(Collider2D other, double dt);

    public void getBounds(float[] box);
}
