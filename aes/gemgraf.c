/*      GEMGRAF.C       04/11/84 - 09/17/85     Lee Lorenzen            */
/*      merge High C vers. w. 2.2               8/21/87         mdf     */
/*      fix gr_gicon null text                  11/18/87        mdf     */

/*
*       Copyright 1999, Caldera Thin Clients, Inc.
*                 2002-2017 The EmuTOS development team
*
*       This software is licenced under the GNU Public License.
*       Please see LICENSE.TXT for further information.
*
*                  Historical Copyright
*       -------------------------------------------------------------
*       GEM Application Environment Services              Version 2.3
*       Serial No.  XXXX-0000-654321              All Rights Reserved
*       Copyright (C) 1987                      Digital Research Inc.
*       -------------------------------------------------------------
*/

/* #define ENABLE_KDEBUG */

#include "config.h"
#include "portab.h"
#include "obdefs.h"
#include "intmath.h"
#include "gsxdefs.h"
#include "funcdef.h"

#include "gemdos.h"
#include "gemgraf.h"
#include "gemgsxif.h"
#include "optimize.h"
#include "optimopt.h"
#include "gsx2.h"
#include "rectfunc.h"
#include "kprint.h"
#include "string.h"

#define ORGADDR NULL

                                                /* in GSXBIND.C         */
#define g_vsf_interior( x )       gsx_1code(S_FILL_STYLE, x)
#define g_vsl_type( x )           gsx_1code(S_LINE_TYPE, x)
#define g_vsf_style( x )          gsx_1code(S_FILL_INDEX, x)
#define g_vsf_color( x )          gsx_1code(S_FILL_COLOR, x)
#define g_vsl_udsty( x )          gsx_1code(ST_UD_LINE_STYLE, x)

#define YRES_LIMIT  380     /* screens with yres less than this are considered */
                            /*  'small' for the purposes of get_char_height()  */

GLOBAL WORD     gl_width;
GLOBAL WORD     gl_height;

GLOBAL WORD     gl_wchar;
GLOBAL WORD     gl_hchar;

GLOBAL WORD     gl_wschar;
GLOBAL WORD     gl_hschar;

GLOBAL WORD     gl_wptschar;
GLOBAL WORD     gl_hptschar;

GLOBAL WORD     gl_wbox;
GLOBAL WORD     gl_hbox;

GLOBAL WORD     gl_xclip;
GLOBAL WORD     gl_yclip;
GLOBAL WORD     gl_wclip;
GLOBAL WORD     gl_hclip;

GLOBAL WORD     gl_nplanes;
GLOBAL WORD     gl_handle;

GLOBAL FDB      gl_src;
GLOBAL FDB      gl_dst;

GLOBAL WS       gl_ws;
GLOBAL WORD     contrl[12];
GLOBAL WORD     intin[128];
GLOBAL WORD     ptsin[20];

GLOBAL WORD     gl_mode;
GLOBAL WORD     gl_tcolor;
GLOBAL WORD     gl_lcolor;
GLOBAL WORD     gl_fis;
GLOBAL WORD     gl_patt;
GLOBAL WORD     gl_font;

GLOBAL GRECT    gl_rscreen;
GLOBAL GRECT    gl_rfull;
GLOBAL GRECT    gl_rzero;
GLOBAL GRECT    gl_rcenter;
GLOBAL GRECT    gl_rmenu;

static WORD     gl_wsptschar;
static WORD     gl_hsptschar;


/*
 *  Routine to set the clip rectangle.  If the w,h of the clip is 0,
 *  then no clip should be set.  Ohterwise, set the appropriate clip.
 */
void gsx_sclip(const GRECT *pt)
{
    r_get(pt, &gl_xclip, &gl_yclip, &gl_wclip, &gl_hclip);

    if (gl_wclip && gl_hclip)
    {
        ptsin[0] = gl_xclip;
        ptsin[1] = gl_yclip;
        ptsin[2] = gl_xclip + gl_wclip - 1;
        ptsin[3] = gl_yclip + gl_hclip - 1;
        vst_clip( TRUE, ptsin);
    }
    else
        vst_clip( FALSE, ptsin);
}


/*
 *  Routine to get the current clip setting
 */
void gsx_gclip(GRECT *pt)
{
    r_set(pt, gl_xclip, gl_yclip, gl_wclip, gl_hclip);
}


/*
 *  Routine to return TRUE iff the specified rectangle intersects the
 *  current clip rectangle ... or clipping is off (?)
 */
WORD gsx_chkclip(GRECT *pt)
{
    /* if clipping is on */
    if (gl_wclip && gl_hclip)
    {
        if ((pt->g_y + pt->g_h) < gl_yclip)
            return FALSE;                   /* all above    */
        if ((pt->g_x + pt->g_w) < gl_xclip)
            return FALSE;                   /* all left     */
        if ((gl_yclip + gl_hclip) <= pt->g_y)
            return FALSE;                   /* all below    */
        if ((gl_xclip + gl_wclip) <= pt->g_x)
            return FALSE;                   /* all right    */
    }

    return TRUE;
}


static void gsx_xline(WORD ptscount, WORD *ppoints)
{
    static const WORD hztltbl[2] = { 0x5555, 0xaaaa };
    static const WORD verttbl[4] = { 0x5555, 0xaaaa, 0xaaaa, 0x5555 };
    WORD    *linexy, i;
    WORD    st;

    for (i = 1; i < ptscount; i++)
    {
        if (*ppoints == *(ppoints + 2))
        {
            st = verttbl[((*ppoints & 1) | ((*(ppoints+1) & 1) << 1))];
        }
        else
        {
            linexy = (*ppoints < *(ppoints+2)) ? ppoints : ppoints + 2;
            st = hztltbl[*(linexy+1) & 1];
        }
        g_vsl_udsty(st);
        g_v_pline(2, ppoints);
        ppoints += 2;
    }
    g_vsl_udsty(0xffff);
}


