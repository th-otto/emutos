/*
 * vt52.h - vt52 like screen handling routine headers
 *
 *
 * Copyright (C) 2013 The EmuTOS development team
 * Copyright (C) 2004 Martin Doering
 *
 * Authors:
 *  MAD     Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */


#ifndef VT52_H
#define VT52_H

#include "portab.h"

extern void vt52_init(void);            /* initialize the vt52 console */
extern WORD __CDECL cursconf(WORD, WORD);       /* XBIOS cursor configuration */

extern void cputc(WORD);

#endif /* VT52_H */
