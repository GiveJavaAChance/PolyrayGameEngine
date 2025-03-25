package polyray.audio;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class NativeDCDBREffect implements Effect {

    static {
        try {
            File tempFile = File.createTempFile("DCDBRNative", ".dll");
            tempFile.deleteOnExit();
            try ( FileOutputStream out = new FileOutputStream(tempFile);  InputStream in = NativeDCDBREffect.class.getResourceAsStream("DCDBRNative.dll")) {
                byte[] buffer = new byte[4096];
                int bytesRead;
                while ((bytesRead = in.read(buffer)) != -1) {
                    out.write(buffer, 0, bytesRead);
                }
            }
            System.load(tempFile.getAbsolutePath());
        } catch (IOException e) {
        }
    }

    private native void setCollectorSize(int length);

    private native void applyDBR(float[] buffer, float[] rightChannel, float[] leftChannel, int bufferSize);

    private native void setSamples(float[] volumesRight, float[] volumesLeft, int[] delays, int numSamples);

    private final int bufferSize;
    private final float[] rightChannel, leftChannel;

    public NativeDCDBREffect(int maxDelay, int bufferSize) {
        setCollectorSize(maxDelay + bufferSize);
        this.bufferSize = bufferSize;
        this.rightChannel = new float[bufferSize];
        this.leftChannel = new float[bufferSize];
    }

    public void setSampleData(float[] volumesRight, float[] volumesLeft, int[] delays, int numSamples) {
        setSamples(volumesRight, volumesLeft, delays, numSamples);
    }

    @Override
    public void computeRight(int[] channel) {
        float[] buffer = new float[bufferSize];
        for (int i = 0; i < bufferSize; i++) {
            buffer[i] = (float) channel[i];
        }
        applyDBR(buffer, rightChannel, leftChannel, bufferSize);
        for (int i = 0; i < bufferSize; i++) {
            channel[i] = (int) rightChannel[i];
        }
    }

    @Override
    public void computeLeft(int[] channel) {
        for (int i = 0; i < bufferSize; i++) {
            channel[i] = (int) leftChannel[i];
        }
    }

    @Override
    public void onStart() {
    }

    @Override
    public void onFinnish() {

    }
}