/*
 *  Routine to draw a certain number of points in a polyline relative
 *  to a given x,y offset
 */
void gsx_pline(WORD offx, WORD offy, WORD cnt, const WORD *pts)
{
    WORD    i, j;

    for (i = 0; i < cnt; i++)
    {
        j = i * 2;
        ptsin[j] = offx + pts[j];
        ptsin[j+1] = offy + pts[j+1];
    }

    gsx_xline(cnt, ptsin);
}


/*
 *  Routine to draw a clipped polyline, hiding the mouse as you do it
 */
void gsx_cline(UWORD x1, UWORD y1, UWORD x2, UWORD y2)
{
    WORD pxy[4] = { x1, y1, x2, y2 };

    gsx_moff();
    g_v_pline(2, pxy);
    gsx_mon();
}


/*
 *  Routine to set the text, writing mode, and color attributes
 */
void gsx_attr(UWORD text, UWORD mode, UWORD color)
{
    WORD    tmp;

    tmp = intin[0];
    contrl[1] = 0;
    contrl[3] = 1;
    contrl[6] = gl_handle;
    if (mode != gl_mode)
    {
        contrl[0] = SET_WRITING_MODE;
        intin[0] = gl_mode = mode;
        gsx2();
    }

    contrl[0] = FALSE;
    if (text)
    {
        if (color != gl_tcolor)
        {
            contrl[0] = S_TEXT_COLOR;
            gl_tcolor = color;
        }
    }
    else
    {
        if (color != gl_lcolor)
        {
            contrl[0] = S_LINE_COLOR;
            gl_lcolor = color;
        }
    }

    if (contrl[0])
    {
        intin[0] = color;
        gsx2();
    }
    intin[0] = tmp;
}


/*
 *  Routine to set up the points for drawing a box
 */
static void gsx_bxpts(GRECT *pt)
{
    ptsin[0] = pt->g_x;
    ptsin[1] = pt->g_y;
    ptsin[2] = pt->g_x + pt->g_w - 1;
    ptsin[3] = pt->g_y;
    ptsin[4] = pt->g_x + pt->g_w - 1;
    ptsin[5] = pt->g_y + pt->g_h - 1;
    ptsin[6] = pt->g_x;
    ptsin[7] = pt->g_y + pt->g_h - 1;
    ptsin[8] = pt->g_x;
    ptsin[9] = pt->g_y;
}


/*
 *  Routine to draw a box using the current attributes
 */
static void gsx_box(GRECT *pt)
{
    gsx_bxpts(pt);
    g_v_pline(5, ptsin);
}


/*
 *  Routine to draw a box that will look right on a dithered surface
 */
void gsx_xbox(GRECT *pt)
{
    gsx_bxpts(pt);
    gsx_xline(5, ptsin);
}


/*
 *  Routine to draw a portion of the corners of a box that will look
 *  right on a dithered surface
 */
void gsx_xcbox(GRECT *pt)
{
    WORD    wa, ha;

    wa = 2 * gl_wbox;
    ha = 2 * gl_hbox;
    ptsin[0] = pt->g_x;
    ptsin[1] = pt->g_y + ha;
    ptsin[2] = pt->g_x;
    ptsin[3] = pt->g_y;
    ptsin[4] = pt->g_x + wa;
    ptsin[5] = pt->g_y;
    gsx_xline(3, ptsin);

    ptsin[0] = pt->g_x + pt->g_w - wa;
    ptsin[1] = pt->g_y;
    ptsin[2] = pt->g_x + pt->g_w - 1;
    ptsin[3] = pt->g_y;
    ptsin[4] = pt->g_x + pt->g_w - 1;
    ptsin[5] = pt->g_y + ha;
    gsx_xline(3, ptsin);

    ptsin[0] = pt->g_x + pt->g_w - 1;
    ptsin[1] = pt->g_y + pt->g_h - ha;
    ptsin[2] = pt->g_x + pt->g_w - 1;
    ptsin[3] = pt->g_y + pt->g_h - 1;
    ptsin[4] = pt->g_x + pt->g_w - wa;
    ptsin[5] = pt->g_y + pt->g_h - 1;
    gsx_xline(3, ptsin);

    ptsin[0] = pt->g_x + wa;
    ptsin[1] = pt->g_y + pt->g_h - 1;
    ptsin[2] = pt->g_x;
    ptsin[3] = pt->g_y + pt->g_h - 1;
    ptsin[4] = pt->g_x;
    ptsin[5] = pt->g_y + pt->g_h - ha;
    gsx_xline(3, ptsin);
}


/*
 *  Routine to fix up the MFDB of a particular raster form
 */
void gsx_fix(FDB *pfd, void *theaddr, WORD wb, WORD h)
{
    if (theaddr == ORGADDR)
    {
        pfd->fd_w = gl_ws.ws_xres + 1;
        pfd->fd_wdwidth = pfd->fd_w / 16;
        pfd->fd_h = gl_ws.ws_yres + 1;
        pfd->fd_nplanes = gl_nplanes;
    }
    else
    {
        pfd->fd_wdwidth = wb / 2;
        pfd->fd_w = wb * 8;
        pfd->fd_h = h;
        pfd->fd_nplanes = 1;
    }
    pfd->fd_stand = FALSE;
    pfd->fd_addr = theaddr;
}


/*
 *  Routine to blit, to and from a specific area
 */
