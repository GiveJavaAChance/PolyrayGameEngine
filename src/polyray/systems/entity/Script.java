package polyray.systems.entity;

public interface Script extends Component {

    public void frameUpdate(double dt);

    public void update(double dt);
}
