package polyray.examples;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.text.DecimalFormat;
import java.util.ArrayList;
import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.SourceDataLine;
import javax.sound.sampled.TargetDataLine;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JPanel;
import polyray.FFTV2;
import polyray.audio.DBREffect;
import polyray.audio.SoundEffector;

public class AudioEffectsAndFFTExample extends JPanel {

    public static final DecimalFormat dc = new DecimalFormat("#.##");

    public static JFrame frame;
    public FFTV2 g2;
    public float[] freq;
    public int[] DATA;
    public ArrayList<float[]> heights = new ArrayList<>();
    public float[] h;

    public SoundEffector effector = new SoundEffector(2, 4);

    public BufferedImage render = new BufferedImage(1920, 1080, BufferedImage.TYPE_INT_ARGB);
    public BufferedImage buffer = new BufferedImage(1920, 1080, BufferedImage.TYPE_INT_ARGB);

    public boolean MIC = false;

    public AudioEffectsAndFFTExample() {
        // Switch between different acoustics:
        // Large Cave
        //effector.addEffect(new VolumeModifier(new DCDBREffect(10000, 4096 * 32, 1024, 2.0d), 0.005f), 0);

        // Outside a room
        effector.addEffect(DBREffect.loadDBRData("samples.dat", 0.1f, 1.5f, 1024), 0);

        setBackground(new Color(0, 0, 0, 0));
        new Thread(() -> {
            JFileChooser fc = new JFileChooser();
            fc.setMultiSelectionEnabled(true);
            fc.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
            while (true) {
                if(fc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION) {
                    System.exit(0);
                }
                ArrayList<File> files = new ArrayList<>();
                for (File f : fc.getSelectedFiles()) {
                    if (f.isDirectory()) {
                        File[] fi = f.listFiles();
                        if (fi == null || fi.length == 0) {
                            continue;
                        }
                        for (File file : fi) {
                            if (file.getName().endsWith(".wav")) {
                                files.add(file);
                            }
                        }
                    } else if (f.getName().endsWith(".wav")) {
                        files.add(f);
                    }
                }
                for (File f : files) {
                    try {
                        int sampleRate = 44100;
                        AudioFormat format = new AudioFormat(AudioFormat.Encoding.PCM_SIGNED, sampleRate, 16, 2, 4, sampleRate, false);
                        InputStream in = getInput(f, format);

                        float time = SoundEffector.getDelay(format, 4096);
                        g2 = new FFTV2(1000, time);

                        DataLine.Info info = new DataLine.Info(SourceDataLine.class, format);
                        SourceDataLine out = (SourceDataLine) AudioSystem.getLine(info);
                        out.open(format);
                        out.start();

                        byte[] buffer = new byte[4096];
                        int bytesRead;
                        while ((bytesRead = in.read(buffer)) != -1) {

                            long startTime = System.nanoTime();
                            buffer = effector.nextBuffer(buffer, time, true);
                            DATA = effector.getRightChannel();
                            System.out.printf("Effect Time: %sms\n", dc.format((System.nanoTime() - startTime) / 1000000.0f));

                            out.write(buffer, 0, bytesRead);

                            int smoothing = 1;
                            g2.doLogarithmicFFT(DATA, 32767, 0.0f, 20000.0f, 100.0f, 10.0f, false);
                            freq = g2.getAmp();
                            heights.add(freq);
                            if (heights.size() > smoothing) {
                                heights.remove(0);
                            }
                            h = new float[freq.length];
                            float mul = 1.0f / 10000.0f / heights.size();
                            for (int j = 0; j < freq.length; j++) {
                                float hc = 0.0f;
                                for (float[] height : heights) {
                                    hc += height[j];
                                }
                                h[j] = hc * mul;
                            }
                            render();
                            repaint();
                            while (!(out.getBufferSize() - out.available() < buffer.length)) {
                            }
                        }
                        out.stop();
                    } catch (Exception e) {
                    }
                }
            }
        }).start();
    }

    private InputStream getInput(File file, AudioFormat format) throws Exception {
        if (!MIC) {
            return AudioSystem.getAudioInputStream(file);
        }
        DataLine.Info info = new DataLine.Info(TargetDataLine.class, format);
        TargetDataLine targetDataLine = (TargetDataLine) AudioSystem.getLine(info);
        targetDataLine.open(format, 4096 * 2);
        targetDataLine.start();
        return new InputStream() {
            @Override
            public int read(byte[] buffer) {
                return targetDataLine.read(buffer, 0, buffer.length);
            }

            @Override
            public int read() throws IOException {
                byte[] buffer = new byte[1];
                int bytesRead = read(buffer);
                return bytesRead > 0 ? buffer[0] & 0xFF : -1;
            }
        };
    }

    public static void main(String[] args) throws Exception {
        frame = new JFrame("Effector");
        Dimension d = new Dimension(500, 500);
        frame.setSize(d);
        frame.setPreferredSize(d);
        frame.setExtendedState(JFrame.MAXIMIZED_BOTH);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        AudioEffectsAndFFTExample p = new AudioEffectsAndFFTExample();
        frame.add(p);
        frame.setUndecorated(true);
        frame.setBackground(new Color(0, 0, 0, 0));
        frame.pack();
        frame.setVisible(true);
    }

    public void render() {
        Graphics2D g = buffer.createGraphics();
        g.setBackground(new Color(0, 0, 0, 0));
        g.clearRect(0, 0, getWidth(), getHeight());

        int len = DATA.length;

        g.scale((float) getWidth() / freq.length, 1.0f);
        for (int i = 0; i < freq.length; i++) {
            float h1 = h[i];
            g.setColor(Color.getHSBColor(h1, 1.0f, 1.0f));
            int hei = (int) (h1 * getHeight());
            g.fillRect(i, getHeight() - hei, 1, hei);
        }

        g.setTransform(new AffineTransform());

        g.setColor(Color.BLUE);
        float wi = getWidth() / 2.0f, he = 50.0f;
        float yPos = 0.0f, xPos = getWidth() / 2.0f - wi / 2.0f;
        float stepSize = wi / len;
        g.translate(xPos, yPos);
        g.scale(stepSize, he);
        g.translate(0.0f, 1.0f);
        g.scale(1.0f, 1.0f / 32767.0f);
        int[] xpoints = new int[len];
        for (int i = 0; i < len; i++) {
            xpoints[i] = i;
        }
        g.drawPolyline(xpoints, DATA, len);
        g.dispose();
        BufferedImage tmp = render;
        render = buffer;
        buffer = tmp;
    }

    @Override
    public void paintComponent(Graphics g) {
        super.paintComponent(g);
        Graphics2D g2d = (Graphics2D) g;
        g2d.setBackground(new Color(0, 0, 0, 0));
        g2d.clearRect(0, 0, getWidth(), getHeight());
        g2d.drawImage(render, 0, 0, null);
    }

}
