package polyray.systems.jds;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

public class JDS {

    private static boolean READY = false;

    /**
     * Puts the JDS API.dll and the discord_partner_sdk.dll next to the java bin
     * directory. For standalone JRE's, it's "path\to\JRE\bin". For downloaded
     * java, it's "C:\Program Files\Java\jdk-{JAVA VERSION}\bin".
     *
     * @param javaPath Provide the path to the java bin directory.
     * @param discordPartnerSDK Provide the path to the discord socail SDK dll file.
     */
    public static void load(File javaPath, File discordPartnerSDK) {
        File sdkOut = new File(javaPath, discordPartnerSDK.getName());
        File jdsOut = new File(javaPath, "JDS API.dll");
        if (!sdkOut.exists()) try {
            BufferedInputStream in = new BufferedInputStream(new FileInputStream(discordPartnerSDK));
            BufferedOutputStream out = new BufferedOutputStream(new FileOutputStream(sdkOut));
            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = in.read(buffer)) != -1) {
                out.write(buffer, 0, bytesRead);
            }
        } catch (IOException e) {
        }
        if (!jdsOut.exists()) try {
            BufferedInputStream in = new BufferedInputStream(JDS.class.getResourceAsStream("JDS API.dll"));
            BufferedOutputStream out = new BufferedOutputStream(new FileOutputStream(jdsOut));
            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = in.read(buffer)) != -1) {
                out.write(buffer, 0, bytesRead);
            }
        } catch (IOException e) {
        }
        System.load(jdsOut.getAbsolutePath());
    }

    public static native void setup(long applicationId);

    public static native void setActivity(String state, String details);

    public static native void update();

    public static final boolean isReady() {
        return READY;
    }
}
