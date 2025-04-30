package polyray.systems.entity;

import java.io.BufferedOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import polyray.Vector3f;
import polyray.modular.Instance;
import polyray.multiplayer.MultiplayerPacket;
import polyray.multiplayer.ServerStream;
import polyray.physics.PhysicsObject;

public class Entity<T extends Instance> extends PhysicsObject implements MultiplayerPacket {

    public boolean dead;
    public int deathTime = 100;

    public double entityHeight = 2.0d;

    public T instance;

    public int health, maxHealth;

    public Vector3f viewDir;

    public String name = "";

    public Entity() {
        super(0.05f, 0.0f);
        this.viewDir = new Vector3f(1.0f, 0.0f, 0.0f);
    }

    public Entity(int health, int maxHealth, String name) {
        this();
        this.health = health;
        this.maxHealth = maxHealth;
        this.name = name;
    }

    public void updateInstance() {
    }

    public void set(Entity e) {
        this.name = e.name;
        this.health = e.health;
        this.maxHealth = e.maxHealth;
        this.dead = e.dead;
        this.deathTime = e.deathTime;
        this.pos.x = e.pos.x;
        this.pos.y = e.pos.y;
        this.pos.z = e.pos.z;
        this.prevPos.x = e.prevPos.x;
        this.prevPos.y = e.prevPos.y;
        this.prevPos.z = e.prevPos.z;
        this.acc.x = e.acc.x;
        this.acc.y = e.acc.y;
        this.acc.z = e.acc.z;
        this.viewDir.x = e.viewDir.x;
        this.viewDir.y = e.viewDir.y;
        this.viewDir.z = e.viewDir.z;
    }

    @Override
    public void applyGround(double dt, double height, Vector3f normal) {

        double x = pos.x;
        double y = pos.y;
        double z = pos.z;
        //super.applyGround(dt, TerrainGeneration.getHeight(x, z) + entityHeight, TerrainGeneration.getNormal(x, z));

        height += entityHeight;
        Vector3f v = new Vector3f((float) (x - prevPos.x), (float) (y - prevPos.y), (float) (z - prevPos.z));
        if (y < height) {
            double diff = height - y;
            Vector3f resolve = Vector3f.mul(normal, (float) diff);
            pos.x += resolve.x;
            pos.y += resolve.y;
            pos.z += resolve.z;
            if (Vector3f.dot(v, normal) > 0.0f) {
                return;
            }
            Vector3f vel = bounce(v, normal);
            prevPos.x = pos.x - vel.x;
            prevPos.y = pos.y - vel.y;
            prevPos.z = pos.z - vel.z;
        }
        if (y < height + 0.1d) {
            if (v.x * v.x + v.z * v.z < 0.0000001d) {
                pos.x = x;
                pos.z = z;
                prevPos.x = x;
                prevPos.z = z;
                return;
            }
            float f = 1.0f - friction;
            prevPos.x = pos.x - v.x * f;
            prevPos.z = pos.z - v.z * f;
        }
    }

    @Override
    protected Vector3f bounce(Vector3f v, Vector3f n) {
        float height = -Vector3f.dot(v, n);
        Vector3f normVel = Vector3f.mul(n, height);
        Vector3f perp = Vector3f.add(v, normVel);
        return Vector3f.add(perp, Vector3f.mul(normVel, restitution));
    }

    @Override
    public void write(BufferedOutputStream out) throws IOException {
        ByteBuffer buffer = ByteBuffer.allocate(1 + 12 + 8 * 6 + 4 * 3 + 4 + name.length()).order(ByteOrder.BIG_ENDIAN);
        buffer.put((byte) (dead ? 1 : 0));
        buffer.putInt(deathTime);
        buffer.putInt(health);
        buffer.putInt(maxHealth);
        buffer.putDouble(pos.x);
        buffer.putDouble(pos.y);
        buffer.putDouble(pos.z);
        buffer.putDouble(prevPos.x);
        buffer.putDouble(prevPos.y);
        buffer.putDouble(prevPos.z);
        buffer.putFloat(viewDir.x);
        buffer.putFloat(viewDir.y);
        buffer.putFloat(viewDir.z);
        buffer.putInt(name.length());
        buffer.put(name.getBytes(StandardCharsets.ISO_8859_1));
        out.write(buffer.array());
    }

    @Override
    public void read(ServerStream in) throws IOException {
        ByteBuffer buffer = in.read(1 + 12 + 8 * 6 + 4 * 3 + 4).order(ByteOrder.BIG_ENDIAN);
        this.dead = buffer.get() == 1;
        this.deathTime = buffer.getInt();
        this.health = buffer.getInt();
        this.maxHealth = buffer.getInt();
        this.pos.x = buffer.getDouble();
        this.pos.y = buffer.getDouble();
        this.pos.z = buffer.getDouble();
        this.prevPos.x = buffer.getDouble();
        this.prevPos.y = buffer.getDouble();
        this.prevPos.z = buffer.getDouble();
        this.viewDir.x = buffer.getFloat();
        this.viewDir.y = buffer.getFloat();
        this.viewDir.z = buffer.getFloat();
        int nameLength = buffer.getInt();
        this.name = new String(in.read(nameLength).array(), StandardCharsets.ISO_8859_1);
    }
}
