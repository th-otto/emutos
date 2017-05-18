/*
 * floppy.h - floppy routines
 *
 * Copyright (C) 2001-2016 The EmuTOS development team
 *
 * Authors:
 *  LVL   Laurent Vogel
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef FLOPPY_H
#define FLOPPY_H

#include "portab.h"

/* bios functions */

/* Functions hdv_init, rwabs, getbpb and mediach are
 * called using variable pointers. The flop_* functions
 * are the implementation of these provided by floppy.c.
 */

extern void flop_hdv_init(void);
extern WORD flop_boot_read(void);
extern LONG flop_mediach(WORD dev);

/* xbios functions */

extern LONG __CDECL floprd(UBYTE *buf, LONG filler, WORD dev,
                   WORD sect, WORD track, WORD side, WORD count);
extern LONG __CDECL flopwr(const UBYTE *buf, LONG filler, WORD dev,
                   WORD sect, WORD track, WORD side, WORD count);
extern LONG __CDECL flopfmt(UBYTE *buf, WORD *skew, WORD dev, WORD spt,
                    WORD track, WORD side, WORD interleave,
                    ULONG magic, WORD virgin);
extern void __CDECL protobt(UBYTE *buf, LONG serial, WORD type, WORD exec);
extern LONG __CDECL flopver(WORD *buf, LONG filler, WORD dev,
                    WORD sect, WORD track, WORD side, WORD count);
extern LONG __CDECL floprate(WORD dev, WORD rate);

#if CONF_WITH_FDC

/* internal functions */

extern void __CDECL flopvbl(void);

#endif /* CONF_WITH_FDC */

/* lowlevel floppy_rwabs */
LONG floppy_rw(WORD rw, UBYTE *buf, WORD cnt, LONG recnr, WORD spt, WORD sides, WORD dev);

#endif /* FLOPPY_H */