void gsx_blt(void *saddr, UWORD sx, UWORD sy, UWORD swb,
             void *daddr, UWORD dx, UWORD dy, UWORD dwb, UWORD w, UWORD h,
             UWORD rule, WORD fgcolor, WORD bgcolor)
{
    gsx_fix(&gl_src, (void *)saddr, swb, h);
    gsx_fix(&gl_dst, (void *)daddr, dwb, h);

    gsx_moff();
    ptsin[0] = sx;
    ptsin[1] = sy;
    ptsin[2] = sx + w - 1;
    ptsin[3] = sy + h - 1;
    ptsin[4] = dx;
    ptsin[5] = dy;
    ptsin[6] = dx + w - 1;
    ptsin[7] = dy + h - 1 ;
    if (fgcolor == -1)
        vro_cpyfm(rule, ptsin, &gl_src, &gl_dst);
    else
        vrt_cpyfm(rule, ptsin, &gl_src, &gl_dst, fgcolor, bgcolor);
    gsx_mon();
}


/*
 *  Routine to blit around something on the screen
 */
void bb_screen(WORD scrule, WORD scsx, WORD scsy, WORD scdx, WORD scdy,
               WORD scw, WORD sch)
{
    gsx_blt(NULL, scsx, scsy, 0,
            NULL, scdx, scdy, 0,
            scw, sch, scrule, -1, -1);
}


/*
 *  Determine char height based on yres in WS
 */
static WORD get_char_height(WS *ws)
{
    return (ws->ws_yres<YRES_LIMIT) ? 6 : ws->ws_chmaxh;
}


static void gr_gblt(WORD *pimage, GRECT *pi, WORD col1, WORD col2)
{
    gsx_blt(pimage, 0, 0, pi->g_w/8, NULL, pi->g_x, pi->g_y,
            gl_width/8, pi->g_w, pi->g_h, MD_TRANS, col1, col2);
}


#if CONF_WITH_COLORICONS


#define TEST_COLOR 15
#define XMAX_PLANES 32
#define XMAX_COLOR 256

typedef WORD table4[XMAX_COLOR][4];

/* number of bytes per pixel (0 == planeoriented) */
static WORD xpixelbytes;
/* table for plane oriented images */
static WORD colortbl[8 * XMAX_COLOR];
/* table for pixel oriented images */
static ULONG colortbl2[XMAX_COLOR];
static table4 rgb_palette;

static WORD const pixtbl[XMAX_COLOR] = {
    0,   2,   3,   6,   4,   7,   5,   8,   9,  10,  11,  14,  12,  15,  13, 255,
   16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
   32 , 33 , 34 , 35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
   48 , 49 , 50 , 51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
   64 , 65 , 66 , 67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
   80 , 81 , 82 , 83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
   96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
  112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
  128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
  144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
  160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
  176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
  192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
  208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
  224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
  240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,   1
};

/*
 * find the color icon which is the best match
 * for the current resolution.
 * We use the one from the list that has equal
 * or less planes.  Returns NULL if no match.
 */
static CICON *find_best_match(CICON *iconlist, WORD planes)
{
    CICON *tempicon, *lasticon;

    lasticon = NULL;
    for (tempicon = iconlist; tempicon; tempicon = tempicon->next_res)
    {
        if (tempicon->num_planes == planes)
            return tempicon;
        if (tempicon->num_planes < planes)
        {
            if (!lasticon || (lasticon->num_planes < tempicon->num_planes))
                lasticon = tempicon;
        }
    }
    return lasticon;
}


static void xfix_make_selmask(WORD w, WORD h, void *dst, const void *src)
{
    UWORD mask = 0x5555;
    WORD x, y;
    UWORD *d = (UWORD *)dst;
    const UWORD *s = (const UWORD *)src;

    w >>= 1;
    for (y = h; --y >= 0;)
    {
        for (x = w; --x >= 0; )
            *d++ = (*s++) & mask;
        mask = ~mask;
    }
}


/*****************************************************************************/
/* Test how many bytes per pixel are needed in device-dependent format       */
/*****************************************************************************/

#define g_vsl_color(color) gsx_1code(S_LINE_COLOR, color)

static WORD test_rez(void)
{
    WORD i, np, color, bpp = 0;
    WORD pxy[8], rgb[3];
    WORD backup[XMAX_PLANES], test[XMAX_PLANES], test2[XMAX_PLANES];
    WORD black[3] = { 0, 0, 0 };
    WORD white[3] = { 1000, 1000, 1000 };
    FDB screen;
    FDB pixel;
    FDB stdfm;

    pixel.fd_addr = NULL;
    pixel.fd_w = 16;
    pixel.fd_h = 1;
    pixel.fd_wdwidth = 1;
    pixel.fd_stand = FALSE;
    pixel.fd_nplanes = gl_nplanes;
    pixel.fd_r1 = 0;
    pixel.fd_r2 = 0;
    pixel.fd_r3 = 0;
    stdfm = pixel;
    stdfm.fd_stand = TRUE;

    if (gl_nplanes >= 8)
    {
        if (gl_nplanes == 8)
        {
            color = 0xff;
            memset(test, 0, sizeof(test));
            memset(test2, 0, sizeof(test2));
            for (np = 0; np < gl_nplanes; np++)
                test2[np] = (color & (1 << np)) << (15 - np);

            pixel.fd_addr = test;
            stdfm.fd_addr = test2;
            vrn_trnfm(&stdfm, &pixel);

            for (i = 1; i < gl_nplanes; i++)
                if (test[i])
                    break;

            if (i >= gl_nplanes && !(test[0] & 0x00ff))
                bpp = 1;
        } else
        {
            gsx_fix(&screen, NULL, 0, 0);
            pxy[0] = 0;
            pxy[1] = 0;
            pxy[2] = screen.fd_w - 1;
            pxy[3] = screen.fd_h - 1;
            vst_clip(FALSE, pxy);

            memset(backup, 0, sizeof(backup));

            gsx_attr(FALSE, MD_REPLACE, TEST_COLOR);
            g_vsl_type(1);
            g_vsl_width(1);
            pxy[0] = 0;
            pxy[1] = 0;
            pxy[2] = 0;
            pxy[3] = 0;
            pxy[4] = 0;
            pxy[5] = 0;
            pxy[6] = 0;
            pxy[7] = 0;

            gsx_moff();

            /* save pixel */
            pixel.fd_addr = backup;
            vro_cpyfm(S_ONLY, pxy, &screen, &pixel);

            /* save old color */
            g_vq_color(TEST_COLOR, 1, rgb);

            /* set 1 white pixel */
            g_vsl_color(TEST_COLOR);
            g_vs_color(TEST_COLOR, white);
            g_v_pline(2, pxy);

            /* fetch pixel value */
            memset(test, 0, sizeof(test));
            pixel.fd_addr = test;
            vro_cpyfm(S_ONLY, pxy, &screen, &pixel);

            for (i = ((gl_nplanes + 15) >> 4) * 2; i < gl_nplanes; i++)
                if (test[i])
                    break;

            if (i >= gl_nplanes)
            {
                g_vs_color(TEST_COLOR, black);
                g_v_pline(2, pxy);

                memset(test, 0, sizeof(test));
                vro_cpyfm(S_ONLY, pxy, &screen, &pixel);

                for (i = ((gl_nplanes + 15) >> 4) * 2; i < gl_nplanes; i++)
                    if (test[i])
                        break;

                if (i >= gl_nplanes)
                    bpp = (gl_nplanes + 7) >> 3;
            }

            /* restore old color */
            g_vs_color(TEST_COLOR, rgb);

            /* restore saved pixel */
            pixel.fd_addr = backup;
            vro_cpyfm(S_ONLY, pxy, &pixel, &screen);

            gsx_mon();
        }
    }

    KINFO(("test_rez: bytes/pixel = %d\n", bpp));

    return bpp;
}

