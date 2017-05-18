/*
 * chardev.h - bios devices
 *
 * Copyright (C) 2001-2013 The EmuTOS development team
 *
 * Authors:
 *  MAD   Martin Doering
 *  LVL   Laurent Vogel
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef _CHARDEV_H
#define _CHARDEV_H

#include        "portab.h"


/* Prototypes */
LONG __CDECL bconstat1(void);
LONG __CDECL bconstat2(void);
LONG __CDECL bconstat3(void);

LONG __CDECL bconin0(void);
LONG __CDECL bconin1(void);
LONG __CDECL bconin2(void);
LONG __CDECL bconin3(void);

LONG __CDECL bconout0(WORD, WORD);
LONG __CDECL bconout1(WORD, WORD);
LONG __CDECL bconout2(WORD, WORD);
LONG __CDECL bconout3(WORD, WORD);
LONG __CDECL bconout4(WORD, WORD);
LONG __CDECL bconout5(WORD, WORD);

LONG __CDECL bcostat0(void);
LONG __CDECL bcostat1(void);
LONG __CDECL bcostat2(void);
LONG __CDECL bcostat3(void);
LONG __CDECL bcostat4(void);

LONG __CDECL char_dummy(void);
LONG __CDECL charout_dummy(WORD dev, WORD x);


/* internal init routine */

extern void chardev_init(void);

#endif /* _CHARDEV_H */
