/*
 * EmuTOS AES: functions and variables implemened in gemdosif.S
 *
 * Copyright (C) 2002-2016 The EmuTOS development team
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef GEMDOSIF_H
#define GEMDOSIF_H


extern LONG     drwaddr;

extern void *   tikaddr;
extern void *   tiksav;

extern LONG     NUM_TICK;                       /* number of ticks      */
                                                /*   since last sample  */
                                                /*   while someone was  */
                                                /*   waiting            */
extern LONG     CMP_TICK;                       /* indicates to tick    */
                                                /*   handler how much   */
                                                /*   time to wait before*/
                                                /*   sending the first  */
                                                /*   tchange            */


extern void __CDECL disable_interrupts(void);
extern void __CDECL enable_interrupts(void);

extern void __CDECL far_bcha(void);
extern void __CDECL far_mcha(void);
extern void __CDECL aes_wheel(void);
extern void __CDECL justretf(void);

extern void __CDECL unset_aestrap(void);
extern void __CDECL set_aestrap(void);
extern BOOL __CDECL aestrap_intercepted(void);

extern void __CDECL takeerr(void);
extern void __CDECL giveerr(void);
extern void __CDECL retake(void);

extern void __CDECL drawrat(WORD newx, WORD newy);


#endif
