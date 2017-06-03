/*
 * intmath.h - misc integer math routines
 *
 * Copyright (C) 2002-2016 The EmuTOS development team
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef INTMATH_H
#define INTMATH_H

ULONG Isqrt(ULONG x);

/*
 *  min(): return minimum of two values
 */
static __inline__
WORD min(WORD a, WORD b)
{
    return (a < b) ? a : b;
}

/*
 *  max(): return maximum of two values
 */
static __inline__
WORD max(WORD a, WORD b)
{
    return (a > b) ? a : b;
}

/*
 * mul_div - signed integer multiply and divide
 *
 * mul_div (m1,m2,d1)
 *
 * ( ( m1 * m2 ) / d1 ) + 1/2
 *
 * m1 = signed 16 bit integer
 * m2 = unsigned 15 bit integer
 * d1 = signed 16 bit integer
 */

/*
 * mul_div - signed integer multiply and divide
 * return ( m1 * m2 ) / d1
 * While the operands are WORD, the intermediate result is LONG.
 */
static __inline__ WORD mul_div(WORD m1, WORD m2, WORD d1)
{
    __asm__ (
      "muls %1,%0\n\t"
      "divs %2,%0"
    : "+d"(m1)
    : "idm"(m2), "idm"(d1)
    );

    return m1;
}

#endif /* INTMATH_H */
