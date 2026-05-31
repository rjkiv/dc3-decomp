#pragma once

int CalculateSinCosTable(long, float *);
int FFTRealForward(float *, unsigned long, float *);
int FFTComplex(float *, long, long, float *);
void SquareComplexTransposeVector(float *, long);
