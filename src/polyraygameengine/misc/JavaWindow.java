package polyraygameengine.misc;

import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsEnvironment;
import java.awt.KeyboardFocusManager;
import java.awt.Toolkit;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.image.BufferedImage;
import java.util.Collections;
import javax.swing.JFrame;
import javax.swing.JPanel;

public class JavaWindow extends JPanel {

    private BufferedImage frontBuffer, backBuffer;
    private final JFrame frame;
    private Dimension s;
    private int w, h;
    private final GraphicsConfiguration gfxConfig = GraphicsEnvironment.getLocalGraphicsEnvironment().getDefaultScreenDevice().getDefaultConfiguration();

    /**
     * Creates a window
     *
     * @param name The name of the frame
     */
    public JavaWindow(String name) {
        frame = new JFrame(name);
    }

    /**
     * Adds a Component to the JFrame
     *
     * @param comp The component to add
     */
    public void addComponent(Component comp) {
        frame.add(comp);
    }

    public void setIcon(BufferedImage icon) {
        frame.setIconImage(icon);
    }

    public void setPosition(int x, int y) {
        frame.setLocation(x, y);
    }

    /**
     * Creates and initialises a frame.
     *
     * @param width The width of the frame.
     * @param height The height of the frame.
     * @param title Set the title bar visible.
     * @param exitOnClose Set if the program should exit when the frame is
     * closed.
     * @param fullscreen Set fullscreen mode.
     * @param opacity Set opacity of the frame. Note: only works if the frame
     * dossen't have a title bar.
     */
    public void createFrame(int width, int height, boolean title, boolean exitOnClose, boolean fullscreen, float opacity) {
        w = width;
        h = height;
        s = new Dimension(w, h);
        if (fullscreen) {
            s = Toolkit.getDefaultToolkit().getScreenSize();
            w = s.width;
            h = s.height;
        }
        frontBuffer = gfxConfig.createCompatibleImage(w, h);
        backBuffer = gfxConfig.createCompatibleImage(w, h);
        Dimension d = new Dimension(width, height);
        frame.setSize(d);
        frame.setPreferredSize(d);
        if (fullscreen) {
            setFocusTraversalKeysEnabled(false);
            setFocusTraversalKeys(KeyboardFocusManager.FORWARD_TRAVERSAL_KEYS, Collections.emptySet());
            frame.setExtendedState(JFrame.MAXIMIZED_BOTH);
        }
        frame.setDefaultCloseOperation(exitOnClose ? JFrame.EXIT_ON_CLOSE : JFrame.DISPOSE_ON_CLOSE);
        frame.setUndecorated(!title);
        if (!title) {
            frame.setOpacity(Math.max(Math.min(opacity, 1.0f), 0.0f));
        }
        setFocusable(true);
        addMouseListener(new MouseAdapter() {
            @Override
            public void mousePressed(MouseEvent e) {
                mouseDown(e);
            }

            @Override
            public void mouseReleased(MouseEvent e) {
                mouseRelease(e);
            }
        });
        addMouseMotionListener(new MouseAdapter() {
            @Override
            public void mouseMoved(MouseEvent e) {
                move(e);
            }

            @Override
            public void mouseDragged(MouseEvent e) {
                drag(e);
            }
        });
        addMouseWheelListener(this::scroll);
        addKeyListener(new KeyListener() {
            @Override
            public void keyTyped(KeyEvent e) {
                keyType(e);
            }

            @Override
            public void keyPressed(KeyEvent e) {
                keyPress(e);
            }

            @Override
            public void keyReleased(KeyEvent e) {
                keyRelease(e);
            }
        });
        frame.add(this);
        frame.setVisible(true);
    }

    /**
     * Render the content from draw().
     */
    public void render() {
        Graphics2D g = backBuffer.createGraphics();
        g.setColor(Color.BLACK);
        g.fillRect(0, 0, w, h);
        draw(g, w, h);
        
        // Swap Buffers
        BufferedImage tmp = frontBuffer;
        frontBuffer = backBuffer;
        backBuffer = tmp;
        g.dispose();
        repaint();
    }

    public void draw(Graphics2D g, int width, int height) {
    }

    public void mouseDown(MouseEvent e) {
    }

    public void mouseRelease(MouseEvent e) {
    }

    public void move(MouseEvent e) {
    }

    public void drag(MouseEvent e) {
    }

    public void scroll(MouseWheelEvent e) {
    }

    public void keyType(KeyEvent e) {
    }

    public void keyPress(KeyEvent e) {
    }

    public void keyRelease(KeyEvent e) {
    }

    public Dimension getImageSize() {
        return s;
    }

    public BufferedImage getRender() {
        return this.frontBuffer;
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        if (frontBuffer != null) {
            g.drawImage(frontBuffer, 0, 0, this);
        }
    }
}
