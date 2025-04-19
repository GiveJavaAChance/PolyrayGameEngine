package polyray.modular;

import polyray.GLTexture;

public abstract class PostProcessor {

    protected final GLTexture source, target;
    
    public PostProcessor(GLTexture source, GLTexture target) {
        this.source = source;
        this.target = target;
    }
    
    public abstract void process();
    
}
