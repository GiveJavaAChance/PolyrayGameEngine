package polyraygameengine.cpurenderer;

public class RepeatTexture extends Texture {
    
    @Override
    public int getColor(float u, float v) {
        if(u < 0.0f || u > 1.0f) {
            u -= Math.floor(u);
        }
        int U = (int)(u * textureWidth);
        if(U < 0) {
            U = 0;
        } else if(U >= textureWidth) {
            U = textureWidth - 1;
        }
        if(v < 0.0f || v > 1.0f) {
            v -= Math.floor(v);
        }
        int V = (int)(v * textureHeight);
        if(V < 0) {
            V = 0;
        } else if(V >= textureHeight) {
            V = textureHeight - 1;
        }
        return texture[U + V * textureWidth];
    }
}
