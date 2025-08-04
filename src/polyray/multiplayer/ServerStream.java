package polyray.multiplayer;

import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;

public class ServerStream {

    private final InputStream in;

    public ServerStream(InputStream in) {
        this.in = in;
    }

    public ByteBuffer read(int len) throws IOException {
        if (len < 0) {
            throw new IllegalArgumentException("Invalid length: " + len);
        }
        if(len == 0) {
            return ByteBuffer.wrap(new byte[0]);
        }
        byte[] b = new byte[len];
        int n = 0;
        while (n < len) {
            int count = in.read(b, n, len - n);
            if (count < 0) {
                throw new EOFException();
            }
            n += count;
        }
        return ByteBuffer.wrap(b);
    }
}