/*****************************************************************************/
/* determine pixel values for the selected palette                           */
/*****************************************************************************/

static void xfill_colortbl(void)
{
    WORD np, color, backup[XMAX_PLANES * 4];
    WORD pxy[8], rgb[3];
    FDB screen;
    FDB pixel;
    FDB stdfm;
    UWORD pixel_data[XMAX_PLANES];
    UWORD pixel_data2[XMAX_PLANES];

    pixel.fd_addr = NULL;
    pixel.fd_w = 16;
    pixel.fd_h = 1;
    pixel.fd_wdwidth = 1;
    pixel.fd_stand = FALSE;
    pixel.fd_nplanes = gl_nplanes;
    pixel.fd_r1 = 0;
    pixel.fd_r2 = 0;
    pixel.fd_r3 = 0;
    stdfm = pixel;
    stdfm.fd_stand = TRUE;

    for (color = 0; color < XMAX_COLOR; color++)
    {
        g_vq_color(pixtbl[color], 1, rgb_palette[color]);
        rgb_palette[color][3] = pixtbl[color];
    }

    if (gl_nplanes >= 8)
    {
        if (gl_nplanes > 8)
        {
            gsx_fix(&screen, NULL, 0, 0);

            pxy[0] = 0;
            pxy[1] = 0;
            pxy[2] = screen.fd_w - 1;
            pxy[3] = screen.fd_h - 1;
            vst_clip(FALSE, pxy);
            gsx_moff();

            memset(backup, 0, sizeof(backup));
            if (xpixelbytes == 0)
                memset(colortbl, 0, gl_nplanes * XMAX_COLOR * sizeof(WORD));

            gsx_attr(FALSE, MD_REPLACE, TEST_COLOR);
            g_vsl_type(1);
            g_vsl_width(1);

            /* save pixel value */
            memset(pxy, 0, sizeof(pxy));
            pxy[2] = 1;
            pxy[6] = 1;
            pixel.fd_addr = backup;
            vro_cpyfm(S_ONLY, pxy, &screen, &pixel);
            pxy[6] = 0;

            /* save old color */
            g_vq_color(TEST_COLOR, 1, rgb);

            for (color = 0; color < XMAX_COLOR; color++)
            {
                g_vs_color(TEST_COLOR, rgb_palette[color]);
                g_vsl_color(TEST_COLOR);
                pxy[2] = 1;
                g_v_pline(2, pxy);

                pixel.fd_addr = pixel_data;
                stdfm.fd_addr = pixel_data2;

                /* vro_cpyfm, because v_get_pixel does not work for TrueColor */
                pxy[2] = 0;
                memset(pixel_data, 0, sizeof(pixel_data));
                vro_cpyfm(S_ONLY, pxy, &screen, &pixel);

                if (xpixelbytes != 0)
                {
                    colortbl2[color] = 0L;
                    memcpy(&colortbl2[color], pixel.fd_addr, xpixelbytes);
                } else
                {
                    memset(pixel_data2, 0, sizeof(pixel_data2));
                    vrn_trnfm(&pixel, &stdfm);
                    for (np = 0; np < gl_nplanes; np++)
                        if (pixel_data2[np])
                            pixel_data2[np] = 0xffff;
                    memcpy(&colortbl[color * gl_nplanes], pixel_data2, gl_nplanes * sizeof(WORD));
                }
            }

            /* restore old color */
            g_vs_color(TEST_COLOR, rgb);

            /* restore old pixel */
            pixel.fd_addr = backup;
            pxy[2] = 1;
            pxy[6] = 1;
            vro_cpyfm(S_ONLY, pxy, &pixel, &screen);

            gsx_mon();
        } else
        {
            if (xpixelbytes != 0)
                for (color = 0; color < XMAX_COLOR; color++)
                    *(UBYTE *)&colortbl2[color] = color;
        }
    }
}

/*****************************************************************************/
/* std_to_byte converts an image from standard format to device dependent    */
/* format (for resolutions >= 16 Planes)                                     */
/*****************************************************************************/

