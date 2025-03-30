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
            int idx = i << 1;
            even[i] = x[idx];
            odd[i] = x[idx + 1];
        }

        fft(even);
        fft(odd);
        
        double th = -2.0d * Math.PI;
        for (int k = 0; k < size; k++) {
            double kth = k * th / n;
            Complex e = even[k];
            Complex wk = new Complex(Math.cos(kth), Math.sin(kth)).mul(odd[k]);
            x[k] = e.add(wk);
            x[k + size] = e.sub(wk);
        }
    }
    
    public static void ifft(Complex[] x) {
        int n = x.length;
        for (int i = 0; i < n; i++) {
            Complex c = x[i];
            c.imag = -c.imag;
        }
        fft(x);
        double k = 1.0d / n;
        for (int i = 0; i < n; i++) {
            Complex c = x[i];
            c.imag = -c.imag;
            c.real *= k;
            c.imag *= k;
        }
    }
    
    public static Complex[][] fft2D(Complex[][] data) {
        int width = data.length;
        int height = data[0].length;

        for (int i = 0; i < width; i++) {
            fft(data[i]);
        }
        transpose(data);
        
        for (int i = 0; i < height; i++) {
            fft(data[i]);
        }
        
        transpose(data);
        return data;
    }
    
    public static Complex[][] ifft2D(Complex[][] data) {
        int width = data.length;
        int height = data[0].length;

        for (int i = 0; i < width; i++) {
            ifft(data[i]);
        }
        transpose(data);

        for (int i = 0; i < height; i++) {
            ifft(data[i]);
        }

        transpose(data);
        return data;
    }

    private static void transpose(Complex[][] matrix) {
        int size = matrix.length;
        for (int i = 0; i < size; i++) {
            Complex[] matrixI = matrix[i];
            for (int j = i + 1; j < size; j++) {
                Complex temp = matrixI[j];
                matrixI[j] = matrix[j][i];
                matrix[j][i] = temp;
            }
        }
    }
}
