// clang-format off
/*
    File:       vBigDSP.c

    Contains:   AltiVec-based Implementation of DSP routines (real & complex FFT, convolution)

    Version:    1.0

    Copyright:  (c) 1999 by Apple Computer, Inc., all rights reserved.

    Change History (most recent first):

                10/12/99    JK      Created

*/

/////////////////////////////////////////////////////////////////////////////////
//      File Name: vBigDSP.c                                                   //
//                                                                             //
//      This library provides a set of DSP routines, implemented using the     //
//      AltiVec instruction set.                                               //
//                                                                             //
//                                                                             //
//      Copyright (c) 1999 Apple Computer, Inc.  All rights reserved.          //
//                                                                             //
//      Version 1.0                                                            //
//                                                                             //                                                                             //
/////////////////////////////////////////////////////////////////////////////////

/*
    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                ("Apple") in consideration of your agreement to the following terms, and your
                use, installation, modification or redistribution of this Apple software
                constitutes acceptance of these terms.  If you do not agree with these terms,
                please do not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following terms, and subject
                to these terms, Apple grants you a personal, non-exclusive license, under Apple's
                copyrights in this original Apple software (the "Apple Software"), to use,
                reproduce, modify and redistribute the Apple Software, with or without
                modifications, in source and/or binary forms; provided that if you redistribute
                the Apple Software in its entirety and without modifications, you must retain
                this notice and the following text and disclaimers in all such redistributions of
                the Apple Software.  Neither the name, trademarks, service marks or logos of
                Apple Computer, Inc. may be used to endorse or promote products derived from the
                Apple Software without specific prior written permission from Apple.  Except as
                expressly stated in this notice, no other rights or licenses, express or implied,
                are granted by Apple herein, including but not limited to any patent rights that
                may be infringed by your derivative works or by other works in which the Apple
                Software may be incorporated.

                The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
                WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
                WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
                PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
                COMBINATION WITH YOUR PRODUCTS.

                IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
                CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
                GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
                ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
                OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
                (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
                ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// clang-format on

// someone modified this to some extent to turn this into generic code that runs on the 360, but it is mostly just a straight up port
// i hope this is the last time i will touch altivec code

#include "synth360/synapse_apo/FFT.h"
#include "vectorintrinsics.h"
#include <cmath>
#include <cstdlib>
#include <errno.h>

#define PI 3.1415926535897932384626433832795f

struct TempBuffer {
    ////////////////////////////////////////////////////////////////////////////////
    //  temporary workspace buffer for FFT implementations
    ////////////////////////////////////////////////////////////////////////////////
    float *mBuffer;

    ////////////////////////////////////////////////////////////////////////////////
    //  current buffer length for above handle
    ////////////////////////////////////////////////////////////////////////////////
    unsigned long mSize;
};

static TempBuffer gTempBuffer;

////////////////////////////////////////////////////////////////////////////////
//
//  log2max
//
//  This function computes the ceiling of log2(n).  For example:
//
//  log2max(7) = 3
//  log2max(8) = 3
//  log2max(9) = 4
//
////////////////////////////////////////////////////////////////////////////////
static long log2max(long n) {
    long power = 1;
    long k = 1;
    if (n == 1) {
        return 0;
    }
    while ((k <<= 1) < n) {
        power++;
    }
    return power;
}

////////////////////////////////////////////////////////////////////////////////
//
//  EnsureStaticBufferSize
//
// This function is used to ensure that the global Handle gTempBufferHandle
// is large enough to hold complexCount complex floats.  It compares the
// currently allocated length (if there is one) with the desired length,
// and, if necessary, allocates a new, larger handle.
// It will NOT shrink the handle, because a function higher in the calling
// chain may need the larger size.
////////////////////////////////////////////////////////////////////////////////
static int EnsureStaticBufferSize(unsigned long complexCount) {
    int result = 0;
    if (gTempBuffer.mSize < complexCount) {
        gTempBuffer.mSize = complexCount;
        if (gTempBuffer.mBuffer) {
            free(gTempBuffer.mBuffer);
        }
        gTempBuffer.mBuffer = (float *)malloc(2 * complexCount * sizeof(float));
        if (!gTempBuffer.mBuffer) {
            result = ENOMEM;
            gTempBuffer.mBuffer = nullptr;
            gTempBuffer.mSize = 0;
        }
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////
//  SquareComplexTransposeVector performs a transpose on a square matrix of
//  complex floats with side length of rowLength.  Data is assumed to be
//  arranged lexicographically, i.e., with side length n:
//
//  re[0] im[0] re[1] im[1] ... re[n-1] im[n-1]
//  re[n] im[n]     ...         re[2n-1] im[2n-1]
//  .
//  .
//  re[(n-1)*n] im[(n-1)*n] ... re[n^2-1] im[n^2-1]
//
//  Data becomes:
//
//  re[0] im[0] re[n] im[n] re[2n] im[2n]...
//  re[1] im[1] re[n+1] im[n+1] ...
//  re[2] im[2] ...
//  .
//  .
//  .
//  re[n-1] im[n-1] ...     re[n^2-1] im[n^2-1]
//
//
////////////////////////////////////////////////////////////////////////////////
void SquareComplexTransposeVector(float *data, long rowLength) {
    ////////////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //  X = x0 x1 x2 x3
    //  Y = y0 y1 y2 y3
    //
    // will generate
    //  Z = x0 x1 y0 y1
    ////////////////////////////////////////////////////////////////////////////////
    XMVECTORU32 vMergeHiPairPerm = { 0x00010203, 0x04050607, 0x10111213, 0x14151617 };

    ////////////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //  X = x0 x1 x2 x3
    //  Y = y0 y1 y2 y3
    //
    // will generate
    //  Z = x2 x3 y2 y3
    ////////////////////////////////////////////////////////////////////////////////
    XMVECTORU32 vMergeLoPairPerm = { 0x08090A0B, 0x0C0D0E0F, 0x18191A1B, 0x1C1D1E1F };

    for (long i = 0; i < rowLength / 2; i++) {
        __vector4 *pInLeft1 = ((__vector4 *)data) + i * rowLength;
        __vector4 *pInLeft2 = pInLeft1 + rowLength / 2;
        __vector4 *pInTop1 = ((__vector4 *)data) + i;
        __vector4 *pInTop2 = pInTop1 + rowLength / 2;

        for (long j = 0; j < i; j++) {
            __vector4 vInLeft1 = *pInLeft1;
            __vector4 vInLeft2 = *pInLeft2;

            ////////////////////////////////////////////////////////////////////////////////
            // read in two vectors that contain a 2x2 square of matrix elements
            // from the top side of the diagonal
            ////////////////////////////////////////////////////////////////////////////////
            __vector4 vInTop1 = *pInTop1;
            __vector4 vInTop2 = *pInTop2;

            __vector4 vOutLeft1 = __vperm(vInTop1, vInTop2, vMergeHiPairPerm.v);
            __vector4 vOutLeft2 = __vperm(vInTop1, vInTop2, vMergeLoPairPerm.v);
            __vector4 vOutTop1 = __vperm(vInLeft1, vInLeft2, vMergeHiPairPerm.v);
            __vector4 vOutTop2 = __vperm(vInLeft1, vInLeft2, vMergeLoPairPerm.v);

            ////////////////////////////////////////////////////////////////////////////////
            // store the transposed matrices in their swapped positions
            ////////////////////////////////////////////////////////////////////////////////
            *pInLeft1 = vOutLeft1;
            *pInLeft2 = vOutLeft2;
            *pInTop1 = vOutTop1;
            *pInTop2 = vOutTop2;

            ////////////////////////////////////////////////////////////////////////////////
            // set pointers to point to the beginning of row 2i and 2i+1
            ////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////
            // advance pointers to next elements in the current columns & rows
            ////////////////////////////////////////////////////////////////////////////////
            pInLeft1 += 1;
            pInLeft2 += 1;

            ////////////////////////////////////////////////////////////////////////////////
            // set pointers to point to the first elements of columns 2i and 2i+1
            ////////////////////////////////////////////////////////////////////////////////
            pInTop1 += rowLength;
            pInTop2 += rowLength;
        }

        ////////////////////////////////////////////////////////////////////////////////
        // read in two vectors that contain a 2x2 square of matrix elements
        // from the left side of the diagonal
        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        // when the above loop is finished, our input pointers point to the 2x2 sub-
        // matrix that is on the diagonal of our matrix to be transposed.  We read in
        // this sub-matrix into two vectors, transpose the sub-matrix (using vector
        // transposes) and store the sub-matrix.
        ////////////////////////////////////////////////////////////////////////////////
        __vector4 vInLeft1 = *pInLeft1;
        __vector4 vInLeft2 = *pInLeft2;

        ////////////////////////////////////////////////////////////////////////////////
        //  We now transpose our two 2x2 sub-matrices, and swap their positions.  The
        // transpose is done with vector permute operations.
        ////////////////////////////////////////////////////////////////////////////////
        __vector4 vOutLeft1 = __vperm(vInLeft1, vInLeft2, vMergeHiPairPerm.v);
        __vector4 vOutLeft2 = __vperm(vInLeft1, vInLeft2, vMergeLoPairPerm.v);
        *pInLeft1 = vOutLeft1;
        *pInLeft2 = vOutLeft2;
    }
}

////////////////////////////////////////////////////////////////////////////////
//  fft_matrix_forward_columnwise
//
//  Performs a recursive 2D matrix FFT on the data pointed to by
// pData. It leaves data in columnwise order.
//
//  There are three steps to the 2D matrix FFT.  First, a FFT is performed on
// each column of the matrix.  Second, each element in the matrix is multiplied
// by a twist factor.  In a length N signal, element X(j, k) is multiplied by
// e^(+/- 2 pi i j k / N ). Finally, an FFT is performed on each row of the
// matrix.  The resulting data is the FFT of the original signal, stored in
// columnwise order.
//
//
////////////////////////////////////////////////////////////////////////////////
int fft_matrix_forward_columnwise(float *pData, long length, float *sinCosTable) {
    long i, j;
    long pow;
    float *pRow;
    long rowCount;
    long colCount;
    long rowPower;
    long colPower;
    float *rowBuffer = nullptr;
    int result = 0;

    pow = log2max(length);

    ////////////////////////////////////////////////////////////////////////////////
    // length must be an exact power of 2
    ////////////////////////////////////////////////////////////////////////////////
    if ((1 << pow) != length) {
        return EINVAL;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // data must be 16-byte aligned because of vector load/store requirements
    ////////////////////////////////////////////////////////////////////////////////
    if (((long)pData) & 0x0F) {
        return EINVAL;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // calculate dimensions of matrix.  If matrix can not be square, then the
    // column count will be 2X the row count
    ////////////////////////////////////////////////////////////////////////////////
    rowPower = pow / 2;
    colPower = rowPower;
    if (pow & 1)
        colPower++;

    rowCount = 1 << rowPower;
    colCount = 1 << colPower;

    ////////////////////////////////////////////////////////////////////////////////
    // allocate a buffer that will be used for column ferrying, and can store
    // two columns at a time.
    ////////////////////////////////////////////////////////////////////////////////
    rowBuffer = (float *)malloc(2 * 2 * sizeof(float) * rowCount);
    if (!rowBuffer) {
        result = ENOMEM;
    } else {
        __vector4 *pCurrentColumn;
        __vector4 vIn1, vIn2;
        __vector4 vOut1, vOut2;
        __vector4 *pSplit1, *pSplit2;
        float *pFFT1, *pFFT2;
        __vector4 vZero = { 0, 0, 0, 0 };

        ////////////////////////////////////////////////////////////////////////////////
        // vSinSignMultiplier is a vector used to create a vector in the form
        // sin(x) -sin(x) sin(y) -sin(y)
        ////////////////////////////////////////////////////////////////////////////////
        __vector4 vSinSignMultiplier = { 1, -1, 1, -1 };
        __vector4 vCosTwistA0;
        __vector4 vCosTwistA1;
        __vector4 vCosTemp0;
        __vector4 vCosTemp1;
        __vector4 vSinTwistA0;
        __vector4 vSinTwistA1;
        __vector4 vA;
        __vector4 vB;
        __vector4 vTransition;

        double fTemp;
        double baseAngle1;
        double baseAngle2;
        double fSinBaseAngle1;
        double fSinBaseAngle2;

        ////////////////////////////////////////////////////////////////////////////////
        // initialize a permute vector that, given input vectors:
        //
        //  X = x0 x1 x2 x3
        //  Y = y0 y1 y2 y3
        //
        // will generate
        //  Z = x0 x1 y0 y1
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vMergeHiPairPerm = { 0x00010203, 0x04050607, 0x10111213, 0x14151617 };

        ////////////////////////////////////////////////////////////////////////////////
        // initialize a permute vector that, given input vectors:
        //
        //  X = x0 x1 x2 x3
        //  Y = y0 y1 y2 y3
        //
        // will generate
        //  Z = x2 x3 y2 y3
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vMergeLoPairPerm = { 0x08090A0B, 0x0C0D0E0F, 0x18191A1B, 0x1C1D1E1F };

        ////////////////////////////////////////////////////////////////////////////////
        // initialize a permute vector that, given input vectors:
        //
        //  X = x0 x1 x2 x3
        //  Y = y0 y1 y2 y3
        //
        // will generate
        //  Z = x1 x0 x3 y2
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vSwappedPerm = { 0x04050607, 0x00010203, 0x0C0D0E0F, 0x08090A0B };

        ////////////////////////////////////////////////////////////////////////////////
        // we now loop through all columns, two columns at a time, and perform column
        // ferrying FFTs on the columns.  We first copy the columns to a separate
        // buffer, then perform the FFTs, and finally copy them back to their original
        // column positions
        ////////////////////////////////////////////////////////////////////////////////
        for (i = 0; i < colCount / 2; i++) {
            ////////////////////////////////////////////////////////////////////////////////
            // set up vectors for incremental updating of cos & sin vectors
            //
            // Given c = cos(w) and s = sin(w), if we want to find the cos and sin of w plus
            // some small angle d, then we can do so by defining:
            //
            // a = 2 * ((sin(d/2)) ^ 2)
            // b = sin(d)
            //
            // Then, we can calculate the cos and sin of the updated angle w+d as:
            //
            // cos(w+d) = c - ac - bs
            // sin(w+d) = s - as + bc
            //
            // the vectors vA and vB contain these increment factors a and b.  Note that
            // each outer loop iteration requires different increment angles.  Looking at
            // the matrix, the angle multipliers that we use for our cos & sin twist
            // multipliers look something like:
            //
            //      0/N     0/N     0/N     0/N     0/N     0/N     0/N ... 0/N
            //      0/N     1/N     2/N     3/N     4/N     5/N     6/N
            //      0/N     2/N     4/N     6/N     8/N     10/N    12/N    .
            //      0/N     3/N     6/N     9/N     12/N    15/N    18/N    .
            //      0/N     4/N     8/N     12/N    16/N    20/N    24/N
            //      .       .
            //      .       .
            //      0/N                                         ...         (N-1)(N-1)
            //
            // Since we're doing two columns at a time, the vA and vB vectors
            // need to have two different sets of increment values.  For example, for
            // the third and fourth columns (handled in the second time through the
            // outer loop) each row increments by 2/N and 3/N.  Since we actually
            // have two sets of twist multiplier vectors, we skip a row for each, so
            // the increment angle is doubled.  This helps reduce calculation dependencies
            // and cuts the number of increment iterations in half, which helps reduce
            // errors in the single-precision calculations used.
            //
            ////////////////////////////////////////////////////////////////////////////////
            baseAngle1 = 4 * i * PI / (colCount * rowCount);
            baseAngle2 = (2 * (2 * i + 1) * PI) / (colCount * rowCount);

            fSinBaseAngle1 = sin(baseAngle1);
            fTemp = 2 * fSinBaseAngle1 * fSinBaseAngle1;

            vTransition.v[0] = fTemp;
            vTransition.v[2] = sin(2 * baseAngle1);

            fSinBaseAngle2 = sin(baseAngle2);
            fTemp = 2 * fSinBaseAngle2 * fSinBaseAngle2;

            vTransition.v[1] = fTemp;
            vTransition.v[3] = sin(2 * baseAngle2);

            vA = __vmrghw(vTransition, vTransition);
            vB = __vmrglw(vTransition, vTransition);

            vB = __vmaddfp(vB, vSinSignMultiplier, vZero);

            ////////////////////////////////////////////////////////////////////////////////
            //  Set up the initial twist multiplier vectors for the beginning of this
            // column.  Values are calculated in the scalar domain, and then
            // transferred to the vector domain via the vTransition vector.
            ////////////////////////////////////////////////////////////////////////////////
            vTransition.v[0] = 1;
            vTransition.v[2] = cos(baseAngle1);
            vTransition.v[3] = cos(baseAngle2);

            vCosTwistA0 = __vspltw(vTransition, 0);
            vCosTwistA1 = __vmrglw(vTransition, vTransition);

            vSinTwistA0 = vZero;

            vTransition.v[2] = fSinBaseAngle1;
            vTransition.v[3] = fSinBaseAngle2;

            vSinTwistA1 = __vmrglw(vTransition, vTransition);
            vSinTwistA1 = __vmaddfp(vSinTwistA1, vSinSignMultiplier, vZero);

            ////////////////////////////////////////////////////////////////////////////////
            // initialize pointers in temporary buffer for storing copied columns.
            ////////////////////////////////////////////////////////////////////////////////
            pSplit1 = (__vector4 *)rowBuffer;
            pSplit2 = pSplit1 + rowCount / 2;

            ////////////////////////////////////////////////////////////////////////////////
            // point at top of columns to be copied
            ////////////////////////////////////////////////////////////////////////////////
            pCurrentColumn = ((__vector4 *)pData) + i;

            ////////////////////////////////////////////////////////////////////////////////
            // loop through all rows in the current column, copying elements to the
            // temporary buffer.  We load two rows at a time, and permute the vectors
            // to create a single vector that contains the appropriate column elements
            // from both rows.
            ////////////////////////////////////////////////////////////////////////////////
            for (j = 0; j < rowCount / 2; j++) {
                ////////////////////////////////////////////////////////////////////////////////
                // read in two rows worth of vectors (two rows X two columns)
                ////////////////////////////////////////////////////////////////////////////////
                vIn1 = *pCurrentColumn;
                pCurrentColumn += colCount / 2;
                vIn2 = *pCurrentColumn;
                pCurrentColumn += colCount / 2;

                ////////////////////////////////////////////////////////////////////////////////
                // permute the vectors so that the two left column entries are in one vector
                // and the two right column entries are in one vector
                ////////////////////////////////////////////////////////////////////////////////
                vOut1 = __vperm(vIn1, vIn2, vMergeHiPairPerm.v);
                vOut2 = __vperm(vIn1, vIn2, vMergeLoPairPerm.v);

                ////////////////////////////////////////////////////////////////////////////////
                // store the split columns to the appropriate buffers
                ////////////////////////////////////////////////////////////////////////////////
                *pSplit1++ = vOut1;
                *pSplit2++ = vOut2;
            }

            ////////////////////////////////////////////////////////////////////////////////
            // perform FFTs on the two columns of data that we have copied to the temp
            // buffer
            ////////////////////////////////////////////////////////////////////////////////
            pFFT1 = rowBuffer;
            pFFT2 = pFFT1 + 2 * rowCount;

            result = FFTComplex(pFFT1, rowCount, -1, sinCosTable);
            if (result != 0)
                break;

            result = FFTComplex(pFFT2, rowCount, -1, sinCosTable);
            if (result != 0)
                break;

            ////////////////////////////////////////////////////////////////////////////////
            // point at the beginning of the two copied column buffers, and at the top
            // of the columns in the matrix where we will store them back.
            ////////////////////////////////////////////////////////////////////////////////
            pSplit1 = (__vector4 *)rowBuffer;
            pSplit2 = pSplit1 + rowCount / 2;
            pCurrentColumn = ((__vector4 *)pData) + i;

            ////////////////////////////////////////////////////////////////////////////////
            // loop through all of the column entries that are stored in the temp buffer,
            // and merge them to be stored back into the columns of the matrix.  At the
            // same time, we perform the twist multiply on the entries.
            ////////////////////////////////////////////////////////////////////////////////
            for (j = 0; j < rowCount / 2; j++) {
                __vector4 vNew1, vNew2;

                ////////////////////////////////////////////////////////////////////////////////
                // get two vectors of column data
                ////////////////////////////////////////////////////////////////////////////////
                vIn1 = *pSplit1++;
                vIn2 = *pSplit2++;

                ////////////////////////////////////////////////////////////////////////////////
                // turn them into row vectors
                ////////////////////////////////////////////////////////////////////////////////
                vNew1 = __vperm(vIn1, vIn2, vMergeHiPairPerm.v);
                vNew2 = __vperm(vIn1, vIn2, vMergeLoPairPerm.v);

                ////////////////////////////////////////////////////////////////////////////////
                // do twist multiplication on both row vectors
                ////////////////////////////////////////////////////////////////////////////////
                vOut1 = __vmaddfp(vCosTwistA0, vNew1, vZero);
                vOut1 =
                    __vmaddfp(vSinTwistA0, __vperm(vNew1, vNew1, vSwappedPerm.v), vOut1);

                vOut2 = __vmaddfp(vCosTwistA1, vNew2, vZero);
                vOut2 =
                    __vmaddfp(vSinTwistA1, __vperm(vNew2, vNew2, vSwappedPerm.v), vOut2);

                ////////////////////////////////////////////////////////////////////////////////
                // store row vectors back into the matrix
                ////////////////////////////////////////////////////////////////////////////////
                *pCurrentColumn = vOut1;
                pCurrentColumn += colCount / 2;
                *pCurrentColumn = vOut2;
                pCurrentColumn += colCount / 2;

                ////////////////////////////////////////////////////////////////////////////////
                // update sin & cos vectors for twist multiply
                ////////////////////////////////////////////////////////////////////////////////
                vCosTemp0 = __vnmsubfp(vCosTwistA0, vA, vCosTwistA0);
                vCosTemp0 = __vnmsubfp(vSinTwistA0, vB, vCosTemp0);

                vSinTwistA0 = __vnmsubfp(vSinTwistA0, vA, vSinTwistA0);
                vSinTwistA0 = __vmaddfp(vCosTwistA0, vB, vSinTwistA0);

                vCosTwistA0 = vCosTemp0;

                vCosTemp1 = __vnmsubfp(vCosTwistA1, vA, vCosTwistA1);
                vCosTemp1 = __vnmsubfp(vSinTwistA1, vB, vCosTemp1);

                vSinTwistA1 = __vnmsubfp(vSinTwistA1, vA, vSinTwistA1);
                vSinTwistA1 = __vmaddfp(vCosTwistA1, vB, vSinTwistA1);

                vCosTwistA1 = vCosTemp1;
            }
        }

        if (result == 0) {
            ////////////////////////////////////////////////////////////////////////////////
            // finally, loop through all rows of matrix, performing an FFT on each row
            ////////////////////////////////////////////////////////////////////////////////
            pRow = pData;
            for (i = rowCount - 1; i >= 0; i--) {
                result = FFTComplex(pRow + i * (colCount * 2), colCount, -1, sinCosTable);
                if (result != 0)
                    break;
            }
        }
    }

    if (rowBuffer) {
        free(rowBuffer);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
//  fft_matrix_inverse_columnwise
//
//  Performs a recursive 2D matrix inverse FFT on the data pointed
// to by pData. It assumes that the data is in columnwise order, and returns the
// resulting data in "normal" linear (lexicographic) order.
//
//  There are three steps to the 2D matrix inverse FFT.  First, a FFT is
// performed on each row of the matrix.  Second, each element in the matrix is
// multiplied by a twist factor. In a length N signal, element X(j, k) is
// multiplied by e^(+/- 2 pi i j k / N ). Finally, an FFT is performed on each
// column of the matrix.  The resulting data is the recovered signal from the
// inverse FFT, in linear order.
//
////////////////////////////////////////////////////////////////////////////////
int fft_matrix_inverse_columnwise(float *pData, long length, float *sinCosTable) {
    long i, j;
    long pow;
    float *pRow;
    long rowCount;
    long colCount;
    long rowPower;
    long colPower;
    float *rowBuffer = nullptr;
    int result = 0;

    pow = log2max(length);

    ////////////////////////////////////////////////////////////////////////////////
    // length must be an exact power of 2
    ////////////////////////////////////////////////////////////////////////////////
    if ((1 << pow) != length) {
        return EINVAL;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // data must be 16-byte aligned because of vector load/store requirements
    ////////////////////////////////////////////////////////////////////////////////
    if (((long)pData) & 0x0F) {
        return EINVAL;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // calculate dimensions of matrix.  If matrix can not be square, then the
    // column count will be 2X the row count
    ////////////////////////////////////////////////////////////////////////////////
    rowPower = pow / 2;
    colPower = rowPower;
    if (pow & 1)
        colPower++;

    rowCount = 1 << rowPower;
    colCount = 1 << colPower;

    ////////////////////////////////////////////////////////////////////////////////
    // allocate a buffer that will be used for column ferrying, and can store
    // two columns at a time.
    ////////////////////////////////////////////////////////////////////////////////
    rowBuffer = (float *)malloc(2 * 2 * sizeof(float) * rowCount);
    if (!rowBuffer) {
        result = ENOMEM;
    } else {
        __vector4 *pCurrentColumn;
        __vector4 vIn1, vIn2;
        __vector4 vOut1, vOut2;
        __vector4 *pSplit1, *pSplit2;
        float *pFFT1, *pFFT2;
        __vector4 vZero = { 0, 0, 0, 0 };

        ////////////////////////////////////////////////////////////////////////////////
        // vSinSignMultiplier is a vector used to create a vector in the form
        // -sin(x) sin(x) -sin(y) sin(y)
        ////////////////////////////////////////////////////////////////////////////////
        __vector4 vSinSignMultiplier = { -1, 1, -1, 1 };
        __vector4 vCosTwistA0;
        __vector4 vCosTwistA1;
        __vector4 vCosTemp0;
        __vector4 vCosTemp1;
        __vector4 vSinTwistA0;
        __vector4 vSinTwistA1;
        __vector4 vA;
        __vector4 vB;
        __vector4 vTransition;

        double fTemp;
        double baseAngle1;
        double baseAngle2;
        double fSinBaseAngle1;
        double fSinBaseAngle2;

        ////////////////////////////////////////////////////////////////////////////////
        // initialize a permute vector that, given input vectors:
        //
        //  X = x0 x1 x2 x3
        //  Y = y0 y1 y2 y3
        //
        // will generate
        //  Z = x0 x1 y0 y1
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vMergeHiPairPerm = { 0x00010203, 0x04050607, 0x10111213, 0x14151617 };

        ////////////////////////////////////////////////////////////////////////////////
        // initialize a permute vector that, given input vectors:
        //
        //  X = x0 x1 x2 x3
        //  Y = y0 y1 y2 y3
        //
        // will generate
        //  Z = x2 x3 y2 y3
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vMergeLoPairPerm = { 0x08090A0B, 0x0C0D0E0F, 0x18191A1B, 0x1C1D1E1F };

        ////////////////////////////////////////////////////////////////////////////////
        // initialize a permute vector that, given input vectors:
        //
        //  X = x0 x1 x2 x3
        //  Y = y0 y1 y2 y3
        //
        // will generate
        //  Z = x1 x0 x3 y2
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vSwappedPerm = { 0x04050607, 0x00010203, 0x0C0D0E0F, 0x08090A0B };

        ////////////////////////////////////////////////////////////////////////////////
        // perform a FFT on every row of the matrix
        ////////////////////////////////////////////////////////////////////////////////
        pRow = pData;
        for (i = rowCount - 1; i >= 0; i--) {
            result = FFTComplex(pRow + i * (colCount * 2), colCount, 1, sinCosTable);
            if (result != 0)
                break;
        }

        if (result == 0) {
            ////////////////////////////////////////////////////////////////////////////////
            // we now loop through all columns, two columns at a time, and perform column
            // ferrying FFTs on the columns.  We must also perform a twist multiply on
            // all entries, so we do this after we load in the columns, and before we
            // ferry them over to the temporary buffer
            ////////////////////////////////////////////////////////////////////////////////
            for (i = 0; i < colCount / 2; i++) {
                ////////////////////////////////////////////////////////////////////////////////
                // set up vectors for incremental updating of cos & sin vectors
                //
                // Given c = cos(w) and s = sin(w), if we want to find the cos and sin of w plus
                // some small angle d, then we can do so by defining:
                //
                // a = 2 * ((sin(d/2)) ^ 2)
                // b = sin(d)
                //
                // Then, we can calculate the cos and sin of the updated angle w+d as:
                //
                // cos(w+d) = c - ac - bs
                // sin(w+d) = s - as + bc
                //
                // the vectors vA and vB contain these increment factors a and b.  Note that
                // each outer loop iteration requires different increment angles.  Looking at
                // the matrix, the angle multipliers that we use for our cos & sin twist
                // multipliers look something like:
                //
                //      0/N     0/N     0/N     0/N     0/N     0/N     0/N ... 0/N
                //      0/N     1/N     2/N     3/N     4/N     5/N     6/N
                //      0/N     2/N     4/N     6/N     8/N     10/N    12/N    .
                //      0/N     3/N     6/N     9/N     12/N    15/N    18/N    .
                //      0/N     4/N     8/N     12/N    16/N    20/N    24/N
                //      .       .
                //      .       .
                //      0/N                                         ...         (N-1)(N-1)
                //
                // Since we're doing two columns at a time, the vA and vB vectors
                // need to have two different sets of increment values.  For example, for
                // the third and fourth columns (handled in the second time through the
                // outer loop) each row increments by 2/N and 3/N.  Since we actually
                // have two sets of twist multiplier vectors, we skip a row for each, so
                // the increment angle is doubled.  This helps reduce calculation dependencies
                // and cuts the number of increment iterations in half, which helps reduce
                // errors in the single-precision calculations used.
                //
                ////////////////////////////////////////////////////////////////////////////////
                baseAngle1 = 4 * i * PI / (colCount * rowCount);
                baseAngle2 = (2 * (2 * i + 1) * PI) / (colCount * rowCount);

                fSinBaseAngle1 = sin(baseAngle1);
                fTemp = 2 * fSinBaseAngle1 * fSinBaseAngle1;

                vTransition.v[0] = fTemp;
                vTransition.v[2] = sin(2 * baseAngle1);

                fSinBaseAngle2 = sin(baseAngle2);
                fTemp = 2 * fSinBaseAngle2 * fSinBaseAngle2;

                vTransition.v[1] = fTemp;
                vTransition.v[3] = sin(2 * baseAngle2);

                vA = __vmrghw(vTransition, vTransition);
                vB = __vmrglw(vTransition, vTransition);

                vB = __vmaddfp(vB, vSinSignMultiplier, vZero);

                ////////////////////////////////////////////////////////////////////////////////
                //  Set up the initial twist multiplier vectors for the beginning of this
                // column.  Values are calculated in the scalar domain, and then
                // transferred to the vector domain via the vTransition vector.
                ////////////////////////////////////////////////////////////////////////////////
                vTransition.v[0] = 1;
                vTransition.v[2] = cos(baseAngle1);
                vTransition.v[3] = cos(baseAngle2);

                vCosTwistA0 = __vspltw(vTransition, 0);
                vCosTwistA1 = __vmrglw(vTransition, vTransition);

                vSinTwistA0 = vZero;

                vTransition.v[2] = fSinBaseAngle1;
                vTransition.v[3] = fSinBaseAngle2;

                vSinTwistA1 = __vmrglw(vTransition, vTransition);
                vSinTwistA1 = __vmaddfp(vSinTwistA1, vSinSignMultiplier, vZero);

                ////////////////////////////////////////////////////////////////////////////////
                // initialize pointers in temporary buffer for storing copied columns.
                ////////////////////////////////////////////////////////////////////////////////
                pSplit1 = (__vector4 *)rowBuffer;
                pSplit2 = pSplit1 + rowCount / 2;

                ////////////////////////////////////////////////////////////////////////////////
                // point at top of columns to be copied
                ////////////////////////////////////////////////////////////////////////////////
                pCurrentColumn = ((__vector4 *)pData) + i;

                ////////////////////////////////////////////////////////////////////////////////
                // loop through all rows in the current column, copying elements to the
                // temporary buffer.  We load two rows at a time, and permute the vectors
                // to create a single vector that contains the appropriate column elements
                // from both rows.
                ////////////////////////////////////////////////////////////////////////////////
                for (j = 0; j < rowCount / 2; j++) {
                    __vector4 vTwistedIn1, vTwistedIn2;

                    ////////////////////////////////////////////////////////////////////////////////
                    // read in two rows worth of vectors (two rows X two columns)
                    ////////////////////////////////////////////////////////////////////////////////
                    vIn1 = *pCurrentColumn;
                    pCurrentColumn += colCount / 2;
                    vIn2 = *pCurrentColumn;
                    pCurrentColumn += colCount / 2;

                    ////////////////////////////////////////////////////////////////////////////////
                    // twist multiply input vectors
                    ////////////////////////////////////////////////////////////////////////////////
                    vTwistedIn1 = __vmaddfp(vCosTwistA0, vIn1, vZero);
                    vTwistedIn1 = __vmaddfp(
                        vSinTwistA0, __vperm(vIn1, vIn1, vSwappedPerm.v), vTwistedIn1
                    );

                    vTwistedIn2 = __vmaddfp(vCosTwistA1, vIn2, vZero);
                    vTwistedIn2 = __vmaddfp(
                        vSinTwistA1, __vperm(vIn2, vIn2, vSwappedPerm.v), vTwistedIn2
                    );

                    ////////////////////////////////////////////////////////////////////////////////
                    // permute the vectors so that the two left column entries are in one vector
                    // and the two right column entries are in one vector
                    ////////////////////////////////////////////////////////////////////////////////
                    vOut1 = __vperm(vTwistedIn1, vTwistedIn2, vMergeHiPairPerm.v);
                    vOut2 = __vperm(vTwistedIn1, vTwistedIn2, vMergeLoPairPerm.v);

                    ////////////////////////////////////////////////////////////////////////////////
                    // store the split columns to the appropriate buffers
                    ////////////////////////////////////////////////////////////////////////////////
                    *pSplit1++ = vOut1;
                    *pSplit2++ = vOut2;

                    ////////////////////////////////////////////////////////////////////////////////
                    // update sin & cos vectors for twist multiply
                    ////////////////////////////////////////////////////////////////////////////////
                    vCosTemp0 = __vnmsubfp(vCosTwistA0, vA, vCosTwistA0);
                    vCosTemp0 = __vnmsubfp(vSinTwistA0, vB, vCosTemp0);

                    vSinTwistA0 = __vnmsubfp(vSinTwistA0, vA, vSinTwistA0);
                    vSinTwistA0 = __vmaddfp(vCosTwistA0, vB, vSinTwistA0);

                    vCosTwistA0 = vCosTemp0;

                    vCosTemp1 = __vnmsubfp(vCosTwistA1, vA, vCosTwistA1);
                    vCosTemp1 = __vnmsubfp(vSinTwistA1, vB, vCosTemp1);

                    vSinTwistA1 = __vnmsubfp(vSinTwistA1, vA, vSinTwistA1);
                    vSinTwistA1 = __vmaddfp(vCosTwistA1, vB, vSinTwistA1);

                    vCosTwistA1 = vCosTemp1;
                }

                ////////////////////////////////////////////////////////////////////////////////
                // perform FFTs on the two columns of data that we have copied to the temp
                // buffer
                ////////////////////////////////////////////////////////////////////////////////
                pFFT1 = rowBuffer;
                pFFT2 = pFFT1 + 2 * rowCount;

                result = FFTComplex(pFFT1, rowCount, 1, sinCosTable);
                if (result != 0)
                    break;

                result = FFTComplex(pFFT2, rowCount, 1, sinCosTable);
                if (result != 0)
                    break;

                ////////////////////////////////////////////////////////////////////////////////
                // point at the beginning of the two copied column buffers, and at the top
                // of the columns in the matrix where we will store them back.
                ////////////////////////////////////////////////////////////////////////////////
                pSplit1 = (__vector4 *)rowBuffer;
                pSplit2 = pSplit1 + rowCount / 2;
                pCurrentColumn = ((__vector4 *)pData) + i;

                ////////////////////////////////////////////////////////////////////////////////
                // loop through all of the column entries that are stored in the temp buffer,
                // and merge them to be stored back into the columns of the matrix.
                ////////////////////////////////////////////////////////////////////////////////
                for (j = 0; j < rowCount / 2; j++) {
                    ////////////////////////////////////////////////////////////////////////////////
                    // get two vectors of column data
                    ////////////////////////////////////////////////////////////////////////////////
                    vIn1 = *pSplit1++;
                    vIn2 = *pSplit2++;

                    ////////////////////////////////////////////////////////////////////////////////
                    // turn them into row vectors
                    ////////////////////////////////////////////////////////////////////////////////
                    vOut1 = __vperm(vIn1, vIn2, vMergeHiPairPerm.v);
                    vOut2 = __vperm(vIn1, vIn2, vMergeLoPairPerm.v);

                    ////////////////////////////////////////////////////////////////////////////////
                    // store row vectors back into the matrix
                    ////////////////////////////////////////////////////////////////////////////////
                    *pCurrentColumn = vOut1;
                    pCurrentColumn += colCount / 2;
                    *pCurrentColumn = vOut2;
                    pCurrentColumn += colCount / 2;
                }
            }
        }
    }

    if (rowBuffer) {
        free(rowBuffer);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
//  fft_square_matrix
//
//  Performs a forward or inverse FFT on the data pointed to
// by pData.  If isign == -1, then a forward FFT is performed, otherwise an
// inverse FFT is performed.  The FFT is performed by doing a recursive matrix
// FFT.  The length of the data must be an even power of two, to ensure that
// the matrix is a square, which allows for a trivial transpose of the matrix
// to ensure that output data is in lexicographic order (or to transform input
// data into columnwise order, in the case of the inverse FFT).
////////////////////////////////////////////////////////////////////////////////
int fft_square_matrix(float *pData, long length, long isign, float *sinCosTable) {
    int result;
    long pow = log2max(length);

    ////////////////////////////////////////////////////////////////////////////////
    // length must be an exact power of 2
    ////////////////////////////////////////////////////////////////////////////////
    if ((1 << pow) != length) {
        return EINVAL;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // length must be an even power of 2
    ////////////////////////////////////////////////////////////////////////////////
    if (pow & 1) {
        return EINVAL;
    }
    if (isign == -1) {
        ////////////////////////////////////////////////////////////////////////////////
        // we are performing a forward FFT, so do a normal forward matrix FFT, which
        // will leave the data in columnwise order
        ////////////////////////////////////////////////////////////////////////////////
        result = fft_matrix_forward_columnwise(pData, length, sinCosTable);
        if (result == 0) {
            ////////////////////////////////////////////////////////////////////////////////
            // transpose the square matrix, so that data is no longer in columnwise order
            ////////////////////////////////////////////////////////////////////////////////
            SquareComplexTransposeVector(pData, 1 << (pow / 2));
        }
    } else {
        ////////////////////////////////////////////////////////////////////////////////
        // transpose the square matrix of input data, to transform it into columnwise
        // order.
        ////////////////////////////////////////////////////////////////////////////////
        SquareComplexTransposeVector(pData, 1 << (pow / 2));

        ////////////////////////////////////////////////////////////////////////////////
        // perform an inverse matrix FFT on the data, which expects the data to be
        // in columnwise order, and leaves the result in lexicographic order
        ////////////////////////////////////////////////////////////////////////////////
        result = fft_matrix_inverse_columnwise(pData, length, sinCosTable);
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////
// fft_altivec
//
//  Performs a forward or inverse FFT on the complex signal data
// pointed to by pData. pTempBuffer must point to a buffer that is of equal
// length as the signal pointed to by pData, and which will be overwritten by
// the routine.  If isign == -1, then a forward FFT is performed.  Otherwise
// an inverse FFT is performed.
//
// requirements:
//
//  - length must be an exact power of 2
//  - pData and pTempBuffer must be 16-byte aligned
//  - signal length must be at least 16, because of vector sizes and
//      implementation details.
//
////////////////////////////////////////////////////////////////////////////////
int fft_altivec(
    float *pData, float *pTempBuffer, unsigned long len, long isign, float *sinCosTable
) {
    long j, i;
    float *srcPtr, *dstPtr, *tmp;
    long pow, root, trig;
    int result = 0;

    ////////////////////////////////////////////////////////////////////////////////
    // initialize a zero vector
    ////////////////////////////////////////////////////////////////////////////////
    __vector4 vZero = { 0, 0, 0, 0 };

    ////////////////////////////////////////////////////////////////////////////////
    // initialize a multiplier vector that will
    // negate the second and fourth float
    // elements.
    ////////////////////////////////////////////////////////////////////////////////
    __vector4 vForwardSinSignMultiplier = { 1, -1, 1, -1 };

    ////////////////////////////////////////////////////////////////////////////////
    // initialize a multiplier vector that will
    // negate the first and third float
    // elements.
    ////////////////////////////////////////////////////////////////////////////////
    __vector4 vInverseSinSignMultiplier = { -1, 1, -1, 1 };

    ////////////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //  X = x0 x1 x2 x3
    //  Y = y0 y1 y2 y3
    //
    // will generate
    //  Z = x1 x0 x3 y2
    ////////////////////////////////////////////////////////////////////////////////
    XMVECTORU32 vSwappedPerm = { 0x04050607, 0x00010203, 0x0C0D0E0F, 0x08090A0B };

    ////////////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //  X = x0 x1 x2 x3
    //  Y = y0 y1 y2 y3
    //
    // will generate
    //  Z = x0 x1 y0 y1
    ////////////////////////////////////////////////////////////////////////////////
    XMVECTORU32 vMergeHiPairPerm = { 0x00010203, 0x04050607, 0x10111213, 0x14151617 };

    ////////////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //  X = x0 x1 x2 x3
    //  Y = y0 y1 y2 y3
    //
    // will generate
    //  Z = x2 x3 y2 y3
    ////////////////////////////////////////////////////////////////////////////////
    XMVECTORU32 vMergeLoPairPerm = { 0x08090A0B, 0x0C0D0E0F, 0x18191A1B, 0x1C1D1E1F };

    ////////////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //  X = x0 x1 x2 x3
    //  Y = y0 y1 y2 y3
    //
    // will generate
    //  Z = x0 x0 x2 x2
    ////////////////////////////////////////////////////////////////////////////////
    XMVECTORU32 vCosPermute = { 0x00010203, 0x00010203, 0x08090A0B, 0x08090A0B };

    ////////////////////////////////////////////////////////////////////////////////
    // initialize a select vector that, given input vectors:
    //
    //  X = x0 x1 x2 x3
    //  Y = y0 y1 y2 y3
    //
    // will generate
    //  Z = x0 y1 x2 y3
    ////////////////////////////////////////////////////////////////////////////////
    XMVECTORU32 vForwardSinNegSinSelect = { 0, 0xFFFFFFFF, 0, 0xFFFFFFFF };

    ////////////////////////////////////////////////////////////////////////////////
    // initialize a select vector that, given input vectors:
    //
    //  X = x0 x1 x2 x3
    //  Y = y0 y1 y2 y3
    //
    // will generate
    //  Z = y0 x1 y2 x3
    ////////////////////////////////////////////////////////////////////////////////
    XMVECTORU32 vInverseSinNegSinSelect = { 0xFFFFFFFF, 0, 0xFFFFFFFF, 0 };

    ////////////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //  X = x0 x1 x2 x3
    //  Y = y0 y1 y2 y3
    //
    // will generate
    //  Z = x1 y1 x3 y3
    ////////////////////////////////////////////////////////////////////////////////
    XMVECTORU32 vForwardSinPermute = { 0x04050607, 0x14151617, 0x0C0D0E0F, 0x1C1D1E1F };

    ////////////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //  X = x0 x1 x2 x3
    //  Y = y0 y1 y2 y3
    //
    // will generate
    //  Z = y1 x1 y3 x3
    ////////////////////////////////////////////////////////////////////////////////
    XMVECTORU32 vInverseSinPermute = { 0x14151617, 0x04050607, 0x1C1D1E1F, 0x0C0D0E0F };
    __vector4 vSinNegSinSelect;
    __vector4 vSinPermute;
    __vector4 vCSLoad1;
    __vector4 vCSLoad2;
    __vector4 vNegCSLoad1;
    __vector4 vNegCSLoad2;
    __vector4 vCosA1;
    __vector4 vSinA1;
    __vector4 vCosA2;
    __vector4 vSinA2;
    __vector4 vIn1;
    __vector4 vIn2;
    __vector4 vIn3;
    __vector4 vIn4;
    __vector4 vDiffA1;
    __vector4 vDiffA2;
    __vector4 vSumA1;
    __vector4 vSumA2;
    __vector4 vSwappedDiffA1;
    __vector4 vSwappedDiffA2;
    __vector4 vButterflyA1;
    __vector4 vButterflyA2;
    __vector4 vInLoB1, vInHiB1;
    __vector4 vInLoB2, vInHiB2;
    __vector4 vSinB1, vSinB2;
    __vector4 vCosB1, vCosB2;
    __vector4 vDiffB1, vDiffB2;
    __vector4 vSwappedDiffB1, vSwappedDiffB2;
    __vector4 vResultLoB1, vResultLoB2;
    __vector4 vResultHiB1, vResultHiB2;
    __vector4 vCSLoadB1;
    __vector4 vNegSinB1;
    __vector4 vCSLoadB2;
    __vector4 vNegSinB2;
    __vector4 *pInVec1, *pInVec2, *pInVec3, *pInVec4;
    __vector4 *pOutVec1, *pOutVec2, *pOutVec3, *pOutVec4;
    __vector4 vInLo1, vInLo2;
    __vector4 vInHi1, vInHi2;
    __vector4 vResultLo1, vResultLo2;
    __vector4 vResultHi1, vResultHi2;
    __vector4 vSinSignMultiplier;
    __vector4 vTransitionFloatVector;
    __vector4 vInverseDivideMultiplier;

    ////////////////////////////////////////////////////////////////////////////////
    // calculate log2(len)
    ////////////////////////////////////////////////////////////////////////////////
    pow = log2max(len);

    ////////////////////////////////////////////////////////////////////////////////
    // length must be an exact power of 2
    ////////////////////////////////////////////////////////////////////////////////
    if ((1 << pow) != len) {
        return EINVAL;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // data must be 16-byte aligned because of vector load/store requirements
    ////////////////////////////////////////////////////////////////////////////////
    if (((long)pData) & 0x0F) {
        return EINVAL;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // temp buffer must be 16-byte aligned because of vector load/store requirements
    ////////////////////////////////////////////////////////////////////////////////
    if (((long)pTempBuffer) & 0x0F) {
        return EINVAL;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // initialize permute, select, and
    // multiplier vectors based on whether
    // we are doing a forward or inverse
    // fft.
    ////////////////////////////////////////////////////////////////////////////////
    if (isign < 0) {
        vSinSignMultiplier = vForwardSinSignMultiplier;
        vSinNegSinSelect = vForwardSinNegSinSelect.v;
        vSinPermute = vForwardSinPermute.v;
    } else {
        vSinSignMultiplier = vInverseSinSignMultiplier;
        vSinNegSinSelect = vInverseSinNegSinSelect.v;
        vSinPermute = vInverseSinPermute.v;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // init trig counter, for indexing output vectors
    ////////////////////////////////////////////////////////////////////////////////
    trig = 1;

    ////////////////////////////////////////////////////////////////////////////////
    // start with data as source and temp buffer as destination
    ////////////////////////////////////////////////////////////////////////////////
    srcPtr = pData;
    dstPtr = pTempBuffer;

    ////////////////////////////////////////////////////////////////////////////////
    // initialize root to zero.  Root is used as an index into the sin & cos table
    ////////////////////////////////////////////////////////////////////////////////
    root = 0;

    do {
        ////////////////////////////////////////////////////////////////////////////////
        // load sin & cos
        ////////////////////////////////////////////////////////////////////////////////
        vCSLoad1 = *(__vector4 *)&sinCosTable[root];
        vCSLoad2 = *(__vector4 *)&sinCosTable[(len / 2) + root];

        ////////////////////////////////////////////////////////////////////////////////
        // create negative sin & cos
        ////////////////////////////////////////////////////////////////////////////////
        vNegCSLoad1 = __vsubfp(vZero, vCSLoad1);
        vNegCSLoad2 = __vsubfp(vZero, vCSLoad2);

        ////////////////////////////////////////////////////////////////////////////////
        // create vector
        //
        //  vCosA1 = cos(root*pi/len) cos(root*pi/len) cos((root+2)*pi/len) cos((root+2)*pi/len)
        ////////////////////////////////////////////////////////////////////////////////
        vCosA1 = __vperm(vCSLoad1, vCSLoad1, vCosPermute.v);

        ////////////////////////////////////////////////////////////////////////////////
        // create vector
        //
        //  vSinA1 = sin(root*pi/len) -sin(root*pi/len) sin((root+2)*pi/len) -sin((root+2)*pi/len)
        //
        // or
        //
        //  vSinA1 = -sin(root*pi/len) sin(root*pi/len) -sin((root+2)*pi/len) sin((root+2)*pi/len)
        //
        // depending on whether we are doing a forward or inverse fft
        ////////////////////////////////////////////////////////////////////////////////
        vSinA1 = __vperm(vCSLoad1, vNegCSLoad1, vSinPermute);

        ////////////////////////////////////////////////////////////////////////////////
        // create vector
        //
        //  vCosA2 = cos(pi/2 + root*pi/len) cos(pi/2 + root*pi/len) cos(pi/2 + (root+2)*pi/len) cos(pi/2 + (root+2)*pi/len)
        ////////////////////////////////////////////////////////////////////////////////
        vCosA2 = __vperm(vCSLoad2, vCSLoad2, vCosPermute.v);

        ////////////////////////////////////////////////////////////////////////////////
        // create vector
        //
        //  vSinA1 = sin(pi/2 + root*pi/len) -sin(pi/2 + root*pi/len) sin(pi/2 + (root+2)*pi/len) -sin(pi/2 + (root+2)*pi/len)
        //
        // or
        //
        //  vSinA1 = -sin(pi/2 + root*pi/len) sin(pi/2 + root*pi/len) -sin(pi/2 + (root+2)*pi/len) sin(pi/2 + (root+2)*pi/len)
        //
        // depending on whether we are doing a forward or inverse fft
        ////////////////////////////////////////////////////////////////////////////////
        vSinA2 = __vperm(vCSLoad2, vNegCSLoad2, vSinPermute);

        ////////////////////////////////////////////////////////////////////////////////
        // load four input vectors for calculating butterflies
        ////////////////////////////////////////////////////////////////////////////////
        vIn1 = *(__vector4 *)srcPtr;
        vIn2 = *(__vector4 *)&srcPtr[len];
        vIn3 = *(__vector4 *)&srcPtr[len / 2];
        vIn4 = *(__vector4 *)&srcPtr[len + len / 2];

        ////////////////////////////////////////////////////////////////////////////////
        // calculate four butterflies of input vectors (two from vIn1 and vIn2, two
        // from vIn3 and vIn4).
        ////////////////////////////////////////////////////////////////////////////////
        vDiffA1 = __vsubfp(vIn1, vIn2);
        vDiffA2 = __vsubfp(vIn3, vIn4);
        vSumA1 = __vaddfp(vIn1, vIn2);
        vSumA2 = __vaddfp(vIn3, vIn4);

        vSwappedDiffA1 = __vperm(vDiffA1, vDiffA1, vSwappedPerm.v);
        vSwappedDiffA2 = __vperm(vDiffA2, vDiffA2, vSwappedPerm.v);

        vButterflyA1 = __vmaddfp(vDiffA1, vCosA1, vZero);
        vButterflyA2 = __vmaddfp(vDiffA2, vCosA2, vZero);
        vButterflyA1 = __vmaddfp(vSwappedDiffA1, vSinA1, vButterflyA1);
        vButterflyA2 = __vmaddfp(vSwappedDiffA2, vSinA2, vButterflyA2);

        ////////////////////////////////////////////////////////////////////////////////
        // results of butterflies from first stage are input to butterflies
        // of second stage
        ////////////////////////////////////////////////////////////////////////////////
        vInLoB1 = __vperm(vSumA1, vButterflyA1, vMergeHiPairPerm.v);
        vInLoB2 = __vperm(vSumA1, vButterflyA1, vMergeLoPairPerm.v);
        vInHiB1 = __vperm(vSumA2, vButterflyA2, vMergeHiPairPerm.v);
        vInHiB2 = __vperm(vSumA2, vButterflyA2, vMergeLoPairPerm.v);

        ////////////////////////////////////////////////////////////////////////////////
        // load sin & cos and generate sin, cos vectors for second-stage butterfly
        ////////////////////////////////////////////////////////////////////////////////
        vCSLoadB1 = *(__vector4 *)&sinCosTable[2 * root];
        vSinB1 = __vspltw(vCSLoadB1, 1);
        vCosB1 = __vspltw(vCSLoadB1, 0);
        vNegSinB1 = __vsubfp(vZero, vSinB1);
        vSinB1 = __vsel(vSinB1, vNegSinB1, vSinNegSinSelect);

        vCSLoadB2 = *(__vector4 *)&sinCosTable[2 * root + 4];
        vSinB2 = __vspltw(vCSLoadB2, 1);
        vCosB2 = __vspltw(vCSLoadB2, 0);
        vNegSinB2 = __vsubfp(vZero, vSinB2);
        vSinB2 = __vsel(vSinB2, vNegSinB2, vSinNegSinSelect);

        vDiffB1 = __vsubfp(vInLoB1, vInHiB1);
        vDiffB2 = __vsubfp(vInLoB2, vInHiB2);

        vSwappedDiffB1 = __vperm(vDiffB1, vDiffB1, vSwappedPerm.v);
        vSwappedDiffB2 = __vperm(vDiffB2, vDiffB2, vSwappedPerm.v);

        vResultLoB1 = __vaddfp(vInLoB1, vInHiB1);
        vResultLoB2 = __vaddfp(vInLoB2, vInHiB2);

        vResultHiB1 = __vmaddfp(vCosB1, vDiffB1, vZero);
        vResultHiB1 = __vmaddfp(vSinB1, vSwappedDiffB1, vResultHiB1);
        vResultHiB2 = __vmaddfp(vCosB2, vDiffB2, vZero);
        vResultHiB2 = __vmaddfp(vSinB2, vSwappedDiffB2, vResultHiB2);

        ////////////////////////////////////////////////////////////////////////////////
        // store results of second stage butterfly
        ////////////////////////////////////////////////////////////////////////////////
        *(__vector4 *)dstPtr = vResultLoB1;
        *(__vector4 *)(dstPtr + 4) = vResultHiB1;
        *(__vector4 *)(dstPtr + 8) = vResultLoB2;
        *(__vector4 *)(dstPtr + 12) = vResultHiB2;

        ////////////////////////////////////////////////////////////////////////////////
        // update pointers and sin/cos root index for next time through loop
        ////////////////////////////////////////////////////////////////////////////////
        srcPtr += 4;
        dstPtr += 16;
        root += 4;
    } while (root < len / 2);

    ////////////////////////////////////////////////////////////////////////////////
    // for second step, source is temp buffer, and destination is original
    // data buffer
    ////////////////////////////////////////////////////////////////////////////////
    srcPtr = pTempBuffer;
    dstPtr = pData;
    trig *= 4;

    ////////////////////////////////////////////////////////////////////////////////
    // In the ping-pong FFT, for each power of two, butterflies are calculated
    // using all elements in the data.  To eliminate load/store operations, we
    // perform a "double butterfly", essentially doing two steps of butterflies
    // for each pass through the data.  So, we need to do pow/2 passes through
    // the data.  We've already done the first pass, so we must do (pow-2/2) loops
    // through the data at this point.
    ////////////////////////////////////////////////////////////////////////////////
    for (i = (pow - 2) / 2; i > 0; i--) {
        ////////////////////////////////////////////////////////////////////////////////
        // initialize source pointers
        ////////////////////////////////////////////////////////////////////////////////
        pInVec1 = (__vector4 *)srcPtr;
        pInVec2 = (__vector4 *)&srcPtr[len];
        pInVec3 = (__vector4 *)&srcPtr[len / 2];
        pInVec4 = (__vector4 *)&srcPtr[len + len / 2];

        root = 0;

        ////////////////////////////////////////////////////////////////////////////////
        // if this is the last time through the loop, and the length is an even power
        // of two, but an odd power of four, then the source data is the input buffer,
        // and the dest data would be the temp buffer.  However, for the last step,
        // the input data and output data indices for the butterflies are the same, so
        // we can write directly back into the source data.  This eliminates the need
        // to copy from the temp buffer back to the original buffer to return the
        // fft result data.
        ////////////////////////////////////////////////////////////////////////////////
        if (i == 1) {
            ////////////////////////////////////////////////////////////////////////////////
            // if the length is an even power of two, then this is the last iteration that
            // we will go through.  Otherwise we'll do an additional single butterfly
            // pass through the data.
            ////////////////////////////////////////////////////////////////////////////////
            if (!(pow & 1)) {
                ////////////////////////////////////////////////////////////////////////////////
                // we're copying from original buffer back into original buffer
                ////////////////////////////////////////////////////////////////////////////////
                if ((pow & 3) == 2) {
                    dstPtr = srcPtr;
                }

                ////////////////////////////////////////////////////////////////////////////////
                // load sin & cos for first butterfly, and generate sin & cos vectors
                ////////////////////////////////////////////////////////////////////////////////
                vCSLoad1 = *(__vector4 *)&sinCosTable[root];
                vCosA1 = __vspltw(vCSLoad1, 0);
                vCosA2 = __vspltw(vCSLoad1, 1);
                vSinA1 = __vmaddfp(vCosA2, vSinSignMultiplier, vZero);
                vCosA2 = __vsubfp(vZero, vCosA2);
                vSinA2 = __vmaddfp(vCosA1, vSinSignMultiplier, vZero);

                ////////////////////////////////////////////////////////////////////////////////
                // set up output data pointers
                ////////////////////////////////////////////////////////////////////////////////
                pOutVec1 = ((__vector4 *)dstPtr) + root;
                pOutVec2 = ((__vector4 *)(dstPtr + trig * 4)) + root;
                pOutVec3 = ((__vector4 *)(dstPtr + trig * 2)) + root;
                pOutVec4 = ((__vector4 *)(dstPtr + trig * 6)) + root;

                if (isign > 0) {
                    ////////////////////////////////////////////////////////////////////////////////
                    // we are performing an inverse FFT, so we need to divide the final results by
                    // the length of the FFT input data.  Since there is no vector divide, we
                    // create a float vector of 1/N, and then do a multiply of the data before
                    // we store it back.
                    //
                    // We use a transition vector to go from the scalar to vector domain.
                    // We don't store directly to our multiplier vector, so that the compiler can
                    // more easily optimize the multiplier vector to be register-based
                    // rather than stack-based.
                    ////////////////////////////////////////////////////////////////////////////////
                    vTransitionFloatVector.v[0] = 1.0f / len;
                    vInverseDivideMultiplier = __vspltw(vTransitionFloatVector, 0);

                    for (j = trig / 4; j > 0; j--) {
                        ////////////////////////////////////////////////////////////////////////////////
                        // load in four input vectors
                        ////////////////////////////////////////////////////////////////////////////////
                        vIn1 = *pInVec1++;
                        vIn2 = *pInVec2++;
                        vIn3 = *pInVec3++;
                        vIn4 = *pInVec4++;

                        ////////////////////////////////////////////////////////////////////////////////
                        // calculate butterflies for first stage
                        ////////////////////////////////////////////////////////////////////////////////
                        vDiffA1 = __vsubfp(vIn1, vIn2);
                        vDiffA2 = __vsubfp(vIn3, vIn4);
                        vSumA1 = __vaddfp(vIn1, vIn2);
                        vSumA2 = __vaddfp(vIn3, vIn4);
                        vSwappedDiffA1 = __vperm(vDiffA1, vDiffA1, vSwappedPerm.v);
                        vSwappedDiffA2 = __vperm(vDiffA2, vDiffA2, vSwappedPerm.v);
                        vButterflyA1 = __vmaddfp(vDiffA1, vCosA1, vZero);
                        vButterflyA1 = __vmaddfp(vSwappedDiffA1, vSinA1, vButterflyA1);
                        vButterflyA2 = __vmaddfp(vDiffA2, vCosA2, vZero);
                        vButterflyA2 = __vmaddfp(vSwappedDiffA2, vSinA2, vButterflyA2);

                        ////////////////////////////////////////////////////////////////////////////////
                        // calculate butterflies for second stage
                        ////////////////////////////////////////////////////////////////////////////////
                        vDiffB1 = __vsubfp(vSumA1, vSumA2);
                        vDiffB2 = __vsubfp(vButterflyA1, vButterflyA2);
                        vResultLoB1 = __vaddfp(vSumA1, vSumA2);
                        vResultLoB2 = __vaddfp(vButterflyA1, vButterflyA2);
                        vSwappedDiffB1 = __vperm(vDiffB1, vDiffB1, vSwappedPerm.v);
                        vSwappedDiffB2 = __vperm(vDiffB2, vDiffB2, vSwappedPerm.v);
                        vResultHiB1 = __vmaddfp(vCosA1, vDiffB1, vZero);
                        vResultHiB1 = __vmaddfp(vSinA1, vSwappedDiffB1, vResultHiB1);
                        vResultHiB2 = __vmaddfp(vCosA1, vDiffB2, vZero);
                        vResultHiB2 = __vmaddfp(vSinA1, vSwappedDiffB2, vResultHiB2);

                        ////////////////////////////////////////////////////////////////////////////////
                        // since we're performing an inverse FFT, we now divide each element by len
                        // before we store it back.
                        ////////////////////////////////////////////////////////////////////////////////
                        vResultLoB1 =
                            __vmaddfp(vResultLoB1, vInverseDivideMultiplier, vZero);
                        vResultLoB2 =
                            __vmaddfp(vResultLoB2, vInverseDivideMultiplier, vZero);
                        vResultHiB1 =
                            __vmaddfp(vResultHiB1, vInverseDivideMultiplier, vZero);
                        vResultHiB2 =
                            __vmaddfp(vResultHiB2, vInverseDivideMultiplier, vZero);

                        ////////////////////////////////////////////////////////////////////////////////
                        // For speed, we unroll the loop once. This allows the compiler to better
                        // optimize the object code.  Because the compiler doesn't move loads and
                        // stores around each other, the code is faster if we explicitly move the
                        // second set of input vector loads above the first set of output vector
                        // stores, which allows the compiler to better optimize the dispatch of
                        // the input loads.
                        ////////////////////////////////////////////////////////////////////////////////

                        ////////////////////////////////////////////////////////////////////////////////
                        // load input vectors for second half of unrolled loop
                        ////////////////////////////////////////////////////////////////////////////////
                        vIn1 = *pInVec1++;
                        vIn2 = *pInVec2++;
                        vIn3 = *pInVec3++;
                        vIn4 = *pInVec4++;

                        ////////////////////////////////////////////////////////////////////////////////
                        // store second stage butterfly output of first half of unrolled loop to
                        // dest buffer
                        ////////////////////////////////////////////////////////////////////////////////
                        *pOutVec1++ = vResultLoB1;
                        *pOutVec2++ = vResultHiB1;
                        *pOutVec3++ = vResultLoB2;
                        *pOutVec4++ = vResultHiB2;

                        ////////////////////////////////////////////////////////////////////////////////
                        // calculate butterflies for first stage
                        ////////////////////////////////////////////////////////////////////////////////
                        vDiffA1 = __vsubfp(vIn1, vIn2);
                        vDiffA2 = __vsubfp(vIn3, vIn4);
                        vSumA1 = __vaddfp(vIn1, vIn2);
                        vSumA2 = __vaddfp(vIn3, vIn4);
                        vSwappedDiffA1 = __vperm(vDiffA1, vDiffA1, vSwappedPerm.v);
                        vSwappedDiffA2 = __vperm(vDiffA2, vDiffA2, vSwappedPerm.v);
                        vButterflyA1 = __vmaddfp(vDiffA1, vCosA1, vZero);
                        vButterflyA1 = __vmaddfp(vSwappedDiffA1, vSinA1, vButterflyA1);
                        vButterflyA2 = __vmaddfp(vDiffA2, vCosA2, vZero);
                        vButterflyA2 = __vmaddfp(vSwappedDiffA2, vSinA2, vButterflyA2);

                        ////////////////////////////////////////////////////////////////////////////////
                        // calculate butterflies for second stage
                        ////////////////////////////////////////////////////////////////////////////////
                        vDiffB1 = __vsubfp(vSumA1, vSumA2);
                        vDiffB2 = __vsubfp(vButterflyA1, vButterflyA2);
                        vResultLoB1 = __vaddfp(vSumA1, vSumA2);
                        vResultLoB2 = __vaddfp(vButterflyA1, vButterflyA2);
                        vSwappedDiffB1 = __vperm(vDiffB1, vDiffB1, vSwappedPerm.v);
                        vSwappedDiffB2 = __vperm(vDiffB2, vDiffB2, vSwappedPerm.v);
                        vResultHiB1 = __vmaddfp(vCosA1, vDiffB1, vZero);
                        vResultHiB1 = __vmaddfp(vSinA1, vSwappedDiffB1, vResultHiB1);
                        vResultHiB2 = __vmaddfp(vCosA1, vDiffB2, vZero);
                        vResultHiB2 = __vmaddfp(vSinA1, vSwappedDiffB2, vResultHiB2);

                        ////////////////////////////////////////////////////////////////////////////////
                        // since we're performing an inverse FFT, we now divide each element by len
                        // before we store it back.
                        ////////////////////////////////////////////////////////////////////////////////
                        vResultLoB1 =
                            __vmaddfp(vResultLoB1, vInverseDivideMultiplier, vZero);
                        vResultLoB2 =
                            __vmaddfp(vResultLoB2, vInverseDivideMultiplier, vZero);
                        vResultHiB1 =
                            __vmaddfp(vResultHiB1, vInverseDivideMultiplier, vZero);
                        vResultHiB2 =
                            __vmaddfp(vResultHiB2, vInverseDivideMultiplier, vZero);

                        ////////////////////////////////////////////////////////////////////////////////
                        // store second stage butterfly output of second half of unrolled loop to
                        // dest buffer
                        ////////////////////////////////////////////////////////////////////////////////
                        *pOutVec1++ = vResultLoB1;
                        *pOutVec2++ = vResultHiB1;
                        *pOutVec3++ = vResultLoB2;
                        *pOutVec4++ = vResultHiB2;
                    }
                } else {
                    ////////////////////////////////////////////////////////////////////////////////
                    // perform normal butterfly calculations for forward FFT
                    ////////////////////////////////////////////////////////////////////////////////
                    for (j = trig / 4; j > 0; j--) {
                        ////////////////////////////////////////////////////////////////////////////////
                        // load in four input vectors
                        ////////////////////////////////////////////////////////////////////////////////
                        vIn1 = *pInVec1++;
                        vIn2 = *pInVec2++;
                        vIn3 = *pInVec3++;
                        vIn4 = *pInVec4++;

                        ////////////////////////////////////////////////////////////////////////////////
                        // calculate butterflies for first stage
                        ////////////////////////////////////////////////////////////////////////////////
                        vDiffA1 = __vsubfp(vIn1, vIn2);
                        vDiffA2 = __vsubfp(vIn3, vIn4);
                        vSumA1 = __vaddfp(vIn1, vIn2);
                        vSumA2 = __vaddfp(vIn3, vIn4);
                        vSwappedDiffA1 = __vperm(vDiffA1, vDiffA1, vSwappedPerm.v);
                        vSwappedDiffA2 = __vperm(vDiffA2, vDiffA2, vSwappedPerm.v);
                        vButterflyA1 = __vmaddfp(vDiffA1, vCosA1, vZero);
                        vButterflyA1 = __vmaddfp(vSwappedDiffA1, vSinA1, vButterflyA1);
                        vButterflyA2 = __vmaddfp(vDiffA2, vCosA2, vZero);
                        vButterflyA2 = __vmaddfp(vSwappedDiffA2, vSinA2, vButterflyA2);

                        ////////////////////////////////////////////////////////////////////////////////
                        // calculate butterflies for second stage
                        ////////////////////////////////////////////////////////////////////////////////
                        vDiffB1 = __vsubfp(vSumA1, vSumA2);
                        vDiffB2 = __vsubfp(vButterflyA1, vButterflyA2);
                        vResultLoB1 = __vaddfp(vSumA1, vSumA2);
                        vResultLoB2 = __vaddfp(vButterflyA1, vButterflyA2);
                        vSwappedDiffB1 = __vperm(vDiffB1, vDiffB1, vSwappedPerm.v);
                        vSwappedDiffB2 = __vperm(vDiffB2, vDiffB2, vSwappedPerm.v);
                        vResultHiB1 = __vmaddfp(vCosA1, vDiffB1, vZero);
                        vResultHiB1 = __vmaddfp(vSinA1, vSwappedDiffB1, vResultHiB1);
                        vResultHiB2 = __vmaddfp(vCosA1, vDiffB2, vZero);
                        vResultHiB2 = __vmaddfp(vSinA1, vSwappedDiffB2, vResultHiB2);

                        ////////////////////////////////////////////////////////////////////////////////
                        // For speed, we unroll the loop once. This allows the compiler to better
                        // optimize the object code.  Because the compiler doesn't move loads and
                        // stores around each other, the code is faster if we explicitly move the
                        // second set of input vector loads above the first set of output vector
                        // stores, which allows the compiler to better optimize the dispatch of
                        // the input loads.
                        ////////////////////////////////////////////////////////////////////////////////

                        ////////////////////////////////////////////////////////////////////////////////
                        // load input vectors for second half of
                        // unrolled loop
                        ////////////////////////////////////////////////////////////////////////////////
                        vIn1 = *pInVec1++;
                        vIn2 = *pInVec2++;
                        vIn3 = *pInVec3++;
                        vIn4 = *pInVec4++;

                        ////////////////////////////////////////////////////////////////////////////////
                        // store second stage butterfly output of  first half of unrolled loop to
                        // dest buffer
                        ////////////////////////////////////////////////////////////////////////////////
                        *pOutVec1++ = vResultLoB1;
                        *pOutVec2++ = vResultHiB1;
                        *pOutVec3++ = vResultLoB2;
                        *pOutVec4++ = vResultHiB2;

                        ////////////////////////////////////////////////////////////////////////////////
                        // calculate butterflies for first stage
                        ////////////////////////////////////////////////////////////////////////////////
                        vDiffA1 = __vsubfp(vIn1, vIn2);
                        vDiffA2 = __vsubfp(vIn3, vIn4);
                        vSumA1 = __vaddfp(vIn1, vIn2);
                        vSumA2 = __vaddfp(vIn3, vIn4);
                        vSwappedDiffA1 = __vperm(vDiffA1, vDiffA1, vSwappedPerm.v);
                        vSwappedDiffA2 = __vperm(vDiffA2, vDiffA2, vSwappedPerm.v);
                        vButterflyA1 = __vmaddfp(vDiffA1, vCosA1, vZero);
                        vButterflyA1 = __vmaddfp(vSwappedDiffA1, vSinA1, vButterflyA1);
                        vButterflyA2 = __vmaddfp(vDiffA2, vCosA2, vZero);
                        vButterflyA2 = __vmaddfp(vSwappedDiffA2, vSinA2, vButterflyA2);

                        ////////////////////////////////////////////////////////////////////////////////
                        // calculate butterflies for second stage
                        ////////////////////////////////////////////////////////////////////////////////
                        vDiffB1 = __vsubfp(vSumA1, vSumA2);
                        vDiffB2 = __vsubfp(vButterflyA1, vButterflyA2);
                        vResultLoB1 = __vaddfp(vSumA1, vSumA2);
                        vResultLoB2 = __vaddfp(vButterflyA1, vButterflyA2);
                        vSwappedDiffB1 = __vperm(vDiffB1, vDiffB1, vSwappedPerm.v);
                        vSwappedDiffB2 = __vperm(vDiffB2, vDiffB2, vSwappedPerm.v);
                        vResultHiB1 = __vmaddfp(vCosA1, vDiffB1, vZero);
                        vResultHiB1 = __vmaddfp(vSinA1, vSwappedDiffB1, vResultHiB1);
                        vResultHiB2 = __vmaddfp(vCosA1, vDiffB2, vZero);
                        vResultHiB2 = __vmaddfp(vSinA1, vSwappedDiffB2, vResultHiB2);

                        ////////////////////////////////////////////////////////////////////////////////
                        // store second stage butterfly output of second half of unrolled loop to
                        // dest buffer
                        ////////////////////////////////////////////////////////////////////////////////
                        *pOutVec1++ = vResultLoB1;
                        *pOutVec2++ = vResultHiB1;
                        *pOutVec3++ = vResultLoB2;
                        *pOutVec4++ = vResultHiB2;
                    }
                }
                return result;
            }
        }

        ////////////////////////////////////////////////////////////////////////////////
        // a special case for what would be the first iteration through the
        // "while (root < len/2)" loop below, for which root = 0.  When root is
        // 0, the sin cos table loads for root are the same as the sin cos table
        // loads for 2*root, so we special-case this instance to avoid the extra load and
        // permutes to setup the sin & cos vectors.
        ////////////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////////////
        // load sin & cos for first butterfly, and generate sin & cos vectors
        ////////////////////////////////////////////////////////////////////////////////
        vCSLoad1 = *(__vector4 *)&sinCosTable[root];
        vCosA1 = __vspltw(vCSLoad1, 0);
        vCosA2 = __vspltw(vCSLoad1, 1);
        vSinA1 = __vmaddfp(vCosA2, vSinSignMultiplier, vZero);
        vCosA2 = __vsubfp(vZero, vCosA2);
        vSinA2 = __vmaddfp(vCosA1, vSinSignMultiplier, vZero);

        ////////////////////////////////////////////////////////////////////////////////
        // set up output data pointers
        ////////////////////////////////////////////////////////////////////////////////
        pOutVec1 = ((__vector4 *)dstPtr) + root;
        pOutVec2 = ((__vector4 *)(dstPtr + trig * 4)) + root;
        pOutVec3 = ((__vector4 *)(dstPtr + trig * 2)) + root;
        pOutVec4 = ((__vector4 *)(dstPtr + trig * 6)) + root;

        for (j = trig / 4; j > 0; j--) {
            ////////////////////////////////////////////////////////////////////////////////
            // load in four input vectors
            ////////////////////////////////////////////////////////////////////////////////
            vIn1 = *pInVec1++;
            vIn2 = *pInVec2++;
            vIn3 = *pInVec3++;
            vIn4 = *pInVec4++;

            ////////////////////////////////////////////////////////////////////////////////
            // calculate butterflies for first stage
            ////////////////////////////////////////////////////////////////////////////////
            vDiffA1 = __vsubfp(vIn1, vIn2);
            vDiffA2 = __vsubfp(vIn3, vIn4);
            vSumA1 = __vaddfp(vIn1, vIn2);
            vSumA2 = __vaddfp(vIn3, vIn4);
            vSwappedDiffA1 = __vperm(vDiffA1, vDiffA1, vSwappedPerm.v);
            vSwappedDiffA2 = __vperm(vDiffA2, vDiffA2, vSwappedPerm.v);
            vButterflyA1 = __vmaddfp(vDiffA1, vCosA1, vZero);
            vButterflyA1 = __vmaddfp(vSwappedDiffA1, vSinA1, vButterflyA1);
            vButterflyA2 = __vmaddfp(vDiffA2, vCosA2, vZero);
            vButterflyA2 = __vmaddfp(vSwappedDiffA2, vSinA2, vButterflyA2);

            ////////////////////////////////////////////////////////////////////////////////
            // calculate butterflies for second stage
            ////////////////////////////////////////////////////////////////////////////////
            vDiffB1 = __vsubfp(vSumA1, vSumA2);
            vDiffB2 = __vsubfp(vButterflyA1, vButterflyA2);
            vResultLoB1 = __vaddfp(vSumA1, vSumA2);
            vResultLoB2 = __vaddfp(vButterflyA1, vButterflyA2);
            vSwappedDiffB1 = __vperm(vDiffB1, vDiffB1, vSwappedPerm.v);
            vSwappedDiffB2 = __vperm(vDiffB2, vDiffB2, vSwappedPerm.v);
            vResultHiB1 = __vmaddfp(vCosA1, vDiffB1, vZero);
            vResultHiB1 = __vmaddfp(vSinA1, vSwappedDiffB1, vResultHiB1);
            vResultHiB2 = __vmaddfp(vCosA1, vDiffB2, vZero);
            vResultHiB2 = __vmaddfp(vSinA1, vSwappedDiffB2, vResultHiB2);

            ////////////////////////////////////////////////////////////////////////////////
            // For speed, we unroll the loop once. This allows the compiler to better
            // optimize the object code.  Because the compiler doesn't move loads and
            // stores around each other, the code is faster if we explicitly move the
            // second set of input vector loads above the first set of output vector
            // stores, which allows the compiler to better optimize the dispatch of
            // the input loads.
            ////////////////////////////////////////////////////////////////////////////////


            ////////////////////////////////////////////////////////////////////////////////
            // load input vectors for second half of unrolled loop
            ////////////////////////////////////////////////////////////////////////////////
            vIn1 = *pInVec1++;
            vIn2 = *pInVec2++;
            vIn3 = *pInVec3++;
            vIn4 = *pInVec4++;

            ////////////////////////////////////////////////////////////////////////////////
            // store second stage butterfly output of first half of unrolled loop to
            // dest buffer
            ////////////////////////////////////////////////////////////////////////////////
            *pOutVec1++ = vResultLoB1;
            *pOutVec2++ = vResultHiB1;
            *pOutVec3++ = vResultLoB2;
            *pOutVec4++ = vResultHiB2;

            ////////////////////////////////////////////////////////////////////////////////
            // calculate butterflies for first stage
            ////////////////////////////////////////////////////////////////////////////////
            vDiffA1 = __vsubfp(vIn1, vIn2);
            vDiffA2 = __vsubfp(vIn3, vIn4);
            vSumA1 = __vaddfp(vIn1, vIn2);
            vSumA2 = __vaddfp(vIn3, vIn4);
            vSwappedDiffA1 = __vperm(vDiffA1, vDiffA1, vSwappedPerm.v);
            vSwappedDiffA2 = __vperm(vDiffA2, vDiffA2, vSwappedPerm.v);
            vButterflyA1 = __vmaddfp(vDiffA1, vCosA1, vZero);
            vButterflyA1 = __vmaddfp(vSwappedDiffA1, vSinA1, vButterflyA1);
            vButterflyA2 = __vmaddfp(vDiffA2, vCosA2, vZero);
            vButterflyA2 = __vmaddfp(vSwappedDiffA2, vSinA2, vButterflyA2);

            ////////////////////////////////////////////////////////////////////////////////
            // calculate butterflies for second stage
            ////////////////////////////////////////////////////////////////////////////////
            vDiffB1 = __vsubfp(vSumA1, vSumA2);
            vDiffB2 = __vsubfp(vButterflyA1, vButterflyA2);
            vResultLoB1 = __vaddfp(vSumA1, vSumA2);
            vResultLoB2 = __vaddfp(vButterflyA1, vButterflyA2);
            vSwappedDiffB1 = __vperm(vDiffB1, vDiffB1, vSwappedPerm.v);
            vSwappedDiffB2 = __vperm(vDiffB2, vDiffB2, vSwappedPerm.v);
            vResultHiB1 = __vmaddfp(vCosA1, vDiffB1, vZero);
            vResultHiB1 = __vmaddfp(vSinA1, vSwappedDiffB1, vResultHiB1);
            vResultHiB2 = __vmaddfp(vCosA1, vDiffB2, vZero);
            vResultHiB2 = __vmaddfp(vSinA1, vSwappedDiffB2, vResultHiB2);

            ////////////////////////////////////////////////////////////////////////////////
            // store second stage butterfly output of second half of unrolled loop to
            // dest buffer
            ////////////////////////////////////////////////////////////////////////////////
            *pOutVec1++ = vResultLoB1;
            *pOutVec2++ = vResultHiB1;
            *pOutVec3++ = vResultLoB2;
            *pOutVec4++ = vResultHiB2;
        }

        ////////////////////////////////////////////////////////////////////////////////
        // update root value for sin & cos load
        ////////////////////////////////////////////////////////////////////////////////
        root += 2 * trig;

        while (root < len / 2) {
            ////////////////////////////////////////////////////////////////////////////////
            // load sin & cos
            ////////////////////////////////////////////////////////////////////////////////
            vCSLoad1 = *(__vector4 *)&sinCosTable[root];

            ////////////////////////////////////////////////////////////////////////////////
            // create vector
            //
            //  vCosA1 = cos(root*pi/len) cos(root*pi/len) cos((root+2)*pi/len) cos((root+2)*pi/len)
            ////////////////////////////////////////////////////////////////////////////////
            vCosA1 = __vspltw(vCSLoad1, 0);

            ////////////////////////////////////////////////////////////////////////////////
            // create vector
            //
            //  vCosA2 = -cos(pi/2 + root*pi/len) -cos(pi/2 + root*pi/len) -cos(pi/2 + (root+2)*pi/len) -cos(pi/2 + (root+2)*pi/len)
            ////////////////////////////////////////////////////////////////////////////////
            vCosA2 = __vspltw(vCSLoad1, 1);

            ////////////////////////////////////////////////////////////////////////////////
            // create vector
            //
            //  vSinA1 = sin(root*pi/len) -sin(root*pi/len) sin((root+2)*pi/len) -sin((root+2)*pi/len)
            //
            // or
            //
            //  vSinA1 = -sin(root*pi/len) sin(root*pi/len) -sin((root+2)*pi/len) sin((root+2)*pi/len)
            //
            // depending on whether we are doing a forward or inverse fft
            ////////////////////////////////////////////////////////////////////////////////
            vSinA1 = __vmaddfp(vCosA2, vSinSignMultiplier, vZero);

            ////////////////////////////////////////////////////////////////////////////////
            // turn negative cos to positive cos
            ////////////////////////////////////////////////////////////////////////////////
            vCosA2 = __vsubfp(vZero, vCosA2);

            ////////////////////////////////////////////////////////////////////////////////
            // create vector
            //
            //  vSinA1 = sin(pi/2 + root*pi/len) -sin(pi/2 + root*pi/len) sin(pi/2 + (root+2)*pi/len) -sin(pi/2 + (root+2)*pi/len)
            //
            // or
            //
            //  vSinA1 = -sin(pi/2 + root*pi/len) sin(pi/2 + root*pi/len) -sin(pi/2 + (root+2)*pi/len) sin(pi/2 + (root+2)*pi/len)
            //
            // depending on whether we are doing a forward or inverse fft
            ////////////////////////////////////////////////////////////////////////////////
            vSinA2 = __vmaddfp(vCosA1, vSinSignMultiplier, vZero);

            ////////////////////////////////////////////////////////////////////////////////
            // load sin & cos and generate sin, cos vectors for second-stage butterfly
            ////////////////////////////////////////////////////////////////////////////////
            vCSLoadB1 = *(__vector4 *)&sinCosTable[2 * root];
            vSinB1 = __vspltw(vCSLoadB1, 1);
            vCosB1 = __vspltw(vCSLoadB1, 0);
            vNegSinB1 = __vsubfp(vZero, vSinB1);
            vSinB1 = __vsel(vSinB1, vNegSinB1, vSinNegSinSelect);

            ////////////////////////////////////////////////////////////////////////////////
            // set up output data pointers
            ////////////////////////////////////////////////////////////////////////////////
            pOutVec1 = ((__vector4 *)dstPtr) + root;
            pOutVec2 = ((__vector4 *)(dstPtr + trig * 4)) + root;
            pOutVec3 = ((__vector4 *)(dstPtr + trig * 2)) + root;
            pOutVec4 = ((__vector4 *)(dstPtr + trig * 6)) + root;

            for (j = trig / 4; j > 0; j--) {
                ////////////////////////////////////////////////////////////////////////////////
                // load in four input vectors
                ////////////////////////////////////////////////////////////////////////////////
                vIn1 = *pInVec1++;
                vIn2 = *pInVec2++;
                vIn3 = *pInVec3++;
                vIn4 = *pInVec4++;

                ////////////////////////////////////////////////////////////////////////////////
                // calculate butterflies for first stage
                ////////////////////////////////////////////////////////////////////////////////
                vDiffA1 = __vsubfp(vIn1, vIn2);
                vDiffA2 = __vsubfp(vIn3, vIn4);
                vSumA1 = __vaddfp(vIn1, vIn2);
                vSumA2 = __vaddfp(vIn3, vIn4);
                vSwappedDiffA1 = __vperm(vDiffA1, vDiffA1, vSwappedPerm.v);
                vSwappedDiffA2 = __vperm(vDiffA2, vDiffA2, vSwappedPerm.v);
                vButterflyA1 = __vmaddfp(vDiffA1, vCosA1, vZero);
                vButterflyA1 = __vmaddfp(vSwappedDiffA1, vSinA1, vButterflyA1);
                vButterflyA2 = __vmaddfp(vDiffA2, vCosA2, vZero);
                vButterflyA2 = __vmaddfp(vSwappedDiffA2, vSinA2, vButterflyA2);

                ////////////////////////////////////////////////////////////////////////////////
                // calculate butterflies for second stage
                ////////////////////////////////////////////////////////////////////////////////
                vDiffB1 = __vsubfp(vSumA1, vSumA2);
                vDiffB2 = __vsubfp(vButterflyA1, vButterflyA2);
                vResultLoB1 = __vaddfp(vSumA1, vSumA2);
                vResultLoB2 = __vaddfp(vButterflyA1, vButterflyA2);
                vSwappedDiffB1 = __vperm(vDiffB1, vDiffB1, vSwappedPerm.v);
                vSwappedDiffB2 = __vperm(vDiffB2, vDiffB2, vSwappedPerm.v);
                vResultHiB1 = __vmaddfp(vCosB1, vDiffB1, vZero);
                vResultHiB1 = __vmaddfp(vSinB1, vSwappedDiffB1, vResultHiB1);
                vResultHiB2 = __vmaddfp(vCosB1, vDiffB2, vZero);
                vResultHiB2 = __vmaddfp(vSinB1, vSwappedDiffB2, vResultHiB2);

                ////////////////////////////////////////////////////////////////////////////////
                // For speed, we unroll the loop once. This allows the compiler to better
                // optimize the object code.  Because the compiler doesn't move loads and
                // stores around each other, the code is faster if we explicitly move the
                // second set of input vector loads above the first set of output vector
                // stores, which allows the compiler to better optimize the dispatch of
                // the input loads.
                ////////////////////////////////////////////////////////////////////////////////

                ////////////////////////////////////////////////////////////////////////////////
                // load input vectors for second half of
                // unrolled loop
                ////////////////////////////////////////////////////////////////////////////////
                vIn1 = *pInVec1++;
                vIn2 = *pInVec2++;
                vIn3 = *pInVec3++;
                vIn4 = *pInVec4++;

                ////////////////////////////////////////////////////////////////////////////////
                // store second stage butterfly output of first half of unrolled loop to
                // dest buffer
                ////////////////////////////////////////////////////////////////////////////////
                *pOutVec1++ = vResultLoB1;
                *pOutVec2++ = vResultHiB1;
                *pOutVec3++ = vResultLoB2;
                *pOutVec4++ = vResultHiB2;

                ////////////////////////////////////////////////////////////////////////////////
                // calculate butterflies for first stage
                ////////////////////////////////////////////////////////////////////////////////
                vDiffA1 = __vsubfp(vIn1, vIn2);
                vDiffA2 = __vsubfp(vIn3, vIn4);
                vSumA1 = __vaddfp(vIn1, vIn2);
                vSumA2 = __vaddfp(vIn3, vIn4);
                vSwappedDiffA1 = __vperm(vDiffA1, vDiffA1, vSwappedPerm.v);
                vSwappedDiffA2 = __vperm(vDiffA2, vDiffA2, vSwappedPerm.v);
                vButterflyA1 = __vmaddfp(vDiffA1, vCosA1, vZero);
                vButterflyA1 = __vmaddfp(vSwappedDiffA1, vSinA1, vButterflyA1);
                vButterflyA2 = __vmaddfp(vDiffA2, vCosA2, vZero);
                vButterflyA2 = __vmaddfp(vSwappedDiffA2, vSinA2, vButterflyA2);

                ////////////////////////////////////////////////////////////////////////////////
                // calculate butterflies for second stage
                ////////////////////////////////////////////////////////////////////////////////
                vDiffB1 = __vsubfp(vSumA1, vSumA2);
                vDiffB2 = __vsubfp(vButterflyA1, vButterflyA2);
                vResultLoB1 = __vaddfp(vSumA1, vSumA2);
                vResultLoB2 = __vaddfp(vButterflyA1, vButterflyA2);
                vSwappedDiffB1 = __vperm(vDiffB1, vDiffB1, vSwappedPerm.v);
                vSwappedDiffB2 = __vperm(vDiffB2, vDiffB2, vSwappedPerm.v);
                vResultHiB1 = __vmaddfp(vCosB1, vDiffB1, vZero);
                vResultHiB1 = __vmaddfp(vSinB1, vSwappedDiffB1, vResultHiB1);
                vResultHiB2 = __vmaddfp(vCosB1, vDiffB2, vZero);
                vResultHiB2 = __vmaddfp(vSinB1, vSwappedDiffB2, vResultHiB2);

                ////////////////////////////////////////////////////////////////////////////////
                // store second stage butterfly output of second half of unrolled loop to
                // dest buffer
                ////////////////////////////////////////////////////////////////////////////////
                *pOutVec1++ = vResultLoB1;
                *pOutVec2++ = vResultHiB1;
                *pOutVec3++ = vResultLoB2;
                *pOutVec4++ = vResultHiB2;
            }

            root += 2 * trig;
        }

        trig *= 4;
        tmp = dstPtr;
        dstPtr = srcPtr;
        srcPtr = tmp;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // if the length is an odd power of two, then we need to do one more
    // single-butterfly pass through the data.  Since we know that the sin and cos
    // values for this last pass are 0 and 1, we can simplify the butterfly
    // calculations to adds and subtracts.
    ////////////////////////////////////////////////////////////////////////////////
    if (pow & 1) {
        ////////////////////////////////////////////////////////////////////////////////
        // output always goes back to original input data buffer
        ////////////////////////////////////////////////////////////////////////////////
        pOutVec1 = (__vector4 *)pData;

        ////////////////////////////////////////////////////////////////////////////////
        // set source pointer to load from whatever the last step's output data was
        ////////////////////////////////////////////////////////////////////////////////
        if (pow & 2) {
            pInVec1 = (__vector4 *)pTempBuffer;
        } else {
            pInVec1 = (__vector4 *)pData;
        }

        ////////////////////////////////////////////////////////////////////////////////
        // set second input and output vectors to point half-way
        ////////////////////////////////////////////////////////////////////////////////
        pInVec2 = pInVec1 + (len / 4);
        pOutVec2 = pOutVec1 + (len / 4);

        if (isign > 0) {
            ////////////////////////////////////////////////////////////////////////////////
            // we are performing an inverse FFT, so we need to divide the final results by
            // the length of the FFT input data.  Since there is no vector divide, we create a
            // float vector of 1/N, and then do a multiply of the data before we store it
            // back.
            //
            // We use a transition vector to go from the scalar to vector domain.  We don't store
            // directly to our multiplier vector, so that the compiler can more easily optimize
            // the multiplier vector to be register-based rather than stack-based.
            ////////////////////////////////////////////////////////////////////////////////
            vTransitionFloatVector.v[0] = 1.0f / len;
            vInverseDivideMultiplier = __vspltw(vTransitionFloatVector, 0);

            for (j = trig / 8; j > 0; j--) {
                ////////////////////////////////////////////////////////////////////////////////
                // load four input vectors
                ////////////////////////////////////////////////////////////////////////////////
                vInLo1 = *pInVec1++;
                vInHi1 = *pInVec2++;
                vInLo2 = *pInVec1++;
                vInHi2 = *pInVec2++;

                ////////////////////////////////////////////////////////////////////////////////
                // calc simplified butterflies, multiplying by inverse correction factor.
                ////////////////////////////////////////////////////////////////////////////////
                vInLo1 = __vmaddfp(vInLo1, vInverseDivideMultiplier, vZero);
                vResultHi1 = __vnmsubfp(vInHi1, vInverseDivideMultiplier, vInLo1);
                vResultLo1 = __vmaddfp(vInHi1, vInverseDivideMultiplier, vInLo1);
                vInLo2 = __vmaddfp(vInLo2, vInverseDivideMultiplier, vZero);
                vResultHi2 = __vnmsubfp(vInHi2, vInverseDivideMultiplier, vInLo2);
                vResultLo2 = __vmaddfp(vInHi2, vInverseDivideMultiplier, vInLo2);

                ////////////////////////////////////////////////////////////////////////////////
                // load input vectors for second half of unrolled loop
                ////////////////////////////////////////////////////////////////////////////////
                vInLo1 = *pInVec1++;
                vInHi1 = *pInVec2++;
                vInLo2 = *pInVec1++;
                vInHi2 = *pInVec2++;

                ////////////////////////////////////////////////////////////////////////////////
                // store results from first half of unrolled loop
                ////////////////////////////////////////////////////////////////////////////////
                *pOutVec1++ = vResultLo1;
                *pOutVec2++ = vResultHi1;
                *pOutVec1++ = vResultLo2;
                *pOutVec2++ = vResultHi2;

                ////////////////////////////////////////////////////////////////////////////////
                // calc simplified butterflies, multiplying by inverse multiplier factor.
                ////////////////////////////////////////////////////////////////////////////////
                vInLo1 = __vmaddfp(vInLo1, vInverseDivideMultiplier, vZero);
                vResultHi1 = __vnmsubfp(vInHi1, vInverseDivideMultiplier, vInLo1);
                vResultLo1 = __vmaddfp(vInHi1, vInverseDivideMultiplier, vInLo1);
                vInLo2 = __vmaddfp(vInLo2, vInverseDivideMultiplier, vZero);
                vResultHi2 = __vnmsubfp(vInHi2, vInverseDivideMultiplier, vInLo2);
                vResultLo2 = __vmaddfp(vInHi2, vInverseDivideMultiplier, vInLo2);

                ////////////////////////////////////////////////////////////////////////////////
                // store results from second half of unrolled loop
                ////////////////////////////////////////////////////////////////////////////////
                *pOutVec1++ = vResultLo1;
                *pOutVec2++ = vResultHi1;
                *pOutVec1++ = vResultLo2;
                *pOutVec2++ = vResultHi2;
            }
        } else {
            for (j = trig / 8; j > 0; j--) {
                ////////////////////////////////////////////////////////////////////////////////
                // load four input vectors
                ////////////////////////////////////////////////////////////////////////////////
                vInLo1 = *pInVec1++;
                vInHi1 = *pInVec2++;
                vInLo2 = *pInVec1++;
                vInHi2 = *pInVec2++;

                ////////////////////////////////////////////////////////////////////////////////
                // calc simplified butterflies
                ////////////////////////////////////////////////////////////////////////////////
                vResultHi1 = __vsubfp(vInLo1, vInHi1);
                vResultLo1 = __vaddfp(vInLo1, vInHi1);
                vResultHi2 = __vsubfp(vInLo2, vInHi2);
                vResultLo2 = __vaddfp(vInLo2, vInHi2);

                ////////////////////////////////////////////////////////////////////////////////
                // load input vectors for second half of unrolled loop
                ////////////////////////////////////////////////////////////////////////////////
                vInLo1 = *pInVec1++;
                vInHi1 = *pInVec2++;
                vInLo2 = *pInVec1++;
                vInHi2 = *pInVec2++;

                ////////////////////////////////////////////////////////////////////////////////
                // store results from first half of unrolled loop
                ////////////////////////////////////////////////////////////////////////////////
                *pOutVec1++ = vResultLo1;
                *pOutVec2++ = vResultHi1;
                *pOutVec1++ = vResultLo2;
                *pOutVec2++ = vResultHi2;

                ////////////////////////////////////////////////////////////////////////////////
                // calc simplified butterflies
                ////////////////////////////////////////////////////////////////////////////////
                vResultHi1 = __vsubfp(vInLo1, vInHi1);
                vResultLo1 = __vaddfp(vInLo1, vInHi1);
                vResultHi2 = __vsubfp(vInLo2, vInHi2);
                vResultLo2 = __vaddfp(vInLo2, vInHi2);

                ////////////////////////////////////////////////////////////////////////////////
                // store results from second half of unrolled loop
                ////////////////////////////////////////////////////////////////////////////////
                *pOutVec1++ = vResultLo1;
                *pOutVec2++ = vResultHi1;
                *pOutVec1++ = vResultLo2;
                *pOutVec2++ = vResultHi2;
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
// fft_scalar
//
// Ping-pong Stockham FFT
//
//  Performs a forward or inverse FFT on the complex signal data
// pointed to by pData. pTempBuffer must point to a buffer that is of equal
// length as the signal pointed to by pData, and which will be overwritten by
// the routine.  If isign == -1, then a forward FFT is performed.  Otherwise
// an inverse FFT is performed.
//
// requirements:
//
//  - length must be an exact power of 2
//
////////////////////////////////////////////////////////////////////////////////
int fft_scalar(
    float *pData, float *tempbuff, unsigned long len, long isign, float *sinCosTable
) {
    long j, i;
    float c, s, tre, tim, *srcDataPtr, *dstDataPtr, *tmp;
    double inverseDivideMultiplier;
    long pow, root, trig;
    int result = 0;

    ////////////////////////////////////////////////////////////////////////////////
    // calculate log2(len)
    ////////////////////////////////////////////////////////////////////////////////
    pow = log2max(len);

    ////////////////////////////////////////////////////////////////////////////////
    // length must be an exact power of 2
    ////////////////////////////////////////////////////////////////////////////////
    if ((1 << pow) != len)
        return EINVAL;

    ////////////////////////////////////////////////////////////////////////////////
    // initialize "trig" step for destination index
    ////////////////////////////////////////////////////////////////////////////////
    trig = 1;

    ////////////////////////////////////////////////////////////////////////////////
    // Start with data as source, and temp buffer as destination
    ////////////////////////////////////////////////////////////////////////////////
    srcDataPtr = pData;
    dstDataPtr = tempbuff;

    for (i = pow - 1; i > 0; i--) {
        root = 0;
        while (root < len / 2) {
            ////////////////////////////////////////////////////////////////////////////////
            // load cos and sin values for current root value.
            ////////////////////////////////////////////////////////////////////////////////
            c = sinCosTable[2 * root];
            s = isign * sinCosTable[2 * root + 1];
            for (j = trig; j > 0; j--) {
                ////////////////////////////////////////////////////////////////////////////////
                // calculate butterflies.  For inputs:
                // [re0, im0] [re1, im1]
                //
                // we calculate:
                //
                // out0 = [re0+re1, im0+im1]
                //
                // out1 = [ cos * (re0-re1) - sin * (im0-im1),
                //          sin * (re0-re1) + cos * (im0-im1) ]
                //
                ////////////////////////////////////////////////////////////////////////////////
                tre = srcDataPtr[0] - srcDataPtr[len];
                tim = srcDataPtr[1] - srcDataPtr[len + 1];
                dstDataPtr[0] = srcDataPtr[0] + srcDataPtr[len];
                dstDataPtr[1] = srcDataPtr[1] + srcDataPtr[len + 1];
                dstDataPtr[2 * trig] = c * tre - s * tim;
                dstDataPtr[2 * trig + 1] = s * tre + c * tim;
                srcDataPtr += 2;
                dstDataPtr += 2;
            }

            ////////////////////////////////////////////////////////////////////////////////
            // update dest pointer and root value
            ////////////////////////////////////////////////////////////////////////////////
            dstDataPtr += 2 * trig;
            root += trig;
        }

        ////////////////////////////////////////////////////////////////////////////////
        // update trig value and ping pong source and dest pointers
        ////////////////////////////////////////////////////////////////////////////////
        trig *= 2;
        srcDataPtr -= len;
        dstDataPtr -= 2 * len;
        tmp = srcDataPtr;
        srcDataPtr = dstDataPtr;
        dstDataPtr = tmp;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // For last iteration, we are fortunate in that the source indices for the
    // butterfly calculations are the same as the destination indices. This is
    // not true for other loop iterations, which is why we cannot simply store
    // our results directly back to the data -- we would overwrite source data
    // that had not yet been used for the current iteration of the loop. If the
    // power of two of the length is odd, then, on the second to last iteration,
    // the data will end up ping-ponged back to the source buffer.  If this is the
    // case, then we set source and dest to be the same. Otherwise, if the power
    // of two is even, then we will be reading from temp data, and writing back
    // to our original buffer.
    ////////////////////////////////////////////////////////////////////////////////
    if (pow & 1) {
        dstDataPtr = srcDataPtr;
    }

    root = 0;
    if (isign > 0) {
        ////////////////////////////////////////////////////////////////////////////////
        // we are performing an inverse FFT.  We need to divide all elements of the
        // final result by len.  Since the float multiply operation is faster than
        // the divide operation, we multiply by the reciprocal.
        ////////////////////////////////////////////////////////////////////////////////
        inverseDivideMultiplier = (double)1 / len;
        while (root < len / 2) {
            ////////////////////////////////////////////////////////////////////////////////
            // load cos and sin values for current root value.
            ////////////////////////////////////////////////////////////////////////////////
            c = sinCosTable[2 * root];
            s = isign * sinCosTable[2 * root + 1];
            for (j = trig; j > 0; j--) {
                ////////////////////////////////////////////////////////////////////////////////
                // calculate butterflies.  For inputs:
                // [re0, im0] [re1, im1]
                //
                // we calculate:
                //
                // out0 = [re0+re1, im0+im1]
                //
                // out1 = [ cos * (re0-re1) - sin * (im0-im1),
                //          sin * (re0-re1) + cos * (im0-im1) ]
                //
                //
                // We also divide each element by the length of the signal (this is done
                // by multiplying by the reciprocal).
                ////////////////////////////////////////////////////////////////////////////////
                tre = srcDataPtr[0] - srcDataPtr[len];
                tim = srcDataPtr[1] - srcDataPtr[len + 1];
                dstDataPtr[0] =
                    (srcDataPtr[0] + srcDataPtr[len]) * inverseDivideMultiplier;
                dstDataPtr[1] =
                    (srcDataPtr[1] + srcDataPtr[len + 1]) * inverseDivideMultiplier;
                dstDataPtr[2 * trig] = (c * tre - s * tim) * inverseDivideMultiplier;
                dstDataPtr[2 * trig + 1] = (s * tre + c * tim) * inverseDivideMultiplier;
                srcDataPtr += 2;
                dstDataPtr += 2;
            }

            ////////////////////////////////////////////////////////////////////////////////
            // update dest pointer and root value
            ////////////////////////////////////////////////////////////////////////////////
            dstDataPtr += 2 * trig;
            root += trig;
        }
    } else {
        while (root < len / 2) {
            ////////////////////////////////////////////////////////////////////////////////
            // load cos and sin values for current root value.
            ////////////////////////////////////////////////////////////////////////////////
            c = sinCosTable[2 * root];
            s = isign * sinCosTable[2 * root + 1];
            for (j = trig; j > 0; j--) {
                ////////////////////////////////////////////////////////////////////////////////
                // calculate butterflies.  For inputs:
                // [re0, im0] [re1, im1]
                //
                // we calculate:
                //
                // out0 = [re0+re1, im0+im1]
                //
                // out1 = [ cos * (re0-re1) - sin * (im0-im1),
                //          sin * (re0-re1) + cos * (im0-im1) ]
                //
                ////////////////////////////////////////////////////////////////////////////////
                tre = srcDataPtr[0] - srcDataPtr[len];
                tim = srcDataPtr[1] - srcDataPtr[len + 1];
                dstDataPtr[0] = srcDataPtr[0] + srcDataPtr[len];
                dstDataPtr[1] = srcDataPtr[1] + srcDataPtr[len + 1];
                dstDataPtr[2 * trig] = c * tre - s * tim;
                dstDataPtr[2 * trig + 1] = s * tre + c * tim;
                srcDataPtr += 2;
                dstDataPtr += 2;
            }

            ////////////////////////////////////////////////////////////////////////////////
            // update dest pointer and root value
            ////////////////////////////////////////////////////////////////////////////////
            dstDataPtr += 2 * trig;
            root += trig;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
//
//  fft_pingpong
//
// This function calls either the scalar or vector implementation of the
// pingpong fft, based on the length of the data.  The vector implementation
// only works above certain lengths because of implementation details.
////////////////////////////////////////////////////////////////////////////////
int fft_pingpong(float *data, unsigned long len, long isign, float *sinCosTable) {
    ////////////////////////////////////////////////////////////////////////////////
    // make sure that our temp buffer is sufficiently large to hold a copy of
    // the data.
    ////////////////////////////////////////////////////////////////////////////////
    int result = EnsureStaticBufferSize(len);
    if (result == 0) {
        if (len < 16) {
            ////////////////////////////////////////////////////////////////////////////////
            // call scalar version
            ////////////////////////////////////////////////////////////////////////////////
            result = fft_scalar(data, gTempBuffer.mBuffer, len, isign, sinCosTable);
        } else {
            ////////////////////////////////////////////////////////////////////////////////
            // call AltiVec version
            ////////////////////////////////////////////////////////////////////////////////
            result = fft_altivec(data, gTempBuffer.mBuffer, len, isign, sinCosTable);
        }
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// CalculateSinCosTable
//  Initialize cos & sin lookup table.
///////////////////////////////////////////////////////////////////////////////////////////////
int CalculateSinCosTable(long len, float *sinCosTable) {
    if (len < 4) {
        ////////////////////////////////////////////////////////////////////////////////
        // always create first entries in table
        ////////////////////////////////////////////////////////////////////////////////
        sinCosTable[0] = 1;
        sinCosTable[1] = 0;

        ////////////////////////////////////////////////////////////////////////////////
        // if len is 2, then add second pair to table
        ////////////////////////////////////////////////////////////////////////////////
        if (len == 2) {
            sinCosTable[2] = -1;
            sinCosTable[3] = 0;
        }
    } else {
        ////////////////////////////////////////////////////////////////////////////////
        // calculate cos & sin for range of
        // [0, pi].
        ////////////////////////////////////////////////////////////////////////////////
        for (long i = 0; i < len / 4; i++) {
            float angle = 2.0 * PI * i / len;
            float curCos = cos(angle);
            float curSin = sin(angle);
            sinCosTable[2 * i] = curCos;
            sinCosTable[2 * i + 1] = curSin;
            sinCosTable[len / 2 + 2 * i] = -curSin;
            sinCosTable[len / 2 + 2 * i + 1] = curCos;
        }
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//  fft_recursive
//
//  Performs a recursive (N/2 by 2) forward or inverse FFT on the
// signal pointed to by pData. This is done in the following manner.  First, the
// signal X is divided into two parts, namely X1 = X(0, N/2-1) and
// X2 = X(N/2, N-1). These two sub-signals are then turned into the sum and
// difference, respectively, of these two signals. This is in effect an FFT
// of length 2 performed between each pair of elements taken from X1 and X2.
// This produces two new subsignals X1' and X2'.
//
// Next, a twist operation is performed on X2', where X2'(j) is multiplied
// by e^(2 pi ij / N).
//
// Next, a length N/2 complex FFT is performed on each of X1' and X2', yielding
// X1" and X2".
//
// Finally, the elements of X1" and X2" are merged, so that the resulting
// length-N signal Y is given by:
//
//  Y = X1"(0) X2"(0) X1"(1) X2"(1) ... X1"(N/2-1) X2"(N/2-1)
//
//  This resulting signal Y is the FFT of the original signal X
//
////////////////////////////////////////////////////////////////////////////////
int fft_recursive(float *pData, unsigned long len, long isign, float *sinCosTable) {
    int result;
    long i;
    __vector4 *pInLoBottom, *pInLoTop, *pInHiBottom, *pInHiTop;
    __vector4 *pOutLoBottom, *pOutLoTop, *pOutHiBottom, *pOutHiTop;
    __vector4 vLoBottom, vLoTop, vHiBottom, vHiTop;
    __vector4 vZero = { 0, 0, 0, 0 };
    __vector4 vSinLo;
    __vector4 vCosLo;
    __vector4 vSinHi;
    __vector4 vCosHi;
    __vector4 vFTransition;
    __vector4 vFNegTransition;
    double updateA;
    double updateB;
    double newCos1;
    double newCos2;
    double newSin1;
    double newSin2;
    double tempCos1;
    double tempCos2;
    double startSinMul;

    ////////////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //  X = x0 x1 x2 x3
    //  Y = y0 y1 y2 y3
    //
    // will generate
    //  Z = x0 y0 x1 y1
    ////////////////////////////////////////////////////////////////////////////////
    XMVECTORU32 vForwardSinPerm = { 0x00010203, 0x10111213, 0x04050607, 0x14151617 };

    ////////////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //  X = x0 x1 x2 x3
    //  Y = y0 y1 y2 y3
    //
    // will generate
    //  Z = y0 x0 y1 x1
    ////////////////////////////////////////////////////////////////////////////////
    XMVECTORU32 vInverseSinPerm = { 0x10111213, 0x00010203, 0x14151617, 0x04050607 };
    __vector4 vSinPerm;
    __vector4 vDiffBottom;
    __vector4 vDiffTop;
    __vector4 vTwistBottom;
    __vector4 vTwistTop;
    __vector4 vSwappedDiffBottom;
    __vector4 vSwappedDiffTop;

    ////////////////////////////////////////////////////////////////////////////////
    // make sure that work buffer is large enough to hold half of entire data
    // length for merge operation at end of routine.
    ////////////////////////////////////////////////////////////////////////////////
    result = EnsureStaticBufferSize(len / 2);

    if (result == 0) {
        ////////////////////////////////////////////////////////////////////////////////
        // initialize a permute vector that, given input vectors:
        //
        //  X = x0 x1 x2 x3
        //  Y = y0 y1 y2 y3
        //
        // will generate
        //  Z = x1 x0 x3 x2
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vSwapPairPerm = { 0x04050607, 0x00010203, 0x0C0D0E0F, 0x08090A0B };

        ////////////////////////////////////////////////////////////////////////////////
        // initialize a permute vector that, given input vectors:
        //
        //  X = x0 x1 x2 x3
        //  Y = y0 y1 y2 y3
        //
        // will generate
        //  Z = x0 x0 y3 y3
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vCosLoPerm = { 0x00010203, 0x00010203, 0x1C1D1E1F, 0x1C1D1E1F };

        ////////////////////////////////////////////////////////////////////////////////
        // initialize a select vector that, given input vectors:
        //
        //  X = x0 x1 x2 x3
        //  Y = y0 y1 y2 y3
        //
        // will generate
        //  Z = x0 x1 y2 y3
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vHiPairSelect = { 0, 0, 0xFFFFFFFF, 0xFFFFFFFF };

        if (isign == -1) {
            startSinMul = 1;
            vSinPerm = vForwardSinPerm.v;
        } else {
            startSinMul = -1;
            vSinPerm = vInverseSinPerm.v;
        }

        ////////////////////////////////////////////////////////////////////////////////
        // initialize a permute vector that, given input vectors:
        //
        //  X = x0 x1 x2 x3
        //  Y = y0 y1 y2 y3
        //
        // will generate
        //  Z = x0 x1 y0 y1
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vMergeHiPairPerm = { 0x00010203, 0x04050607, 0x10111213, 0x14151617 };

        ////////////////////////////////////////////////////////////////////////////////
        // initialize a permute vector that, given input vectors:
        //
        //  X = x0 x1 x2 x3
        //  Y = y0 y1 y2 y3
        //
        // will generate
        //  Z = x2 x3 y2 y3
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vMergeLoPairPerm = { 0x08090A0B, 0x0C0D0E0F, 0x18191A1B, 0x1C1D1E1F };

        ////////////////////////////////////////////////////////////////////////////////
        // start source pointers at start and end of high and low halves of the data
        ////////////////////////////////////////////////////////////////////////////////
        pInLoBottom = (__vector4 *)pData;
        pInHiBottom = pInLoBottom + len / 4;
        pInLoTop = pInHiBottom - 1;
        pInHiTop = pInLoBottom + (len / 2) - 1;

        ////////////////////////////////////////////////////////////////////////////////
        // start dest pointers at start and end of high and low halves of the data
        ////////////////////////////////////////////////////////////////////////////////
        pOutHiBottom = pInHiBottom;
        pOutHiTop = pInHiTop;
        pOutLoBottom = pInLoBottom;
        pOutLoTop = (pOutLoBottom + len / 4) - 1;

        ////////////////////////////////////////////////////////////////////////////////
        //
        //  Given c = cos(w) and s = sin(w), if we
        //  want to find the cos and sin of w plus
        //  some small angle d, then we can do so
        //  by defining:
        //
        //  a = 2 * ((sin(d/2)) ^ 2)
        //  b = sin(d)
        //
        //  Then, we can calculate the cos and sin
        //  of the updated angle w+d as:
        //
        //  cos(w+d) = c - ac - bs
        //  sin(w+d) = s - as + bc
        //
        //  We will need to incrementally calculate
        //  cos(w) and sin(w), so we define the
        //  following scalar values.  We use scalar
        //  because we need the precision of doubles,
        //  and vectors are floats.
        //
        ////////////////////////////////////////////////////////////////////////////////
        updateA = sinf(2 * PI / len);
        updateA *= updateA * 2;
        updateB = sinf(4 * PI / len);

        ////////////////////////////////////////////////////////////////////////////////
        //
        // We want to have four cos and sin vectors
        // that are updated incrementally, using
        // the doubles that we have used to calculate
        // our new values.  To do this we have a
        // "transition" vector that allows us to get
        // our scalar values into the vector domain.
        //
        // Scalar values are stored into the elements
        // of the transition vector, and then
        // our end cos and sin vectors are created
        // by permuting values out of our transition
        // vector and other previously created sin
        // and cos vectors.
        //
        ////////////////////////////////////////////////////////////////////////////////


        ////////////////////////////////////////////////////////////////////////////////
        // set up vSinLo as
        // vSinLo = -sin(0) sin(0) -sin(pi/len) sin(pi/len)
        ////////////////////////////////////////////////////////////////////////////////
        vFTransition.v[0] = 0;
        vFTransition.v[1] = 0;
        newSin1 = sinf(2 * PI / len);
        vFTransition.v[2] = newSin1 * startSinMul;
        vFTransition.v[3] = -newSin1 * startSinMul;
        vSinLo = vFTransition;

        ////////////////////////////////////////////////////////////////////////////////
        // set up vSinHi as
        // vSinHi = -sin((len-2)*pi/len) sin((len-2)*pi/len) -sin((len-1)*pi/len) sin((len-1)*pi/len)
        ////////////////////////////////////////////////////////////////////////////////
        newSin2 = sinf(4 * PI / len);
        vFTransition.v[0] = newSin2 * startSinMul;
        vFTransition.v[1] = -newSin2 * startSinMul;
        vSinHi = vFTransition;

        /////////////////////////////////////////////
        // set up vCosLo as
        // vCosLo = -cos(0) cos(0) -cos(pi/len) cos(pi/len)
        ////////////////////////////////////////////////////////////////////////////////
        vFTransition.v[0] = 1;
        vFTransition.v[1] = 1;
        newCos1 = cosf(2 * PI / len);
        vFTransition.v[2] = newCos1;
        vFTransition.v[3] = newCos1;
        vCosLo = vFTransition;

        ////////////////////////////////////////////////////////////////////////////////
        // set up vCosHi as
        // vCosHi = -cos((len-2)*pi/len) cos((len-2)*pi/len) -cos((len-1)*pi/len) cos((len-1)*pi/len)
        ////////////////////////////////////////////////////////////////////////////////
        newCos2 = cosf(4 * PI / len);
        vFTransition.v[0] = newCos2;
        vFTransition.v[1] = newCos2;
        vCosHi = __vsubfp(vZero, vFTransition);

        if (isign == -1) {
            ////////////////////////////////////////////////////////////////////////////////
            // we are doing a forward FFT
            ////////////////////////////////////////////////////////////////////////////////
            for (i = 0; i < len / 16; i++) {
                ////////////////////////////////////////////////////////////////////////////////
                // calculate new sin, cos for next time through loop, using our incremental
                // updating algorithm
                ////////////////////////////////////////////////////////////////////////////////
                tempCos1 = newCos1 - (updateA * newCos1 + updateB * newSin1);
                newSin1 = newSin1 - (updateA * newSin1 - updateB * newCos1);
                newCos1 = tempCos1;

                tempCos2 = newCos2 - (updateA * newCos2 + updateB * newSin2);
                newSin2 = newSin2 - (updateA * newSin2 - updateB * newCos2);
                newCos2 = tempCos2;

                ////////////////////////////////////////////////////////////////////////////////
                // load input vectors
                ////////////////////////////////////////////////////////////////////////////////
                vLoBottom = *pInLoBottom++;
                vHiBottom = *pInHiBottom++;
                vLoTop = *pInLoTop--;
                vHiTop = *pInHiTop--;

                ////////////////////////////////////////////////////////////////////////////////
                // store new cos & sin to transition vector
                ////////////////////////////////////////////////////////////////////////////////
                vFTransition.v[0] = newSin2;
                vFTransition.v[1] = newSin1;
                vFTransition.v[2] = newCos2;
                vFTransition.v[3] = newCos1;

                vFNegTransition = __vsubfp(vZero, vFTransition);

                ////////////////////////////////////////////////////////////////////////////////
                // calc sum of input data
                ////////////////////////////////////////////////////////////////////////////////
                *pOutLoBottom++ = __vaddfp(vLoBottom, vHiBottom);
                *pOutLoTop-- = __vaddfp(vLoTop, vHiTop);

                ////////////////////////////////////////////////////////////////////////////////
                // calc difference of input data
                ////////////////////////////////////////////////////////////////////////////////
                vDiffBottom = __vsubfp(vLoBottom, vHiBottom);
                vDiffTop = __vsubfp(vLoTop, vHiTop);

                ////////////////////////////////////////////////////////////////////////////////
                // multiply difference by twist factor
                ////////////////////////////////////////////////////////////////////////////////
                vTwistBottom = __vmaddfp(vCosLo, vDiffBottom, vZero);
                vTwistTop = __vmaddfp(vCosHi, vDiffTop, vZero);

                vSwappedDiffTop = __vperm(vDiffTop, vDiffTop, vSwapPairPerm.v);
                vSwappedDiffBottom = __vperm(vDiffBottom, vDiffBottom, vSwapPairPerm.v);

                vTwistBottom = __vmaddfp(vSinLo, vSwappedDiffBottom, vTwistBottom);
                vTwistTop = __vmaddfp(vSinHi, vSwappedDiffTop, vTwistTop);

                ////////////////////////////////////////////////////////////////////////////////
                // generate new cos & sin vectors from transition vector and previously
                // calculated steps.
                ////////////////////////////////////////////////////////////////////////////////
                vCosLo = __vsubfp(vZero, __vperm(vCosHi, vFNegTransition, vCosLoPerm.v));
                vCosHi = __vmrglw(vFNegTransition, vFNegTransition);

                vSinLo = __vperm(vFTransition, vFNegTransition, vSinPerm);
                vSinLo = __vsel(vSinHi, vSinLo, vHiPairSelect.v);
                vSinHi = __vperm(vFTransition, vFNegTransition, vSinPerm);

                ////////////////////////////////////////////////////////////////////////////////
                // load input vectors for next sum & difference calculation
                ////////////////////////////////////////////////////////////////////////////////
                vLoBottom = *pInLoBottom++;
                vHiBottom = *pInHiBottom++;
                vLoTop = *pInLoTop--;
                vHiTop = *pInHiTop--;

                ////////////////////////////////////////////////////////////////////////////////
                // store twist-multiplied difference.
                ////////////////////////////////////////////////////////////////////////////////
                *pOutHiBottom++ = vTwistBottom;
                *pOutHiTop-- = vTwistTop;

                ////////////////////////////////////////////////////////////////////////////////
                // calculate new sin, cos for next time through loop, using our incremental
                // updating algorithm
                ////////////////////////////////////////////////////////////////////////////////
                tempCos1 = newCos1 - (updateA * newCos1 + updateB * newSin1);
                newSin1 = newSin1 - (updateA * newSin1 - updateB * newCos1);
                newCos1 = tempCos1;

                tempCos2 = newCos2 - (updateA * newCos2 + updateB * newSin2);
                newSin2 = newSin2 - (updateA * newSin2 - updateB * newCos2);
                newCos2 = tempCos2;

                ////////////////////////////////////////////////////////////////////////////////
                // store new cos & sin to transition vector
                ////////////////////////////////////////////////////////////////////////////////
                vFTransition.v[0] = newSin2;
                vFTransition.v[1] = newSin1;
                vFTransition.v[2] = newCos2;
                vFTransition.v[3] = newCos1;

                vFNegTransition = __vsubfp(vZero, vFTransition);

                ////////////////////////////////////////////////////////////////////////////////
                // calc sum of input data
                ////////////////////////////////////////////////////////////////////////////////
                *pOutLoBottom++ = __vaddfp(vLoBottom, vHiBottom);
                *pOutLoTop-- = __vaddfp(vLoTop, vHiTop);

                ////////////////////////////////////////////////////////////////////////////////
                // calc difference of input data
                ////////////////////////////////////////////////////////////////////////////////
                vDiffBottom = __vsubfp(vLoBottom, vHiBottom);
                vDiffTop = __vsubfp(vLoTop, vHiTop);

                ////////////////////////////////////////////////////////////////////////////////
                // multiply difference by twist factor
                ////////////////////////////////////////////////////////////////////////////////
                vTwistBottom = __vmaddfp(vCosLo, vDiffBottom, vZero);
                vTwistTop = __vmaddfp(vCosHi, vDiffTop, vZero);

                vSwappedDiffTop = __vperm(vDiffTop, vDiffTop, vSwapPairPerm.v);
                vSwappedDiffBottom = __vperm(vDiffBottom, vDiffBottom, vSwapPairPerm.v);

                vTwistBottom = __vmaddfp(vSinLo, vSwappedDiffBottom, vTwistBottom);
                vTwistTop = __vmaddfp(vSinHi, vSwappedDiffTop, vTwistTop);

                ////////////////////////////////////////////////////////////////////////////////
                // generate new cos & sin vectors from transition vector and previously
                // calculated steps.
                ////////////////////////////////////////////////////////////////////////////////
                vCosLo = __vsubfp(vZero, __vperm(vCosHi, vFNegTransition, vCosLoPerm.v));
                vCosHi = __vmrglw(vFNegTransition, vFNegTransition);

                vSinLo = __vperm(vFTransition, vFNegTransition, vSinPerm);
                vSinLo = __vsel(vSinHi, vSinLo, vHiPairSelect.v);
                vSinHi = __vperm(vFTransition, vFNegTransition, vSinPerm);

                ////////////////////////////////////////////////////////////////////////////////
                // store twist-multiplied difference.
                ////////////////////////////////////////////////////////////////////////////////
                *pOutHiBottom++ = vTwistBottom;
                *pOutHiTop-- = vTwistTop;
            }
        } else {
            ////////////////////////////////////////////////////////////////////////////////
            // We are doing an inverse FFT, so we must adjust output by dividing by 2.
            ////////////////////////////////////////////////////////////////////////////////
            __vector4 vOneHalf = { 0.5f, 0.5f, 0.5f, 0.5f };
            for (i = 0; i < len / 16; i++) {
                ////////////////////////////////////////////////////////////////////////////////
                // calculate new sin, cos for next time through loop, using our incremental
                // updating algorithm
                ////////////////////////////////////////////////////////////////////////////////
                tempCos1 = newCos1 - (updateA * newCos1 + updateB * newSin1);
                newSin1 = newSin1 - (updateA * newSin1 - updateB * newCos1);
                newCos1 = tempCos1;

                tempCos2 = newCos2 - (updateA * newCos2 + updateB * newSin2);
                newSin2 = newSin2 - (updateA * newSin2 - updateB * newCos2);
                newCos2 = tempCos2;

                ////////////////////////////////////////////////////////////////////////////////
                // load input vectors
                ////////////////////////////////////////////////////////////////////////////////
                vLoBottom = *pInLoBottom++;
                vHiBottom = *pInHiBottom++;
                vLoTop = *pInLoTop--;
                vHiTop = *pInHiTop--;

                ////////////////////////////////////////////////////////////////////////////////
                // do inverse adjusting by dividing all elements by two.
                ////////////////////////////////////////////////////////////////////////////////
                vLoBottom = __vmaddfp(vLoBottom, vOneHalf, vZero);
                vHiBottom = __vmaddfp(vHiBottom, vOneHalf, vZero);
                vLoTop = __vmaddfp(vLoTop, vOneHalf, vZero);
                vHiTop = __vmaddfp(vHiTop, vOneHalf, vZero);

                ////////////////////////////////////////////////////////////////////////////////
                // store new cos & sin to transition vector
                ////////////////////////////////////////////////////////////////////////////////
                vFTransition.v[0] = newSin2;
                vFTransition.v[1] = newSin1;
                vFTransition.v[2] = newCos2;
                vFTransition.v[3] = newCos1;

                vFNegTransition = __vsubfp(vZero, vFTransition);

                ////////////////////////////////////////////////////////////////////////////////
                // calc sum of input data
                ////////////////////////////////////////////////////////////////////////////////
                *pOutLoBottom++ = __vaddfp(vLoBottom, vHiBottom);
                *pOutLoTop-- = __vaddfp(vLoTop, vHiTop);

                ////////////////////////////////////////////////////////////////////////////////
                // calc difference of input data
                ////////////////////////////////////////////////////////////////////////////////
                vDiffBottom = __vsubfp(vLoBottom, vHiBottom);
                vDiffTop = __vsubfp(vLoTop, vHiTop);

                ////////////////////////////////////////////////////////////////////////////////
                // multiply difference by twist factor
                ////////////////////////////////////////////////////////////////////////////////
                vTwistBottom = __vmaddfp(vCosLo, vDiffBottom, vZero);
                vTwistTop = __vmaddfp(vCosHi, vDiffTop, vZero);

                vSwappedDiffTop = __vperm(vDiffTop, vDiffTop, vSwapPairPerm.v);
                vSwappedDiffBottom = __vperm(vDiffBottom, vDiffBottom, vSwapPairPerm.v);

                vTwistBottom = __vmaddfp(vSinLo, vSwappedDiffBottom, vTwistBottom);
                vTwistTop = __vmaddfp(vSinHi, vSwappedDiffTop, vTwistTop);

                ////////////////////////////////////////////////////////////////////////////////
                // generate new cos & sin vectors from transition vector and previously
                // calculated steps.
                ////////////////////////////////////////////////////////////////////////////////
                vCosLo = __vsubfp(vZero, __vperm(vCosHi, vFNegTransition, vCosLoPerm.v));
                vCosHi = __vmrglw(vFNegTransition, vFNegTransition);

                vSinLo = __vperm(vFTransition, vFNegTransition, vSinPerm);
                vSinLo = __vsel(vSinHi, vSinLo, vHiPairSelect.v);
                vSinHi = __vperm(vFTransition, vFNegTransition, vSinPerm);

                ////////////////////////////////////////////////////////////////////////////////
                // store twist-multiplied difference.
                ////////////////////////////////////////////////////////////////////////////////
                *pOutHiBottom++ = vTwistBottom;
                *pOutHiTop-- = vTwistTop;

                ////////////////////////////////////////////////////////////////////////////////
                // calculate new sin, cos for next time through loop, using our incremental
                // updating algorithm
                ////////////////////////////////////////////////////////////////////////////////
                tempCos1 = newCos1 - (updateA * newCos1 + updateB * newSin1);
                newSin1 = newSin1 - (updateA * newSin1 - updateB * newCos1);
                newCos1 = tempCos1;

                tempCos2 = newCos2 - (updateA * newCos2 + updateB * newSin2);
                newSin2 = newSin2 - (updateA * newSin2 - updateB * newCos2);
                newCos2 = tempCos2;

                ////////////////////////////////////////////////////////////////////////////////
                // load input vectors
                ////////////////////////////////////////////////////////////////////////////////
                vLoBottom = *pInLoBottom++;
                vHiBottom = *pInHiBottom++;
                vLoTop = *pInLoTop--;
                vHiTop = *pInHiTop--;

                ////////////////////////////////////////////////////////////////////////////////
                // do inverse adjusting by dividing all elements by two.
                ////////////////////////////////////////////////////////////////////////////////
                vLoBottom = __vmaddfp(vLoBottom, vOneHalf, vZero);
                vHiBottom = __vmaddfp(vHiBottom, vOneHalf, vZero);
                vLoTop = __vmaddfp(vLoTop, vOneHalf, vZero);
                vHiTop = __vmaddfp(vHiTop, vOneHalf, vZero);

                ////////////////////////////////////////////////////////////////////////////////
                // store new cos & sin to transition vector
                ////////////////////////////////////////////////////////////////////////////////
                vFTransition.v[0] = newSin2;
                vFTransition.v[1] = newSin1;
                vFTransition.v[2] = newCos2;
                vFTransition.v[3] = newCos1;

                vFNegTransition = __vsubfp(vZero, vFTransition);

                ////////////////////////////////////////////////////////////////////////////////
                // calc sum of input data
                ////////////////////////////////////////////////////////////////////////////////
                *pOutLoBottom++ = __vaddfp(vLoBottom, vHiBottom);
                *pOutLoTop-- = __vaddfp(vLoTop, vHiTop);

                ////////////////////////////////////////////////////////////////////////////////
                // calc difference of input data
                ////////////////////////////////////////////////////////////////////////////////
                vDiffBottom = __vsubfp(vLoBottom, vHiBottom);
                vDiffTop = __vsubfp(vLoTop, vHiTop);

                ////////////////////////////////////////////////////////////////////////////////
                // multiply difference by twist factor
                ////////////////////////////////////////////////////////////////////////////////
                vTwistBottom = __vmaddfp(vCosLo, vDiffBottom, vZero);
                vTwistTop = __vmaddfp(vCosHi, vDiffTop, vZero);

                vSwappedDiffTop = __vperm(vDiffTop, vDiffTop, vSwapPairPerm.v);
                vSwappedDiffBottom = __vperm(vDiffBottom, vDiffBottom, vSwapPairPerm.v);

                vTwistBottom = __vmaddfp(vSinLo, vSwappedDiffBottom, vTwistBottom);
                vTwistTop = __vmaddfp(vSinHi, vSwappedDiffTop, vTwistTop);

                ////////////////////////////////////////////////////////////////////////////////
                // generate new cos & sin vectors from transition vector and previously
                // calculated steps.
                ////////////////////////////////////////////////////////////////////////////////
                vCosLo = __vsubfp(vZero, __vperm(vCosHi, vFNegTransition, vCosLoPerm.v));
                vCosHi = __vmrglw(vFNegTransition, vFNegTransition);

                vSinLo = __vperm(vFTransition, vFNegTransition, vSinPerm);
                vSinLo = __vsel(vSinHi, vSinLo, vHiPairSelect.v);
                vSinHi = __vperm(vFTransition, vFNegTransition, vSinPerm);

                ////////////////////////////////////////////////////////////////////////////////
                // store twist-multiplied difference.
                ////////////////////////////////////////////////////////////////////////////////
                *pOutHiBottom++ = vTwistBottom;
                *pOutHiTop-- = vTwistTop;
            }
        }

        ////////////////////////////////////////////////////////////////////////////////
        // perform N/2-length FFT on high half of data
        ////////////////////////////////////////////////////////////////////////////////
        result = FFTComplex(pData + len, len / 2, isign, sinCosTable);
        if (result == 0) {
            ////////////////////////////////////////////////////////////////////////////////
            // perform N/2-length FFT on low half of data
            ////////////////////////////////////////////////////////////////////////////////
            result = FFTComplex(pData, len / 2, isign, sinCosTable);
            if (result == 0) {
                ////////////////////////////////////////////////////////////////////////////////
                // Move low half of data to temporary buffer, which will leave room in
                // main data space to merge elements of low and high data in to the original
                // buffer space.
                ////////////////////////////////////////////////////////////////////////////////
                {
                    __vector4 *pSource = (__vector4 *)pData;
                    __vector4 *pDest = (__vector4 *)gTempBuffer.mBuffer;
                    __vector4 v1, v2, v3, v4;
                    for (i = 0; i < len / 16; i++) {
                        v1 = *pSource++;
                        v2 = *pSource++;
                        v3 = *pSource++;
                        v4 = *pSource++;
                        *pDest++ = v1;
                        *pDest++ = v2;
                        *pDest++ = v3;
                        *pDest++ = v4;
                    }
                }

                ////////////////////////////////////////////////////////////////////////////////
                // Merge low and high halves of data back into original data buffer.  Elements
                // are merged so that, given high and low signals X1 and X2, resulting merged
                // signal is X1(0), X2(0), X1(1), X2(1), ... , X1(N/2-1), X2(N/2-1)
                ////////////////////////////////////////////////////////////////////////////////
                pInHiBottom = (__vector4 *)gTempBuffer.mBuffer;
                pInHiTop = (__vector4 *)(pData + len);
                pOutLoBottom = (__vector4 *)pData;

                for (i = 0; i < len / 8; i++) {
                    __vector4 vInEven1, vInOdd1;
                    __vector4 vInEven2, vInOdd2;
                    __vector4 vOut1, vOut2, vOut3, vOut4;

                    vInEven1 = *pInHiBottom++;
                    vInOdd1 = *pInHiTop++;
                    vInEven2 = *pInHiBottom++;
                    vInOdd2 = *pInHiTop++;

                    vOut1 = __vperm(vInEven1, vInOdd1, vMergeHiPairPerm.v);
                    vOut2 = __vperm(vInEven1, vInOdd1, vMergeLoPairPerm.v);
                    vOut3 = __vperm(vInEven2, vInOdd2, vMergeHiPairPerm.v);
                    vOut4 = __vperm(vInEven2, vInOdd2, vMergeLoPairPerm.v);

                    *pOutLoBottom++ = vOut1;
                    *pOutLoBottom++ = vOut2;
                    *pOutLoBottom++ = vOut3;
                    *pOutLoBottom++ = vOut4;
                }
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
//  FFTComplex
//
//  Performs a forward or inverse complex FFT on the data.  This is a wrapper
// function for three different FFT routines.  If the length is below the
// breakover to call the recursive FFT, then it calls the pingpong FFT.  If
// it's above the point at which the recursive FFT is faster, then it calls
// one of the recursive FFTs.  If this is the case, then it chooses between
// two forms of recursion.  For even powers of two, it calls the matrix
// FFT (which can only be performed on even powers of two because it allows
// for a square matrix, which can be easily transposed).  If the length is
// an odd power of two, then a one-step recursive FFT is called.
////////////////////////////////////////////////////////////////////////////////
int FFTComplex(float *data, long length, long isign, float *sinCosTable) {
    int result;
    if (length <= 0x8000) {
        ////////////////////////////////////////////////////////////////////////////////
        // length is in the range where pingpong FFT is fastest
        ////////////////////////////////////////////////////////////////////////////////
        result = fft_pingpong(data, length, isign, sinCosTable);
    } else {
        long pow = log2max(length);
        if (!(pow & 1)) {
            ////////////////////////////////////////////////////////////////////////////////
            // data can be represented in a square matrix, so call matrix FFT
            ////////////////////////////////////////////////////////////////////////////////
            result = fft_square_matrix(data, length, isign, sinCosTable);
        } else {
            ////////////////////////////////////////////////////////////////////////////////
            // data length is odd power of two, so call recursive FFT
            ////////////////////////////////////////////////////////////////////////////////
            result = fft_recursive(data, length, isign, sinCosTable);
        }
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////
//  fft_real_forward_altivec
//
//  Given a real signal X = x_0 ... x_(n-1), one can calculate a real-signal
//  fft as follows:
//
//  First, the real signal is treated as complex data (of length n/2), and
//  a complex FFT is performed on X to yield complex data U, with
//  U = u_0...u_(n/2-1).
//
//  Next, the signal U is used to define e_k and o_k (k in [0, n/2],
//  with e_n/2 = e_0), where
//
//  e_k = (u_k + u_(n/2 - k)*) / 2
//
//  o_k = (u_k + u_(n/2 - k)*) / 2i
//
//  Given e_k and o_k, we define Y as
//
//  Y_k = e_k + ( e^(-2*pi*i*k/N) * o_k )
//
//  for k in [0, n/2].  Then, the signal Y is the real-signal FFT.
//
//  Result is stored in hermitian order, so that it may occupy the exact same
//  space as the original data.  Given the complex-signal FFT result Y,
//  this result is stored in the order:
//
//  Y(0)r Y(N/2)r Y(1)r Y(1)i Y(2)r Y(2)i ... Y(N/2-1)r Y(N/2-1)i
////////////////////////////////////////////////////////////////////////////////
int fft_real_forward_altivec(float *data, long length, float *sinCosTable) {
    int result = 0;
    __vector4 *pInVecLo, *pInVecHi;
    __vector4 *pOutVecLo, *pOutVecHi;
    __vector4 vInLoNext, vInHiPrev;
    __vector4 vUHi1, vUHi2, vULo1, vULo2;
    __vector4 vDiffLo, vDiffHi;
    __vector4 vSumLo, vSumHi;
    __vector4 vFirstMul;
    __vector4 vSecondMul;
    __vector4 vSinLo;
    __vector4 vCosLo;
    __vector4 vResultLo, vResultHi;
    __vector4 vSinHi;
    __vector4 vCosHi;
    long i;
    __vector4 vFTransition;
    double updateA;
    double updateB;
    double newCos1;
    double newCos2;
    double newSin1;
    double newSin2;
    double tempCos1;
    double tempCos2;
    float real0;
    float im0;

    ////////////////////////////////////////////////////////////////////////////////
    // perform a forward complex fft on our real signal data, treating it as a
    // half-length complex signal.
    ////////////////////////////////////////////////////////////////////////////////
    result = FFTComplex(data, length / 2, -1, sinCosTable);

    if (result == 0) {
        ////////////////////////////////////////////////////////////////////////////////
        // initialize zero vector
        ////////////////////////////////////////////////////////////////////////////////
        __vector4 vZero = { 0, 0, 0, 0 };

        ////////////////////////////////////////////////////////////////////////////////
        // create a select vector that will select the first two elements of vector
        // one, and second two elements of vector two
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vHiLoSelect = { 0xFFFFFFFF, 0xFFFFFFFF, 0, 0 };

        ////////////////////////////////////////////////////////////////////////////////
        // create a permute vector that, given input vectors
        //
        // X = x0 x1 x2 x3
        // Y = y0 y1 y2 y3
        //
        // will create the output vector
        // Z = x0 x0 y3 y3
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vNewSinLoPerm = { 0x00010203, 0x00010203, 0x1C1D1E1F, 0x1C1D1E1F };

        ////////////////////////////////////////////////////////////////////////////////
        // create a permute vector that, given input vectors
        //
        // X = x0 x1 x2 x3
        // Y = y0 y1 y2 y3
        //
        // will create the output vector
        // Z = x0 x0 y1 y1
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vNewCosLoPerm = { 0x04050607, 0x04050607, 0x14151617, 0x14151617 };

        ////////////////////////////////////////////////////////////////////////////////
        // create a permute vector that, given input vectors
        //
        // X = x0 x1 x2 x3
        // Y = y0 y1 y2 y3
        //
        // will create the output vector
        // Z = x0 y1 x2 y3
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vAdderPerm = { 0x00010203, 0x14151617, 0x08090A0B, 0x1C1D1E1F };

        ////////////////////////////////////////////////////////////////////////////////
        // create a permute vector that, given input vectors
        //
        // X = x0 x1 x2 x3
        // Y = y0 y1 y2 y3
        //
        // will create the output vector
        // Z = x1 y0 x3 y2
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vFirstMulPerm = { 0x04050607, 0x10111213, 0x0C0D0E0F, 0x18191A1B };

        ////////////////////////////////////////////////////////////////////////////////
        // create a permute vector that, given input vectors
        //
        // X = x0 x1 x2 x3
        // Y = y0 y1 y2 y3
        //
        // will create the output vector
        // Z = y0 x1 y2 x3
        ////////////////////////////////////////////////////////////////////////////////
        XMVECTORU32 vSecondMulPerm = { 0x10111213, 0x04050607, 0x18191A1B, 0x0C0D0E0F };

        ////////////////////////////////////////////////////////////////////////////////
        // create float vector of 0.5
        ////////////////////////////////////////////////////////////////////////////////
        __vector4 vOneHalf = { 0.5f, 0.5f, 0.5f, 0.5f };

        ////////////////////////////////////////////////////////////////////////////////
        // create a multiply vector that will negate second and fourth elements
        ////////////////////////////////////////////////////////////////////////////////
        __vector4 vAlternateNegate = { 1, -1, 1, -1 };

        ////////////////////////////////////////////////////////////////////////////////
        // create a multiply vector that will negate first and third elements
        ////////////////////////////////////////////////////////////////////////////////
        __vector4 vAlternateNegate2 = { -1, 1, -1, 1 };

        ////////////////////////////////////////////////////////////////////////////////
        //
        //  Given c = cos(w) and s = sin(w), if we
        //  want to find the cos and sin of w plus
        //  some small angle d, then we can do so
        //  by defining:
        //
        //  a = 2 * ((sin(d/2)) ^ 2)
        //  b = sin(d)
        //
        //  Then, we can calculate the cos and sin
        //  of the updated angle w+d as:
        //
        //  cos(w+d) = c - ac - bs
        //  sin(w+d) = s - as + bc
        //
        //  We will need to incrementally calculate
        //  cos(w) and sin(w), so we define the
        //  following scalar values.  We use scalar
        //  because we need the precision of doubles,
        //  and vectors are floats.
        //
        ////////////////////////////////////////////////////////////////////////////////
        updateA = sinf(2 * PI / length);
        updateA *= updateA * 2;
        updateB = sinf(4 * PI / length);

        ////////////////////////////////////////////////////////////////////////////////
        //
        // We want to have four cos and sin vectors that are updated incrementally, using
        // the doubles that we have used to calculate our new values.  To do this we
        // have a "transition" vector that allows us to get our scalar values into the
        // vector domain.
        //
        // Scalar values are stored into the elements of the transition vector, and then
        // our end cos and sin vectors are created by permuting values out of our
        // transition vector and other previously created sin and cos vectors.
        //
        // We also take advantage of the fact that
        // sin(pi - d) = sin(0 + d)
        // and
        // cos(pi - d) = -cos(0 + d)
        //
        ////////////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////////////
        // We store values into vFTransiton so that we can create:
        //
        // vCosLo = cos(0) cos(0) cos(2pi/length) cos(2pi/length)
        //
        ////////////////////////////////////////////////////////////////////////////////
        vFTransition.v[0] = 1;
        vFTransition.v[1] = 1;
        newCos1 = cosf(2 * PI / length);
        vFTransition.v[2] = newCos1;
        vFTransition.v[3] = newCos1;
        vCosLo = vFTransition;

        ////////////////////////////////////////////////////////////////////////////////
        // We store values into vFTransiton so that we can create:
        //
        // vCosHi = cos(2*2pi/length) cos(2*2pi/length) cos(2pi/length) cos(2pi/length)
        //
        ////////////////////////////////////////////////////////////////////////////////
        newCos2 = cosf(4 * PI / length);
        vFTransition.v[0] = newCos2;
        vFTransition.v[1] = newCos2;
        vCosHi = vFTransition;

        ////////////////////////////////////////////////////////////////////////////////
        // We store values into vFTransiton so that we can create:
        //
        // vSinLo = sin(0) sin(0) sin(2pi/length) sin(2pi/length)
        //
        ////////////////////////////////////////////////////////////////////////////////
        vFTransition.v[0] = 0;
        vFTransition.v[1] = 0;
        newSin1 = sinf(2 * PI / length);
        vFTransition.v[2] = newSin1;
        vFTransition.v[3] = newSin1;
        vSinLo = vFTransition;

        ////////////////////////////////////////////////////////////////////////////////
        // We store values into vFTransiton so that we can create:
        //
        // vSinHi = sin(2*2pi/length) sin(2*2pi/length) sin(2pi/length) sin(2pi/length)
        //
        ////////////////////////////////////////////////////////////////////////////////
        newSin2 = sinf(4 * PI / length);
        vFTransition.v[0] = newSin2;
        vFTransition.v[1] = newSin2;
        vSinHi = vFTransition;

        ////////////////////////////////////////////////////////////////////////////////
        // save real[0] and im[0] for later use
        ////////////////////////////////////////////////////////////////////////////////
        real0 = data[0];
        im0 = data[1];

        ////////////////////////////////////////////////////////////////////////////////
        // set up hi,lo pointers for input from data to point at first and last vectors,
        // and do the same for output vectors, since we overwrite our results in place.
        ////////////////////////////////////////////////////////////////////////////////
        pInVecLo = (__vector4 *)data;
        pInVecHi = pInVecLo + (length / 4) - 1;
        pOutVecLo = pInVecLo;
        pOutVecHi = pInVecHi;

        ////////////////////////////////////////////////////////////////////////////////
        // get 0 element for wraparound, since U_{n/2} = U_0.
        ////////////////////////////////////////////////////////////////////////////////
        vInLoNext = *pInVecLo++;
        vInHiPrev = vInLoNext;

        ////////////////////////////////////////////////////////////////////////////////
        // Loop through all elements.  Since each vector is four elements, and we are
        // calculating two vectors per iteration through the loop, we calculate 8
        // elements per loop, and so we iterate through the loop (length/8) times.
        ////////////////////////////////////////////////////////////////////////////////
        for (i = 0; i < length / 8; i++) {
            ////////////////////////////////////////////////////////////////////////////////
            // negate alternating elements of vCosLo and vCosHi vectors.
            ////////////////////////////////////////////////////////////////////////////////
            vCosLo = __vmaddfp(vCosLo, vAlternateNegate, vZero);
            vCosHi = __vmaddfp(vCosHi, vAlternateNegate2, vZero);

            ////////////////////////////////////////////////////////////////////////////////
            // calculate new sin, cos for next time through loop, using our incremental
            // updating algorithm
            ////////////////////////////////////////////////////////////////////////////////
            tempCos1 = newCos1 - (updateA * newCos1 + updateB * newSin1);
            newSin1 = newSin1 - (updateA * newSin1 - updateB * newCos1);
            newCos1 = tempCos1;

            tempCos2 = newCos2 - (updateA * newCos2 + updateB * newSin2);
            newSin2 = newSin2 - (updateA * newSin2 - updateB * newCos2);
            newCos2 = tempCos2;

            ////////////////////////////////////////////////////////////////////////////////
            // Using vectors we load in, and vectors from previous time through loop, we
            // generate the vectors:
            //
            // vULo1 =  re[k]       im[k]       re[k+1]     im[k+1]
            // vULo2 =  re[n/2-k]   im[n/2-k]   re[n/2-k-1] im[n/2-k-1]
            //
            // vUHi1 =  re[n/2-k-1] im[n/2-k-1] re[n/2-k-2] im[n/2-k-2]
            // vUHi2 =  re[k+1]     im[k+1]     re[k+2]     im[k+2]
            //
            // with k = 2*i (where i is the loop iterator)
            ////////////////////////////////////////////////////////////////////////////////
            vULo1 = vInLoNext;
            vUHi1 = *pInVecHi--;
            vULo2 = __vsel(vUHi1, vInHiPrev, vHiLoSelect.v);
            vInHiPrev = vUHi1;

            vInLoNext = *pInVecLo++;
            vUHi2 = __vsel(vULo1, vInLoNext, vHiLoSelect.v);

            ////////////////////////////////////////////////////////////////////////////////
            //  Calculate sum and difference of vULo1 and vULo2, which are:
            //
            // vSumLo = re[k]+re[n/2-k], im[k]+im[n/2-k], re[k+1]+re[n/2-k-1], im[k+1]+im[n/2-k-1]
            // vDiffLo = re[k]-re[n/2-k], im[k]-im[n/2-k], re[k+1]-re[n/2-k-1], im[k+1]-im[n/2-k-1]
            ////////////////////////////////////////////////////////////////////////////////
            vSumLo = __vaddfp(vULo1, vULo2);
            vDiffLo = __vsubfp(vULo1, vULo2);

            ////////////////////////////////////////////////////////////////////////////////
            //  We start by generating a result vector that is:
            //
            // vResultLo = re[k]+re[n/2-k]   im[k]-im[n/2-k]   re[k+1]+re[n/2-k-1]   im[k+1]-im[n/2-k-1]
            ////////////////////////////////////////////////////////////////////////////////
            vResultLo = __vperm(vSumLo, vDiffLo, vAdderPerm.v);

            ////////////////////////////////////////////////////////////////////////////////
            // Next we create a multiplier vector that is:
            //
            // vFirstMul = im[k]+im[n/2-k]   re[k]-re[n/2-k]    im[k+1]+im[n/2-k-1]   re[k+1]-re[n/2-k-1]
            ////////////////////////////////////////////////////////////////////////////////
            vFirstMul = __vperm(vSumLo, vDiffLo, vFirstMulPerm.v);

            ////////////////////////////////////////////////////////////////////////////////
            // Next we create a multiplier vector that is:
            //
            // vSecondMul = re[k]-re[n/2-k]   im[k]+im[n/2-k]   re[k+1]-re[n/2-k-1]   im[k+1]+im[n/2-k-1]
            ////////////////////////////////////////////////////////////////////////////////
            vSecondMul = __vperm(vSumLo, vDiffLo, vSecondMulPerm.v);

            ////////////////////////////////////////////////////////////////////////////////
            // We now calculate:
            //
            // vResultLo =
            //
            //      re[k]+re[n/2-k] + (im[k]+im[n/2-k]) * cos(k*2pi/length),
            //      im[k]-im[n/2-k] + (re[k]-re[n/2-k]) * -cos(k*2pi/length),
            //      re[k+1]+re[n/2-k-1] + (im[k+1]+im[n/2-k-1]) * cos((k+1)*2pi/length),
            //      im[k+1]-im[n/2-k-1] + (re[k+1]-re[n/2-k-1]) * -cos((k+1)*2pi/length)
            //
            ////////////////////////////////////////////////////////////////////////////////
            vResultLo = __vmaddfp(vCosLo, vFirstMul, vResultLo);

            ////////////////////////////////////////////////////////////////////////////////
            // with a negative multiply-subtract, we calculate
            //
            // vResultLo =
            //
            //      re[k]+re[n/2-k]     + (im[k]+im[n/2-k]) * cos(k*2pi/length)             + (re[k]-re[n/2-k]) * -sin(k*2pi/length),
            //      im[k]-im[n/2-k]     + (re[k]-re[n/2-k]) * -cos(k*2pi/length)            + (im[k]+im[n/2-k]) * -sin(k*2pi/length),
            //      re[k+1]+re[n/2-k-1] + (im[k+1]+im[n/2-k-1]) * cos((k+1)*2pi/length)     + (re[k+1]-re[n/2-k-1]) * -sin((k+1)*2pi/length),
            //      im[k+1]-im[n/2-k-1] + (re[k+1]-re[n/2-k-1]) * -cos((k+1)*2pi/length)    + (im[k+1]+im[n/2-k-1]) * -sin((k+1)*2pi/length)
            //
            ////////////////////////////////////////////////////////////////////////////////
            vResultLo = __vnmsubfp(vSinLo, vSecondMul, vResultLo);

            ////////////////////////////////////////////////////////////////////////////////
            // finally, divide all elements by two (multiply by one half).  With this
            // step finished, we have calculated
            // e_k + (e^(-2pi*i*k/N) * o_k), e_(k+1) + (e^(-2pi*i*(k+1)/N) * o_(k+1))
            ////////////////////////////////////////////////////////////////////////////////
            vResultLo = __vmaddfp(vResultLo, vOneHalf, vZero);

            ////////////////////////////////////////////////////////////////////////////////
            // store this vector back, overwriting source data.
            ////////////////////////////////////////////////////////////////////////////////
            *pOutVecLo++ = vResultLo;

            ////////////////////////////////////////////////////////////////////////////////
            // store our new angles to the transition vector, and generate new cos,sin
            // vectors from them.
            ////////////////////////////////////////////////////////////////////////////////
            vFTransition.v[0] = newCos2;
            vFTransition.v[1] = newCos1;
            vFTransition.v[2] = newSin2;
            vFTransition.v[3] = newSin1;

            vCosLo = __vperm(vCosHi, vFTransition, vNewCosLoPerm.v);
            vSinLo = __vperm(vSinHi, vFTransition, vNewSinLoPerm.v);

            ////////////////////////////////////////////////////////////////////////////////
            // Calculate
            // e_k + (e^(-2pi*i*k/N) * o_k), e_(k+1) + (e^(-2pi*i*(k+1)/N) * o_(k+1))
            // for high elements in array
            ////////////////////////////////////////////////////////////////////////////////
            vSumHi = __vaddfp(vUHi1, vUHi2);
            vDiffHi = __vsubfp(vUHi1, vUHi2);

            vFirstMul = __vperm(vSumHi, vDiffHi, vFirstMulPerm.v);
            vSecondMul = __vperm(vSumHi, vDiffHi, vSecondMulPerm.v);
            vResultHi = __vperm(vSumHi, vDiffHi, vAdderPerm.v);

            vResultHi = __vmaddfp(vCosHi, vFirstMul, vResultHi);
            vResultHi = __vnmsubfp(vSinHi, vSecondMul, vResultHi);
            vResultHi = __vmaddfp(vResultHi, vOneHalf, vZero);

            ////////////////////////////////////////////////////////////////////////////////
            // create new angle vectors from our transition vectors for next time
            // through loop
            ////////////////////////////////////////////////////////////////////////////////
            vCosHi = __vmrghw(vFTransition, vFTransition);
            vSinHi = __vmrglw(vFTransition, vFTransition);

            ////////////////////////////////////////////////////////////////////////////////
            // store high result, overwriting source
            ////////////////////////////////////////////////////////////////////////////////
            *pOutVecHi-- = vResultHi;
        }

        ////////////////////////////////////////////////////////////////////////////////
        // finally, store re[n/2]
        ////////////////////////////////////////////////////////////////////////////////
        data[1] = real0 - im0;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
// scalar implementation of altivec forward real fft above
////////////////////////////////////////////////////////////////////////////////
int fft_real_forward_scalar(float *data, unsigned long len, float *sinCosTable) {
    int result = 0;
    double updateA;
    double updateB;
    double currentCos;
    double currentSin;
    double tempCos1;
    float realLo, realHi;
    float imLo, imHi;
    float realResultLo, realResultHi;
    float imResultLo, imResultHi;
    float realDiff, imDiff;
    float realSum, imSum;
    float *pInLo, *pInHi;
    long i;

    ////////////////////////////////////////////////////////////////////////////////
    // for length of 0 or 1 we do nothing
    ////////////////////////////////////////////////////////////////////////////////
    if (len < 2)
        return 0;

    ////////////////////////////////////////////////////////////////////////////////
    // perform a forward complex fft on our
    // real signal data, treating it as a
    // half-length complex signal.
    ////////////////////////////////////////////////////////////////////////////////
    result = FFTComplex(data, len / 2, -1, sinCosTable);

    if (result == 0) {
        ////////////////////////////////////////////////////////////////////////////////
        //
        //  Given c = cos(w) and s = sin(w), if we
        //  want to find the cos and sin of w plus
        //  some small angle d, then we can do so
        //  by defining:
        //
        //  a = 2 * ((sin(d/2)) ^ 2)
        //  b = sin(d)
        //
        //  Then, we can calculate the cos and sin
        //  of the updated angle w+d as:
        //
        //  cos(w+d) = c - ac - bs
        //  sin(w+d) = s - as + bc
        //
        //  We will need to incrementally calculate
        //  cos(w) and sin(w), so we define the
        //  following scalar values.  We use scalar
        //  because we need the precision of doubles,
        //  and vectors are floats.
        //
        ////////////////////////////////////////////////////////////////////////////////
        updateA = sinf(PI / len);
        updateB = sinf(2 * PI / len);
        updateA *= updateA * 2;

        ////////////////////////////////////////////////////////////////////////////////
        // start with sin(0) and cos(0)
        ////////////////////////////////////////////////////////////////////////////////
        currentCos = 1;
        currentSin = 0;

        ////////////////////////////////////////////////////////////////////////////////
        // first, calculate re[0], and re[n/2]. we don't bother to do the sin, cos
        // multiplies, because we know that they are zero and one.
        ////////////////////////////////////////////////////////////////////////////////
        realResultLo = data[0] + data[1];
        realResultHi = data[0] - data[1];

        ////////////////////////////////////////////////////////////////////////////////
        // store re[0].  We don't calculate im[0] because we know that it is 0.
        ////////////////////////////////////////////////////////////////////////////////
        data[0] = realResultLo;

        ////////////////////////////////////////////////////////////////////////////////
        // store re[n/2].  We don't calculate im[n/2] because we know it is 0.  We store
        // re[n/2] in the position of im[0] because we don't want to take up any more
        // space than the source data, and if we were to store it in the n/2 position,
        // it would be past the array of source data, which ranges from 0 to (n/2)-1.
        ////////////////////////////////////////////////////////////////////////////////
        data[1] = realResultHi;

        ////////////////////////////////////////////////////////////////////////////////
        // Start our source pointers to point at re[1] and re[(n/2)-1].
        ////////////////////////////////////////////////////////////////////////////////
        pInLo = &data[2];
        pInHi = &data[len - 2];

        ////////////////////////////////////////////////////////////////////////////////
        // for each loop iteration, we calculate two complex results (four floats), so
        // we iterate through the loop len/4 times
        ////////////////////////////////////////////////////////////////////////////////
        for (i = 0; i < len / 4; i++) {
            ////////////////////////////////////////////////////////////////////////////////
            // calculate the new sin and cos values
            ////////////////////////////////////////////////////////////////////////////////
            tempCos1 = currentCos - (updateA * currentCos + updateB * currentSin);
            currentSin = currentSin - (updateA * currentSin - updateB * currentCos);
            currentCos = tempCos1;

            ////////////////////////////////////////////////////////////////////////////////
            // load re[i+1], im[i+1]
            ////////////////////////////////////////////////////////////////////////////////
            realLo = *pInLo;
            imLo = *(pInLo + 1);

            ////////////////////////////////////////////////////////////////////////////////
            // load re[(n/2)-1-i], im[(n/2)-1-i]
            ////////////////////////////////////////////////////////////////////////////////
            realHi = *pInHi;
            imHi = *(pInHi + 1);

            ////////////////////////////////////////////////////////////////////////////////
            // calculate:
            // realDiff = re[i+1]-re[(n/2)-1-i]
            // realSum  = re[i+1]+re[(n/2)-1-i]
            ////////////////////////////////////////////////////////////////////////////////
            realDiff = realLo - realHi;
            realSum = realLo + realHi;

            ////////////////////////////////////////////////////////////////////////////////
            // calculate:
            // imDiff   = im[i+1]-im[(n/2)-1-i]
            // imSum    = im[i+1]+im[(n/2)-1-i]
            ////////////////////////////////////////////////////////////////////////////////
            imDiff = imLo - imHi;
            imSum = imLo + imHi;

            ////////////////////////////////////////////////////////////////////////////////
            // calculate:
            //
            // realResultLo =
            //      re[i+1]+re[(n/2)-1-i] +
            //      (im[i+1]+im[(n/2)-1-i]) * cos( (i+1) * 2pi / n ) -
            //      (re[i+1]-re[(n/2)-1-i]) * sin( (i+1) * 2pi / n )
            //
            // imResultLo =
            //      im[i+1]-im[(n/2)-1-i] -
            //      (re[i+1]-re[(n/2)-1-i]) * cos( (i+1) * 2pi / n ) -
            //      (im[i+1]+im[(n/2)-1-i]) * sin( (i+1) * 2pi / n )
            //
            ////////////////////////////////////////////////////////////////////////////////
            realResultLo = realSum + (imSum * currentCos) - (realDiff * currentSin);
            imResultLo = imDiff - (realDiff * currentCos) - (imSum * currentSin);

            ////////////////////////////////////////////////////////////////////////////////
            // calculate:
            //
            // realResultHi =
            //      re[(n/2)-1-i]+re[i+1] +
            //      (im[(n/2)-1-i]+im[i+1]) * cos( ((n/2)-i-1) * 2pi / n ) -
            //      (re[(n/2)-1-i]-re[i+1]) * sin( ((n/2)-i-1) * 2pi / n )
            //
            // imResultHi =
            //      im[(n/2)-1-i]-im[i+1] -
            //      (re[(n/2)-1-i]-re[i+1]) * cos( ((n/2)-i-1) * 2pi / n ) -
            //      (im[(n/2)-1-i]+im[i+1]) * sin( ((n/2)-i-1) * 2pi / n )
            //
            // taking advantage of the following:
            //
            // sin(  (i+1) * 2pi / n )  =  sin ( ((n/2)-i-1) * 2pi / n )
            // cos(  (i+1) * 2pi / n )  = -cos ( ((n/2)-i-1) * 2pi / n )
            // re[(n/2)-1-i]-re[i+1]    = -(re[i+1]-re[(n/2)-1-i])
            // re[(n/2)-1-i]-re[i+1]    = -(re[i+1]-re[(n/2)-1-i])
            ////////////////////////////////////////////////////////////////////////////////
            realResultHi = realSum - (imSum * currentCos) + (realDiff * currentSin);
            imResultHi = -imDiff - (realDiff * currentCos) - (imSum * currentSin);

            ////////////////////////////////////////////////////////////////////////////////
            // store results, advance low pointer forward to next complex element,
            // and high pointer backward to previous complex element
            ////////////////////////////////////////////////////////////////////////////////
            *pInLo++ = realResultLo / 2;
            *pInLo++ = imResultLo / 2;
            *(pInHi + 1) = imResultHi / 2;
            *pInHi = realResultHi / 2;
            pInHi -= 2;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
// FFTRealForward
//
// Performs a real forward FFT on the data.  A wrapper function for the scalar
// and AltiVec versions of the forward real FFT, since there is a minimum length
// for the AltiVec implementation.
////////////////////////////////////////////////////////////////////////////////
int FFTRealForward(float *data, unsigned long len, float *sinCosTable) {
    if (len < 0x20) {
        return fft_real_forward_scalar(data, len, sinCosTable);
    } else {
        return fft_real_forward_altivec(data, len, sinCosTable);
    }
}
