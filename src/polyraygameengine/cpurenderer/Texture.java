package polyraygameengine.cpurenderer;

public class Texture {
    public int[] texture;
    public int textureWidth, textureHeight;
    
    public int getColor(float u, float v) {
        int U = (int)(u * textureWidth);
        if(U < 0) {
            U = 0;
        } else if(U >= textureWidth) {
            U = textureWidth - 1;
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
