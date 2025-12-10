#include "complex.h"
#include <cmath>

complex expj(double d1) {
    struct complex result;
    double dVar1 = sin(d1);
    double dVar2 = cos(d1);

    result.x = dVar2;
    result.y = dVar1;

    return result;
}

complex csqrt(complex cplx) {
    complex result;
    double hypotenuse = hypot(cplx.y, cplx.x); // actually calls _hypot but i cant find
                                               // it
    double dVar4 = (hypotenuse - cplx.x) * 0.5;

    if (dVar4 >= 0.0)
        result.y = sqrt(dVar4);

    hypotenuse = (cplx.y + hypotenuse) * 0.5;
    if (hypotenuse >= 0.0)
        result.x = sqrt(hypotenuse);

    if (result.x < 0.0)
        result.y = -result.x;

    return result;
}

complex cexp(complex cplx) {
    complex result;
    complex c = expj(cplx.y);
    double ex = exp(cplx.x);
    result.x = c.x * ex;
    result.y = c.y * ex;
    return result;
}

complex operator/(complex cplx1, complex cplx2) {
    complex result;
    double dVar1 = 1.0 / (cplx2.x * cplx2.x + cplx2.y * cplx2.y);
    result.x = (cplx1.y * cplx2.y + cplx1.x * cplx2.x) * dVar1;
    result.y = (cplx1.y * cplx2.x - cplx1.x * cplx2.y) * dVar1;
    return result;
}

complex operator*(complex cplx1, complex cplx2) {
    complex result;
    result.x = cplx1.x * cplx2.x - cplx2.y * cplx1.y;
    result.y = cplx2.x * cplx1.y + cplx2.y * cplx1.x;
    return result;
}
