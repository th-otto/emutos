/*
 * parport.h - limited parallel port support
 *
 * Copyright (C) 2002 The EmuTOS development team
 *
 * Authors:
 *  LVL   Laurent Vogel
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/licence.txt for details.
 */

#include "portab.h"

LONG __CDECL bconstat0(void);
LONG __CDECL bconin0(void);
LONG __CDECL bcostat0(void);
LONG __CDECL bconout0(WORD dev, WORD c);

void parport_init(void);
