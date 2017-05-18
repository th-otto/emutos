/*
 * console.h - console header
 *
 * Copyright (C) 2001-2016 The EmuTOS development team
 *
 * Authors:
 *  LVL   Laurent Vogel
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef CONSOLE_H
#define CONSOLE_H

/******************************************
**
** BDOS level character device file handles
**
*******************************************
*/

#define H_Console       -1
#define H_Aux           -2
#define H_Print         -3


/****************************************
**
** Character device handle conversion
** (BDOS-type handle to BIOS-type handle)
**
*****************************************
*/

#define HXFORM(h)       (3+h)



void stdhdl_init(void);
BYTE get_default_handle(int stdh);

long __CDECL xconstat(void);
long __CDECL xconostat(void);
long __CDECL xprtostat(void);
long __CDECL xauxistat(void);
long __CDECL xauxostat(void);
long __CDECL xconout(int ch);
long __CDECL xauxout(int ch);
long __CDECL xprtout(int ch);
long __CDECL xrawcin(void);
long __CDECL xconin(void);
long __CDECL xnecin(void);
long __CDECL xauxin(void);
long __CDECL xrawio(int parm);
void __CDECL xconws(char *p);
void __CDECL xconrs(char *p);
int cgets(int h, int maxlen, char *buf);
long conin(int h);
void tabout(int h, int ch);



#endif /* CONSOLE_H */