static void std_to_byte(WORD *col_data, LONG len, WORD old_planes, ULONG *colortbl2, FDB *s)
{
    LONG x, i, pos;
    WORD np, *new_data, pixel, color, back[XMAX_PLANES];
    UBYTE *p1, *p2;
    ULONG colback = 0;
    WORD *plane_ptr[XMAX_PLANES];

    if ((s->fd_addr = (UWORD *)dos_alloc_anyram(len * 2 * s->fd_nplanes)) == NULL)
    {
        s->fd_addr = col_data;
        return;
    }
    KDEBUG(("std_to_byte: alloc %p\n", s->fd_addr));
    new_data = s->fd_addr;
    p1 = (UBYTE *)new_data;

    if (old_planes < 8)
    {
        colback = colortbl2[(1 << old_planes) - 1];
        colortbl2[(1 << old_planes) - 1] = colortbl2[XMAX_COLOR - 1];
    }

    for (i = 0; i < old_planes; i++)
        plane_ptr[i] = &col_data[i * len];

    pos = 0;

    for (x = 0; x < len; x++)
    {
        for (np = 0; np < old_planes; np++)
            back[np] = plane_ptr[np][x];

        for (pixel = 0; pixel < 16; pixel++)
        {
            color = 0;
            for (np = 0; np < old_planes; np++)
            {
                color |= ((back[np] & 0x8000) >> (15 - np));
                back[np] <<= 1;
            }

            switch (xpixelbytes)
            {
            case 2:
                new_data[pos++] = *(UWORD *) &colortbl2[color];
                break;

            case 3:
                p2 = (UBYTE *) &colortbl2[color];
                *(p1++) = *(p2++);
                *(p1++) = *(p2++);
                *(p1++) = *(p2++);
                break;

            case 4:
                ((ULONG *)new_data)[pos++] = colortbl2[color];
                break;
            }
        }
    }

    if (old_planes < 8)
        colortbl2[(1 << old_planes) - 1] = colback;
}

/*****************************************************************************/
/* Convert icon data to current resolution                                   */
/* (e.g. 4 Plane Icon to 24 Plane TrueColor)                                 */
/*****************************************************************************/

static void xfix_cicon(WORD *col_data, LONG len, WORD old_planes, WORD new_planes, FDB *s)
{
    LONG x, i, old_len, rest_len, new_len;
    WORD np, *new_data, mask, pixel, bit, color, back[XMAX_PLANES];
    FDB d;

    s->fd_nplanes = new_planes;
    if (old_planes == new_planes)
    {
        d = *s;
        d.fd_stand = FALSE;
        s->fd_addr = col_data;
        len *= new_planes;
        if ((d.fd_addr = dos_alloc_anyram(len)) == 0)
            d.fd_addr = s->fd_addr;

        vrn_trnfm(s, &d);
        if (d.fd_addr != s->fd_addr)
        {
            memcpy(s->fd_addr, d.fd_addr, len);
            dos_free(d.fd_addr);
        }
        return;
    }

    len >>= 1;

    old_len = old_planes * len;
    new_len = new_planes * len;
    rest_len = new_len - old_len;
    if (new_planes <= 8)
    {
        s->fd_addr = dos_alloc_anyram(new_len * 2);
        if (s->fd_addr != NULL)
        {
            KDEBUG(("xfix_cicon1: alloc %p\n", s->fd_addr));
            new_data = s->fd_addr;
            new_data += old_len;
            memset(new_data, 0, rest_len * 2);
            memcpy(s->fd_addr, col_data, old_len * 2);

            col_data = s->fd_addr;

            for (x = 0; x < len; x++)
            {
                mask = 0xffff;

                for (i = 0; i < old_len; i += len)
                    mask &= col_data[x + i];

                if (mask)
                    for (i = 0; i < rest_len; i += len)
                        new_data[x + i] |= mask;
            }

            /* convert to device dependent format */
            d = *s;
            d.fd_stand = FALSE;
            if ((d.fd_addr = dos_alloc_anyram(len * 2 * new_planes)) == 0)
                d.fd_addr = s->fd_addr;

            vrn_trnfm(s, &d);
            if (d.fd_addr != s->fd_addr)
            {
                memcpy(s->fd_addr, d.fd_addr, len * 2 * new_planes);
                dos_free(d.fd_addr);
            }
        }
    } else
    {
        /* TrueColor */
        if (xpixelbytes == 0)
        {
            s->fd_addr = NULL;
            if (colortbl != NULL)
            {
                WORD *plane_ptr[XMAX_PLANES], *pos;
                UWORD old_col[XMAX_PLANES];
                UWORD maxcol = 0;

                if (old_planes < 8)
                {
                    maxcol = (1 << old_planes) - 1;
                    memcpy(old_col, &colortbl[maxcol * new_planes], new_planes * sizeof(WORD));
                    memset(&colortbl[maxcol * new_planes], 0, new_planes * sizeof(WORD));
                }

                if ((new_data = (WORD *)dos_alloc_anyram(len * 2 * new_planes)) != NULL)
                {
                    WORD *colp;

                    KDEBUG(("xfix_cicon2: alloc %p\n", new_data));
                    memcpy(new_data, col_data, old_len * 2);
                    memset(new_data + old_len, 0, rest_len * 2);

                    for (i = 0; i < new_planes; i++)
                        plane_ptr[i] = &new_data[i * len];

                    for (x = 0; x < len; x++)
                    {
                        bit = 1;
                        for (np = 0; np < old_planes; np++)
                            back[np] = plane_ptr[np][x];

                        for (pixel = 0; pixel < 16; pixel++)
                        {
                            color = 0;
                            for (np = 0; np < old_planes; np++)
                            {
                                color += ((back[np] & 1) << np);
                                back[np] >>= 1;
                            }

                            colp = &colortbl[color * new_planes];
                            for (np = 0; np < new_planes; np++)
                            {
                                pos = plane_ptr[np] + x;
                                *pos = (*pos & ~bit) | (colp[np] & bit);
                            }

                            bit <<= 1;
                        }
                    }
                    if (old_planes < 8)
                        memcpy(&colortbl[maxcol * new_planes], old_col, new_planes * sizeof(WORD));

                    /* convert to device dependent format */
                    d = *s;
                    s->fd_addr = new_data;
                    d.fd_stand = FALSE;
                    if ((d.fd_addr = dos_alloc_anyram(len * 2 * new_planes)) == NULL)
                        d.fd_addr = s->fd_addr;

                    vrn_trnfm(s, &d);
                    if (d.fd_addr != s->fd_addr)
                    {
                        memcpy(s->fd_addr, d.fd_addr, len * 2 * new_planes);
                        dos_free(d.fd_addr);
                    }
                }
            }
        } else
        {
            std_to_byte(col_data, len, old_planes, colortbl2, s);
        }
    }
}


