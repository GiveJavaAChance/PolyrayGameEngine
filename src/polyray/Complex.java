package polyray;

public class Complex {
    public double real, imag;

    public Complex(double real, double imag) {
        this.real = real;
        this.imag = imag;
    }

    public Complex add(Complex other) {
        return new Complex(this.real + other.real, this.imag + other.imag);
    }

    public Complex sub(Complex other) {
        return new Complex(this.real - other.real, this.imag - other.imag);
    }

    public Complex mul(Complex other) {
        double realPart = this.real * other.real - this.imag * other.imag;
        double imagPart = this.real * other.imag + this.imag * other.real;
        return new Complex(realPart, imagPart);
    }

    public Complex div(Complex other) {
        double denominator = other.real * other.real + other.imag * other.imag;
        double realPart = (this.real * other.real + this.imag * other.imag) / denominator;
        double imagPart = (this.imag * other.real - this.real * other.imag) / denominator;
        return new Complex(realPart, imagPart);
    }

    public Complex mul(double alpha) {
        return new Complex(alpha * real, alpha * imag);
    }

    public double magnitude() {
        return Math.sqrt(real * real + imag * imag);
    }
    
    public double magnitudeSquared() {
        return real * real + imag * imag;
    }

    public double phase() {
        return Math.atan2(imag, real);
    }

    public Complex conjugate() {
        return new Complex(real, -imag);
    }

    public Complex exp() {
        return new Complex(Math.exp(real) * Math.cos(imag), Math.exp(real) * Math.sin(imag));
    }

    public static Complex polar(double magnitude, double angle) {
        return new Complex(magnitude * Math.cos(angle), magnitude * Math.sin(angle));
    }

    @Override
    public String toString() {
        if (imag == 0) return real + "";
        if (real == 0) return imag + "i";
        if (imag < 0) return real + " - " + (-imag) + "i";
        return real + " + " + imag + "i";
    }
}