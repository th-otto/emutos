/*
 * proc.h - processes defines
 *
 * Copyright (C) 2001-2015 The EmuTOS development team.
 *
 * Authors:
 *  LVL   Laurent Vogel
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef PROC_H
#define PROC_H

#include "pghdr.h"

/*
 *  process management
 */

/*
 * in proc.c
 */

long __CDECL xexec(WORD, char *, char *, char *);
void __CDECL x0term(void);
void __CDECL xterm(UWORD rc)  NORETURN ;
WORD __CDECL xtermres(long blkln, WORD rc);

/*
 * in kpgmld.c
 */

LONG kpgmhdrld(char *s, PGMHDR01 *hd, FH *h);
LONG kpgmld(PD *p, FH h, PGMHDR01 *hd);

#if DETECT_NATIVE_FEATURES
LONG kpgm_relocate( PD *p, long length); /* SOP */
#endif

/*
 * in rwa.S
 */

void __CDECL gouser(void)  NORETURN;
void __CDECL termuser(void)  NORETURN;

#endif /* PROC_H */
