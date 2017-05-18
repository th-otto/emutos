/*
 * midi.c - MIDI routines
 *
 * Copyright (C) 2001 The EmuTOS development team
 *
 * Authors:
 *  LVL   Laurent Vogel
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef MIDI_H
#define MIDI_H

#include "portab.h"

/* initialise the MIDI ACIA */
extern void midi_init(void);

/* some bios functions */
extern LONG __CDECL bconstat3(void);
extern LONG __CDECL bconin3(void);
extern LONG __CDECL bcostat3(void);
extern LONG __CDECL bconout3(WORD dev, WORD c);

/* some xbios functions */
extern void __CDECL midiws(WORD cnt, const UBYTE *ptr);

#endif /* MIDI_H */
