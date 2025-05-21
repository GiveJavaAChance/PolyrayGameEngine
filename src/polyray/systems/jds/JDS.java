package polyray.systems.jds;

public class JDS {

    static {
        
    }
    
    private static boolean READY = false;

    public static native void setup(long applicationId);

    public static native void setActivity(String state, String details);
    
    public static native void update();
    
    public static final boolean isReady() {
        return READY;
    }
}
