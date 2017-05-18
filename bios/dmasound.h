/*
 * dmasound.h - STe/TT/Falcon DMA sound routines
 *
 * Copyright (C) 2011-2016 The EmuTOS development team
 *
 * Authors:
 *  VRI   Vincent Rivière
 *  THH   Thomas Huth
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef DMASOUND_H
#define DMASOUND_H

#if CONF_WITH_DMASOUND

#define SNDNOTLOCK  -128        /* error codes from Unlocksnd(), Locksnd() */
#define SNDLOCKED   -129

void detect_dmasound(void);
void dmasound_init(void);

/* XBIOS DMA sound functions */
LONG __CDECL locksnd(void);
LONG __CDECL unlocksnd(void);
LONG __CDECL soundcmd(WORD mode, WORD data);
LONG __CDECL setbuffer(UWORD mode, ULONG startaddr, ULONG endaddr);
LONG __CDECL setsndmode(UWORD mode);
LONG __CDECL settracks(UWORD playtracks, UWORD rectracks);
LONG __CDECL setmontracks(UWORD montrack);
LONG __CDECL setinterrupt(UWORD mode, WORD cause);
LONG __CDECL buffoper(WORD mode);
LONG __CDECL dsptristate(WORD dspxmit, WORD dsprec);
LONG __CDECL gpio(UWORD mode, UWORD data);
LONG __CDECL devconnect(WORD source, WORD dest, WORD clk, WORD prescale, WORD protocol);
LONG __CDECL sndstatus(WORD reset);
LONG __CDECL buffptr(LONG sptr);

#endif /* CONF_WITH_DMASOUND */

#endif /* DMASOUND_H */