void fix_coloricon_data(CICONBLK *ciconblk)
{
    CICON *cicon;
    int w, h;
    LONG planesize;

    w = ciconblk->monoblk.ib_wicon;
    h = ciconblk->monoblk.ib_hicon;
    cicon = find_best_match(ciconblk->mainlist, gl_nplanes);

    ciconblk->mainlist = cicon;
    if (cicon && cicon->num_planes > 1)
    {
        planesize = calc_planesize(w, h);
        w = ((w + 15) / 16) * 2;
        gsx_fix(&gl_dst, cicon->col_data, w, h);
        gl_dst.fd_stand = TRUE;

        xfix_cicon(cicon->col_data, planesize, cicon->num_planes, gl_nplanes, &gl_dst);
        cicon->col_data = gl_dst.fd_addr;

        if (cicon->sel_data)
        {
            xfix_cicon(cicon->sel_data, planesize, cicon->num_planes, gl_nplanes, &gl_dst);
            cicon->sel_data = gl_dst.fd_addr;
        } else
        {
            /* prepare darken mask */
            cicon->sel_mask = (WORD *)dos_alloc_anyram(planesize);
            KDEBUG(("sel_mask: alloc %p\n", cicon->sel_mask));
            if (cicon->sel_mask != NULL)
                xfix_make_selmask(w, h, cicon->sel_mask, cicon->col_mask);
        }

        cicon->next_res = NULL;     /* won't need the rest of the list */
    }
}


static void gsx_cblt(WORD *saddr, UWORD sx, UWORD sy, UWORD swb, WORD *daddr, UWORD dx, UWORD dy, UWORD dwb, UWORD w, UWORD h, UWORD rule, WORD numplanes)
{
    WORD *ppts;

    ppts = &ptsin[0];

    gsx_fix(&gl_src, saddr, swb, h);
    gsx_fix(&gl_dst, daddr, dwb, h);

    gl_src.fd_nplanes = numplanes;
    gl_dst.fd_nplanes = numplanes;

    gsx_moff();

    ppts[0] = sx;
    ppts[1] = sy;
    ppts[2] = sx + w - 1;
    ppts[3] = sy + h - 1;
    ppts[4] = dx;
    ppts[5] = dy;
    ppts[6] = dx + w - 1;
    ppts[7] = dy + h - 1;
    vro_cpyfm(rule, ppts, &gl_src, &gl_dst);
    gsx_mon();
}


void gr_cicon(WORD state, const GRECT *pos, CICONBLK *ciconblk)
{
    WORD tfgcol, tbgcol;
    WORD selected;
    WORD *pmask, *pdata;
    CICON *cicon = ciconblk->mainlist;
    WORD ch;
    GRECT pi, pt;

    /* crack the color/char definition word */
    ch = ciconblk->monoblk.ib_char;
    tfgcol = (ch >> 12) & 0x000f;
    tbgcol = (ch >> 8) & 0x000f;
    ch &= 0xff;

    /* calculate positions of image & text */
    r_set(&pi, pos->g_x + ciconblk->monoblk.ib_xicon, pos->g_y + ciconblk->monoblk.ib_yicon, ciconblk->monoblk.ib_wicon, ciconblk->monoblk.ib_hicon);
    r_set(&pt, pos->g_x + ciconblk->monoblk.ib_xtext, pos->g_y + ciconblk->monoblk.ib_ytext, ciconblk->monoblk.ib_wtext, ciconblk->monoblk.ib_htext);

    /* substitute mask if color avail */
    pmask = cicon->col_mask;
    pdata = cicon->col_data;
    selected = (state & SELECTED) != 0;
    if (selected)
    {
        if (cicon->sel_data)
        {
            pdata = cicon->sel_data;
            pmask = cicon->sel_mask;
        } else
        {
            /*
             * for >1 planes, draw selection by darken the image
             * instead of inverting, if the icon didn't have a mask
             */
            if (cicon->num_planes > 1)
                selected = 2;
        }
    }

    /* do mask */
    if (!(state & WHITEBAK))
    {
        gr_gblt(pmask, &pi, cicon->num_planes == 1 && selected ? tfgcol : tbgcol, WHITE);

        if (ciconblk->monoblk.ib_ptext[0])
        {
            gr_rect(selected ? tfgcol : tbgcol, IP_SOLID, &pt);
        }
    }

    /* draw the image */
    if (cicon->num_planes == 1)
    {
        gr_gblt(pdata, &pi, selected ? tbgcol : tfgcol, WHITE);
    } else
    {
        WORD rule;

        if (!(state & WHITEBAK))
            rule = gl_nplanes > 8 ? S_AND_D : S_OR_D;
        else
            rule = S_ONLY;
        gsx_cblt(pdata, 0, 0, pi.g_w / 8, ORGADDR, pi.g_x, pi.g_y, gl_width / 8, pi.g_w, pi.g_h, rule, gl_nplanes);
        if (selected == 2)
        {
            /* use the mask created by xfix_make_selmask() to darken the image */
            gr_gblt(cicon->sel_mask, &pi, BLACK, WHITE);
        }
    }

    /* draw the character */
    gsx_attr(TRUE, MD_TRANS, selected ? tbgcol : tfgcol);
    if (ch)
    {
        intin[0] = ch;
        gsx_tblt(SMALL, pi.g_x + ciconblk->monoblk.ib_xchar, pi.g_y + ciconblk->monoblk.ib_ychar, 1);
    }

    /* draw the label */
    gsx_attr(TRUE, MD_TRANS, selected ? tbgcol : tfgcol);
    gr_gtext(TE_CNTR, SMALL, ciconblk->monoblk.ib_ptext, &pt);
}

