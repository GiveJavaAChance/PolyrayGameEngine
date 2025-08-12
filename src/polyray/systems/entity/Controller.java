package polyray.systems.entity;

@FunctionalInterface
public interface Controller extends Component {

    public void update(Entity e, double dt);
}
