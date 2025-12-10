struct complex {
    double x;
    double y;

    complex operator/(complex);
    complex operator*(complex);
    complex expj(double);
    complex csqrt(complex);
    complex cexp(complex);
};