#undef g_vsl_color
#undef TEST_COLOR
#undef XMAX_PLANES
#undef XMAX_COLOR

#endif /* CONF_WITH_COLORICONS */


/*
 *  Routine to initialize all the global variables dealing with
 *  a particular workstation open
 */
void gsx_start(void)
{
    WORD    char_height;

    gl_xclip = 0;
    gl_yclip = 0;
    gl_width = gl_wclip = gl_ws.ws_xres + 1;
    gl_height = gl_hclip = gl_ws.ws_yres + 1;
    gl_nplanes = gsx_nplanes();

    KINFO(("VDI video mode = %dx%d %d-bit\n", gl_width, gl_height, gl_nplanes));

    char_height = gl_ws.ws_chminh;
    vst_height( char_height, &gl_wsptschar, &gl_hsptschar,
                                &gl_wschar, &gl_hschar );
    char_height = get_char_height(&gl_ws);
    vst_height(char_height, &gl_wptschar, &gl_hptschar, &gl_wchar, &gl_hchar);

    gl_hbox = gl_hchar + 3;
    gl_wbox = (gl_hbox * gl_ws.ws_hpixel) / gl_ws.ws_wpixel;
    if (gl_wbox < gl_wchar + 4)
        gl_wbox = gl_wchar + 4;

    KDEBUG(("gsx_start(): gl_wchar=%d, gl_hchar=%d, gl_wbox=%d, gl_hbox=%d\n",
            gl_wchar, gl_hchar, gl_wbox, gl_hbox));

    g_vsl_type(7);
    g_vsl_width(1);
    g_vsl_udsty(0xffff);
    r_set(&gl_rscreen, 0, 0, gl_width, gl_height);
    r_set(&gl_rfull, 0, gl_hbox, gl_width, (gl_height - gl_hbox));
    r_set(&gl_rzero, 0, 0, 0, 0);
    r_set(&gl_rcenter, (gl_width-gl_wbox)/2, (gl_height-(2*gl_hbox))/2, gl_wbox, gl_hbox);
    r_set(&gl_rmenu, 0, 0, gl_width, gl_hbox);

#if CONF_WITH_COLORICONS
    xpixelbytes = test_rez();
    xfill_colortbl();
#endif
}


/*
 *  Routine to do a filled bit blit (a rectangle)
 */
void bb_fill(WORD mode, WORD fis, WORD patt, WORD hx, WORD hy, WORD hw, WORD hh)
{
    gsx_fix(&gl_dst, NULL, 0, 0);
    ptsin[0] = hx;
    ptsin[1] = hy;
    ptsin[2] = hx + hw - 1;
    ptsin[3] = hy + hh - 1;

    gsx_attr(TRUE, mode, gl_tcolor);
    if (fis != gl_fis)
    {
        g_vsf_interior(fis);
        gl_fis = fis;
    }
    if (patt != gl_patt)
    {
        g_vsf_style(patt);
        gl_patt = patt;
    }
    vr_recfl(ptsin, &gl_dst);
}


static UWORD ch_width(WORD fn)
{
    if (fn == IBM)
        return gl_wchar;

    if (fn == SMALL)
        return gl_wschar;

    return 0;
}


static UWORD ch_height(WORD fn)
{
    if (fn == IBM)
        return gl_hchar;

    if (fn == SMALL)
        return gl_hschar;

    return 0;
}


static void gsx_tcalc(WORD font, BYTE *ptext, WORD *ptextw, WORD *ptexth, WORD *pnumchs)
{
    WORD    wc, hc;

    wc = ch_width(font);
    hc = ch_height(font);

    /* figure out the width of the text string in pixels */
    *pnumchs = expand_string(intin, ptext);
    *ptextw = min(*ptextw, *pnumchs * wc);

    /* figure out the height of the text */
    *ptexth = min(*ptexth, hc);
    if (*ptexth / hc)
        *pnumchs = min(*pnumchs, *ptextw / wc);
    else
        *pnumchs = 0;
}


void gsx_tblt(WORD tb_f, WORD x, WORD y, WORD tb_nc)
{
    WORD    pts_height;

    if (tb_f == IBM)
    {
        if (tb_f != gl_font)
        {
            pts_height = get_char_height(&gl_ws);
            vst_height(pts_height, &gl_wptschar, &gl_hptschar, &gl_wchar, &gl_hchar);
            gl_font = tb_f;
        }
        y += gl_hptschar;
    }

    if (tb_f == SMALL)
    {
        if (tb_f != gl_font)
        {
            pts_height = gl_ws.ws_chminh;
            vst_height(pts_height, &gl_wsptschar, &gl_hsptschar, &gl_wschar, &gl_hschar);
            gl_font = tb_f;
        }
        y += gl_hsptschar;
    }

    contrl[0] = 8;          /* TEXT */
    contrl[1] = 1;
    contrl[6] = gl_handle;
    ptsin[0] = x;
    ptsin[1] = y;
    contrl[3] = tb_nc;
    gsx2();
}


/*
 *  Routine to convert a rectangle to its inside dimensions
 */
