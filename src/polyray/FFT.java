package polyray;

public class FFT {

    public static void fft(Complex[] x) {
        int n = x.length;
        if (n == 1) {
            return;
        }
        if ((n & (n - 1)) != 0) {
            throw new IllegalArgumentException("Length of x must be a power of 2");
        }

        int size = n >> 1;
        Complex[] even = new Complex[size];
        Complex[] odd = new Complex[size];
        for (int i = 0; i < size; i++) {
            even[i] = x[i * 2];
            odd[i] = x[i * 2 + 1];
        }
        
        fft(even);
        fft(odd);
        
        for (int k = 0; k < size; k++) {
            double kth = -2 * k * Math.PI / n;
            Complex wk = new Complex(Math.cos(kth), Math.sin(kth));
            x[k] = even[k].add(wk.mul(odd[k]));
            x[k + size] = even[k].sub(wk.mul(odd[k]));
        }
    }
    
    public static void ifft(Complex[] x) {
        int n = x.length;
        for (int i = 0; i < n; i++) {
            x[i] = x[i].conjugate();
        }
        fft(x);
        double k = 1.0d / n;
        for (int i = 0; i < n; i++) {
            x[i] = x[i].conjugate().mul(k);
        }
    }
    
    public static Complex[][] fft2D(Complex[][] data) {
        int width = data.length;
        int height = data[0].length;

        for (int i = 0; i < width; i++) {
            fft(data[i]);
        }
        data = transpose(data);
        
        for (int i = 0; i < height; i++) {
            fft(data[i]);
        }
        
        return transpose(data);
    }
    
    public static Complex[][] ifft2D(Complex[][] data) {
        int width = data.length;
        int height = data[0].length;

        for (int i = 0; i < width; i++) {
            ifft(data[i]);
        }
        data = transpose(data);

        for (int i = 0; i < height; i++) {
            ifft(data[i]);
        }

        return transpose(data);
    }

    private static Complex[][] transpose(Complex[][] matrix) {
        int width = matrix.length;
        int height = matrix[0].length;
        Complex[][] transposed = new Complex[height][width];
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                transposed[j][i] = matrix[i][j];
            }
        }
        return transposed;
    }
}
