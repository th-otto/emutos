/*
 * EmuTOS AES
 *
 * Copyright (C) 2002-2017 The EmuTOS development team
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef GEMAPLIB_H
#define GEMAPLIB_H

extern WORD     gl_play;
extern WORD     gl_recd;
extern WORD     gl_rlen;
extern FPD      *gl_rbuf;

WORD ap_init(void);
WORD ap_rdwr(WORD code, AESPD *p, WORD length, WORD *pbuff);
WORD ap_find(BYTE *pname);
void ap_tplay(FPD *pbuff, WORD length, WORD scale);
WORD ap_trecd(FPD *pbuff, WORD length);
void ap_exit(void);

#endif
