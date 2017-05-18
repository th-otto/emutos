/*
 * screen.h - low-level screen routines
 *
 * Copyright (C) 2001-2017 The EmuTOS development team
 *
 * Authors:
 *  LVL   Laurent Vogel
 *  THH   Thomas Huth
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef SCREEN_H
#define SCREEN_H

#include "portab.h"
#include "tosvars.h"

#define ST_VRAM_SIZE        32000UL
#define TT_VRAM_SIZE        153600UL
#define FALCON_VRAM_SIZE    307200UL

#if CONF_WITH_ATARI_VIDEO

#define VIDEOBASE_ADDR_HI   0xffff8201L
#define VIDEOBASE_ADDR_MID  0xffff8203L
#define VIDEOBASE_ADDR_LOW  0xffff820dL

#define SYNCMODE            0xffff820aL

#define ST_SHIFTER          0xffff8260L
#define TT_SHIFTER          0xffff8262L
#define SPSHIFT             0xffff8266L

#define TT_SHIFTER_BITMASK  0x970f      /* valid bits in TT_SHIFTER */
#define TT_HYPER_MONO       0x1000          /* bit usage */

#define ST_PALETTE_REGS     0xffff8240L
#define FALCON_PALETTE_REGS 0xffff9800L

#define TT_PALETTE_BITMASK  0x0fff      /* valid bits in TT_PALETTE_REGS */

#define TT_DUOCHROME_INVERT 0x0002      /* inversion bit in TT h/w palette reg 0 */

/* hardware dependant xbios routines */

WORD __CDECL esetshift(WORD mode);
WORD __CDECL egetshift(void);
WORD __CDECL esetbank(WORD bank);
WORD __CDECL esetcolor(WORD index,UWORD color);
WORD __CDECL esetpalette(WORD index,WORD count,UWORD *rgb);
WORD __CDECL egetpalette(WORD index,WORD count,UWORD *rgb);
WORD __CDECL esetgray(WORD mode);
WORD __CDECL esetsmear(WORD mode);

/* pallette color definitions */

#define RGB_BLACK     0x0000            /* ST(e) palette */
#define RGB_BLUE      0x000f
#define RGB_GREEN     0x00f0
#define RGB_CYAN      0x00ff
#define RGB_RED       0x0f00
#define RGB_MAGENTA   0x0f0f
#define RGB_LTGRAY    0x0555
#define RGB_GRAY      0x0333
#define RGB_LTBLUE    0x033f
#define RGB_LTGREEN   0x03f3
#define RGB_LTCYAN    0x03ff
#define RGB_LTRED     0x0f33
#define RGB_LTMAGENTA 0x0f3f
#define RGB_YELLOW    0x0ff0
#define RGB_LTYELLOW  0x0ff3
#define RGB_WHITE     0x0fff

#define TTRGB_BLACK     0x0000          /* TT Palette */
#define TTRGB_BLUE      0x000f
#define TTRGB_GREEN     0x00f0
#define TTRGB_CYAN      0x00ff
#define TTRGB_RED       0x0f00
#define TTRGB_MAGENTA   0x0f0f
#define TTRGB_LTGRAY    0x0aaa
#define TTRGB_GRAY      0x0666
#define TTRGB_LTBLUE    0x099f
#define TTRGB_LTGREEN   0x09f9
#define TTRGB_LTCYAN    0x09ff
#define TTRGB_LTRED     0x0f99
#define TTRGB_LTMAGENTA 0x0f9f
#define TTRGB_YELLOW    0x0ff0
#define TTRGB_LTYELLOW  0x0ff9
#define TTRGB_WHITE     0x0fff

/* TT resolutions */
#define TT_HIGH        6
#define TT_MEDIUM      4
#define TT_LOW         7

#endif /* CONF_WITH_ATARI_VIDEO */

/* ST(e) resolutions */
#define ST_HIGH        2
#define ST_MEDIUM      1
#define ST_LOW         0
#define FALCON_REZ     3    /* used as a Falcon indicator */

/* monitor types (from VgetMonitor()) */
#define MON_MONO       0    /* ST monochrome */
#define MON_COLOR      1    /* ST colour */
#define MON_VGA        2    /* VGA */
#define MON_TV         3    /* TV via RF modulator */

/* misc routines */
WORD check_moderez(WORD moderez);
void initialise_palette_registers(WORD rez,WORD mode);

/* determine monitor type, ... */
void screen_init(void);
void set_rez_hacked(void);
int rez_changeable(void);
WORD get_monitor_type(void);
void screen_get_current_mode_info(UWORD *planes, UWORD *hz_rez, UWORD *vt_rez);
void get_pixel_size(WORD *width,WORD *height);

/* hardware independant xbios routines */

const UBYTE *__CDECL physbase(void);
UBYTE *__CDECL logbase(void);
WORD __CDECL getrez(void);
void __CDECL setscreen(UBYTE *logLoc, const UBYTE *physLoc, WORD rez, WORD videlmode);
void __CDECL setpalette(const UWORD *palettePtr);
WORD __CDECL setcolor(WORD colorNum, WORD color);
void __CDECL vsync(void);

#endif /* SCREEN_H */