void gr_inside(GRECT *pt, WORD th)
{
    pt->g_x += th;
    pt->g_y += th;
    pt->g_w -= ( 2 * th );
    pt->g_h -= ( 2 * th );
}


/*
 *  Routine to draw a colored, patterned, rectangle
 */
void gr_rect(UWORD icolor, UWORD ipattern, GRECT *pt)
{
    WORD    fis;

    fis = FIS_PATTERN;
    if (ipattern == IP_HOLLOW)
        fis = FIS_HOLLOW;
    else if (ipattern == IP_SOLID)
        fis = FIS_SOLID;

    g_vsf_color(icolor);
    bb_fill(MD_REPLACE, fis, ipattern, pt->g_x, pt->g_y, pt->g_w, pt->g_h);
}


/*
 *  Routine to adjust the x,y starting point of a text string to
 *  account for its justification.  The number of characters in
 *  the string is also returned.
 */
WORD gr_just(WORD just, WORD font, BYTE *ptext, WORD w, WORD h, GRECT *pt)
{
    WORD    numchs, diff;

    /* figure out the width of the text string in pixels */
    gsx_tcalc(font, ptext, &pt->g_w, &pt->g_h, &numchs);

    h -= pt->g_h;
    if (h > 0)              /* check height */
        pt->g_y += (h + 1) / 2;

    w -= pt->g_w;
    if (w > 0)
    {
        /* do justification */
        if (just == TE_CNTR)
            pt->g_x += (w + 1) / 2;
        else if (just == TE_RIGHT)
            pt->g_x += w;
        /* try to byte align */
        if ((font == IBM) && (w > 16) && ((diff = (pt->g_x & 0x0007)) != 0))
        {
            if (just == TE_LEFT)
                pt->g_x += 8 - diff;
            else if (just == TE_CNTR)
            {
                if (diff > 3)
                    diff -= 8;
                pt->g_x -= diff;
            }
            else if (just == TE_RIGHT)
                pt->g_x -= diff;
        }
    }

    return numchs;
}


/*
 *  Routine to draw a string of graphic text
 */
void gr_gtext(WORD just, WORD font, BYTE *ptext, GRECT *pt)
{
    WORD    numchs;
    GRECT   t;

    /* figure out where & how to put out the text */
    rc_copy(pt, &t);
    numchs = gr_just(just, font, ptext, t.g_w, t.g_h, &t);
    if (numchs > 0)
        gsx_tblt(font, t.g_x, t.g_y, numchs);
}


/*
 *  Routine to crack out the border color, text color, inside pattern,
 *  and inside color from a single color information word
 */
void gr_crack(UWORD color, WORD *pbc, WORD *ptc, WORD *pip, WORD *pic, WORD *pmd)
{
    /* 4 bit encoded border color */
    *pbc = (color >> 12) & 0x000f;

    /* 4 bit encoded text color */
    *ptc = (color >> 8) & 0x000f;

    /* 1 bit used to set text writing mode */
    *pmd = (color & 0x80) ? MD_REPLACE : MD_TRANS;

    /* 3 bit encoded pattern */
    *pip = (color >> 4) & 0x0007;

    /* 4 bit encoded inside color */
    *pic = color & 0x000f;
}


/*
 *  Routine to draw an icon, which is a graphic image with a text
 *  string underneath it
 */
void gr_gicon(WORD state, WORD *pmask, WORD *pdata, BYTE *ptext, WORD ch,
              WORD chx, WORD chy, GRECT *pi, GRECT *pt)
{
    WORD    ifgcol, ibgcol;
    WORD    tfgcol, tbgcol, tmp;

    /* crack the color/char definition word */
    tfgcol = ifgcol = (ch >> 12) & 0x000f;
    tbgcol = ibgcol = (ch >> 8) & 0x000f;
    ch &= 0x00ff;

    /* invert if selected   */
    if (state & SELECTED)
    {
        tmp = tfgcol;
        tfgcol = tbgcol;
        tbgcol = tmp;
        if (!(state & DRAW3D))
        {
            tmp = ifgcol;
            ifgcol = ibgcol;
            ibgcol = tmp;
        }
    }

    /* do mask unless it's on a white background */
    if ( !((state & WHITEBAK) && (ibgcol == WHITE)) )
        gr_gblt(pmask, pi, ibgcol, ifgcol);

    if ( !((state & WHITEBAK) && (tbgcol == WHITE)) )
    {
        if (pt->g_w)
            gr_rect(tbgcol, IP_SOLID, pt);
    }

    /* draw the image */
    gr_gblt(pdata, pi, ifgcol, ibgcol);

    if ((state & SELECTED) && (state & DRAW3D))
    {
        pi->g_x--;
        pi->g_y--;
        gr_gblt(pmask, pi, ifgcol, ibgcol);
        pi->g_x += 2;
        pi->g_y += 2;
        gr_gblt(pmask, pi, ifgcol, ibgcol);
        pi->g_x--;
        pi->g_y--;
    }

    /* draw the character */
    gsx_attr(TRUE, MD_TRANS, ifgcol);
    if (ch)
    {
        intin[0] = ch;
        gsx_tblt(SMALL, pi->g_x+chx, pi->g_y+chy, 1);
    }

    /* draw the label */
    gsx_attr(TRUE, MD_TRANS, tfgcol);
    gr_gtext(TE_CNTR, SMALL, ptext, pt);
}


/*
 * Routine to draw a box of a certain thickness using the current attribute settings
 */
void gr_box(WORD x, WORD y, WORD w, WORD h, WORD th)
{
    GRECT   t, n;

    r_set(&t, x, y, w, h);
    if (th != 0)
    {
        if (th < 0)
            th--;
        gsx_moff();
        do
        {
            th += (th > 0) ? -1 : 1;
            rc_copy(&t, &n);
            gr_inside(&n, th);
            gsx_box(&n);
        } while (th != 0);
        gsx_mon();
    }
}
