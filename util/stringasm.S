/*
 * stringasm.S - simple implementation of <string.h>-like routines
 *
 * Copyright (C) 2002-2016 The EmuTOS development team
 *
 * Authors:
 *  LVL   Laurent Vogel
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#include "asmdefs.h"

        .globl  _strlencpy
        .globl  _strchr

/*
 * WORD strlencpy(char *dest, const char *src)
 * as strcpy but returns the length ot the string instead.
 */

_strlencpy:
#ifndef __FASTCALL__
        movea.l 8(sp),a1
        movea.l 4(sp),a0
#endif
        moveq   #0,d0
lstcpy_loop:
#ifdef __mcoldfire__
        addq.l  #1,d0
#else
        addq.w  #1,d0
#endif
        move.b  (a1)+,(a0)+
        bne.s   lstcpy_loop
#ifdef __mcoldfire__
        subq.l  #1,d0
#else
        subq.w  #1,d0
#endif
        rts

/*
 * char *strchr(const char *s, int c)
 *
 * returns a pointer to the first occurrence of char c in string s, or NULL.
 */

_strchr:
#ifndef __FASTCALL__
        move.l  4(sp),a0
        move.w  8(sp),d0
#endif
strchr_loop:
        move.b  (a0)+,d1
        beq.s   strchr_null
        cmp.b   d1,d0
        bne.s   strchr_loop
        subq.l  #1,a0
        move.l  a0,d0
        rts
strchr_null:
#ifdef __FASTCALL__
        suba.l  a0,a0
#else
        moveq.l #0,d0
#endif
        rts
