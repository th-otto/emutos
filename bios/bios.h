/*
 * bios.h - misc BIOS function prototypes
 *
 * Copyright (C) 2011-2016 The EmuTOS development team
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef BIOS_H
#define BIOS_H

void __CDECL biosmain(void) NORETURN;
LONG __CDECL bios_do_unimpl(WORD number);

/* misc BIOS functions */
LONG __CDECL bconstat(WORD handle);
LONG __CDECL bconin(WORD handle);
LONG __CDECL bconout(WORD handle, WORD what);
LONG __CDECL lrwabs(WORD r_w, UBYTE *adr, WORD numb, WORD first, WORD drive, LONG lfirst);
LONG __CDECL setexc(WORD num, LONG vector);
LONG __CDECL tickcal(void);
LONG __CDECL getbpb(WORD drive);
LONG __CDECL bcostat(WORD handle);
LONG __CDECL mediach(WORD drv);
LONG __CDECL drvmap(void);

/* utility functions */
#if CONF_SERIAL_CONSOLE_ANSI
void bconout_str(WORD handle, const char* str);
#endif

#endif /* BIOS_H */
