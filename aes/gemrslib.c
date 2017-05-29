/*      GEMRSLIB.C      5/14/84 - 06/23/85      Lowell Webster          */
/*      merge High C vers. w. 2.2               8/24/87         mdf     */

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

#include "config.h"
#include "portab.h"
#include "struct.h"
#include "basepage.h"
#include "obdefs.h"
#include "rsdefs.h"
#include "gemlib.h"
#include "gem_rsc.h"

#include "gemdos.h"
#include "gemshlib.h"
#include "gemgraf.h"
#include "geminit.h"
#include "gemrslib.h"

#include "kprint.h"
#include "string.h"
#include "nls.h"

/*
 * defines & typedefs
 */

/* type definitions for use by an application when calling      */
/*  rsrc_gaddr and rsrc_saddr                                   */

#define R_TREE      0
#define R_OBJECT    1
#define R_TEDINFO   2
#define R_ICONBLK   3
#define R_BITBLK    4
#define R_STRING    5               /* gets pointer to free strings */
#define R_IMAGEDATA 6               /* gets pointer to free images  */
#define R_OBSPEC    7
#define R_TEPTEXT   8               /* sub ptrs in TEDINFO  */
#define R_TEPTMPLT  9
#define R_TEPVALID  10
#define R_IBPMASK   11              /* sub ptrs in ICONBLK  */
#define R_IBPDATA   12
#define R_IBPTEXT   13
#define R_BIPDATA   14              /* sub ptrs in BITBLK   */
#define R_FRSTR     15              /* gets addr of ptr to free strings     */
#define R_FRIMG     16              /* gets addr of ptr to free images      */



/*******  LOCALS  **********************/

static RSHDR   *rs_hdr;
static AESGLOBAL *rs_global;
static char    tmprsfname[128];
static char    free_str[256];   /* must be long enough for longest freestring in gem.rsc */


/*
 *  Fix up a character position, from offset,row/col to a pixel value.
 *  If width is 80 then convert to full screen width.
 */
static void fix_chpos(WORD *pfix, WORD offset)
{
    WORD coffset;
    WORD cpos;

    cpos = *pfix;
    coffset = (cpos >> 8) & 0x00ff;
    cpos &= 0x00ff;

    switch(offset)
    {
    case 0:
        cpos *= gl_wchar;
        break;
    case 1:
        cpos *= gl_hchar;
        break;
    case 2:
        if (cpos == 80)
            cpos = gl_width;
        else
            cpos *= gl_wchar;
        break;
    case 3:
        cpos *= gl_hchar;
        break;
    }

    cpos += ( coffset > 128 ) ? (coffset - 256) : coffset;
    *pfix = cpos;
}


/************************************************************************/
/* rs_obfix                                                             */
/************************************************************************/
void rs_obfix(OBJECT *tree, WORD curob)
{
    WORD offset;
    WORD *p;

    /* set X,Y,W,H */
    p = &tree[curob].ob_x;

    for (offset=0; offset<4; offset++)
        fix_chpos(p+offset, offset);
}


static void *get_sub(UWORD rsindex, UWORD offset, UWORD rsize)
{
    /* get base of objects and then index in */
    return (char *)rs_hdr + offset + rsize * rsindex;
}


/*
 *  return address of given type and index, INTERNAL ROUTINE
 */
static void *get_addr(UWORD rstype, UWORD rsindex)
{
    WORD size;
    UWORD offset;
    OBJECT *obj;
    TEDINFO *tedinfo;
    ICONBLK *iconblk;
    RSHDR *hdr = rs_hdr;

    switch(rstype)
    {
    case R_TREE:
        return rs_global->ap_ptree[rsindex];
    case R_OBJECT:
        offset = hdr->rsh_object;
        size = sizeof(OBJECT);
        break;
    case R_TEDINFO:
    case R_TEPTEXT: /* same, because te_ptext is first field of TEDINFO */
        offset = hdr->rsh_tedinfo;
        size = sizeof(TEDINFO);
        break;
    case R_ICONBLK:
    case R_IBPMASK: /* same, because ib_pmask is first field of ICONBLK */
        offset = hdr->rsh_iconblk;
        size = sizeof(ICONBLK);
        break;
    case R_BITBLK:
    case R_BIPDATA: /* same, because bi_pdata is first field of BITBLK */
        offset = hdr->rsh_bitblk;
        size = sizeof(BITBLK);
        break;
    case R_OBSPEC:
        obj = (OBJECT *)get_addr(R_OBJECT, rsindex);
        return &obj->ob_spec;
    case R_TEPTMPLT:
    case R_TEPVALID:
        tedinfo = (TEDINFO *)get_addr(R_TEDINFO, rsindex);
        if (rstype == R_TEPTMPLT)
            return &tedinfo->te_ptmplt;
        return &tedinfo->te_pvalid;
    case R_IBPDATA:
    case R_IBPTEXT:
        iconblk = (ICONBLK *)get_addr(R_ICONBLK, rsindex);
        if (rstype == R_IBPDATA)
            return &iconblk->ib_pdata;
        return &iconblk->ib_ptext;
    case R_STRING:
        return *((void **)get_sub(rsindex, hdr->rsh_frstr, sizeof(LONG)));
    case R_IMAGEDATA:
        return *((void **)get_sub(rsindex, hdr->rsh_frimg, sizeof(LONG)));
    case R_FRSTR:
        offset = hdr->rsh_frstr;
        size = sizeof(LONG);
        break;
    case R_FRIMG:
        offset = hdr->rsh_frimg;
        size = sizeof(LONG);
        break;
    default:
        return (void *)-1L;
    }

    return get_sub(rsindex, offset, size);
} /* get_addr() */


static BOOL fix_long(LONG *plong)
{
    LONG lngval;

    lngval = *plong;
    if (lngval != -1L)
    {
        *plong = (LONG)rs_hdr + lngval;
        return TRUE;
    }

    return FALSE;
}


static void fix_trindex(void)
{
    WORD ii;
    LONG *ptreebase;

    ptreebase = (LONG *)get_sub(R_TREE, rs_hdr->rsh_trindex, sizeof(LONG));
    rs_global->ap_ptree = (OBJECT **)ptreebase;

    for (ii = 0; ii < rs_hdr->rsh_ntree; ii++)
    {
        fix_long(ptreebase+ii);
    }
}


#if CONF_WITH_COLORICONS
static CICONBLK **get_coloricon_table(RSHDR *header)
{
    LONG *ctable;

    if (header->rsh_vrsn & 0x0004)
    {
        ctable = (LONG *)((LONG)header + (ULONG) header->rsh_rssize);
        if (ctable[1] != 0 && ctable[1] != -1)
        {
            ctable = (LONG *)(ctable[1] + (LONG)header);
            return (CICONBLK **)ctable;
        }
    }
    return NULL;
}
#endif


static void fix_objects(void)
{
    WORD ii;
    WORD obtype;
    OBJECT *obj;

#if CONF_WITH_COLORICONS
    CICONBLK **ctable;

    ctable = get_coloricon_table(rs_hdr);
#endif

    for (ii = 0; ii < rs_hdr->rsh_nobs; ii++)
    {
        obj = (OBJECT *)get_addr(R_OBJECT, ii);
        rs_obfix(obj, 0);
        obtype = obj->ob_type & 0x00ff;
#if CONF_WITH_COLORICONS
        if (obtype == G_CICON)
        {
            if (ctable)
            {
                obj->ob_spec.ciconblk = ctable[obj->ob_spec.index];
                continue;
            }
            /*
             * Actually an invalid resource file format.
             * We can't fall through to the default handling though,
             * as the ob_spec member is not a file offset.
             * For the same reason, there is no way of finding
             * the address of the ICONBLK structure without the
             * extension table. To avoid crashes, change the
             * object type to G_BOX instead.
             */
            obtype = obj->ob_type = G_BOX;
            obj->ob_spec.index = 0x00FF1101L;
        }
#endif
        if ((obtype != G_BOX) && (obtype != G_IBOX) && (obtype != G_BOXCHAR))
            fix_long(&obj->ob_spec.index);
    }
}


static void fix_nptrs(WORD cnt, WORD type)
{
    WORD i;

    for (i = 0; i < cnt; i++)
        fix_long(get_addr(type, i));
}


static BOOL fix_ptr(WORD type, WORD index)
{
    return fix_long(get_addr(type, index));
}


static void fix_tedinfo(void)
{
    WORD ii;
    TEDINFO *ted;

    for (ii = 0; ii < rs_hdr->rsh_nted; ii++)
    {
        ted = (TEDINFO *)get_addr(R_TEDINFO, ii);
        if (fix_ptr(R_TEPTEXT, ii))
            ted->te_txtlen = strlen(ted->te_ptext) + 1;
        if (fix_ptr(R_TEPTMPLT, ii))
            ted->te_tmplen = strlen(ted->te_ptmplt) + 1;
        fix_ptr(R_TEPVALID, ii);
    }
}


#if CONF_WITH_COLORICONS

/*
 * fix up the pointers of the monochrome part of a coloricon
 */
static CICONBLK *fix_mono(CICONBLK *ptr, LONG plane_size)
{
    void *end;

    /* data follows CICONBLK structure */
    end = (char *)(ptr + 1);
    ptr->monoblk.ib_pdata = end;
    /* mask follows data */
    end = (char *) end + plane_size;
    ptr->monoblk.ib_pmask = end;
    /* text follows mask */
    end = (char *) end + plane_size;
    /*
     * There is space for a 12-character icon text.
     * If it is longer, it is relocated just like other objects.
     */
    if (ptr->monoblk.ib_ptext == NULL)
        ptr->monoblk.ib_ptext = end;
    else
        fix_long((LONG *)&ptr->monoblk.ib_ptext);
    /* next CICONBLK structure follows icon text */
    end = (char *) end + CICON_STR_SIZE;
    return end;
}


/*
 * fix up the pointers of a single color icon
 */
static void *fixup_cicon(CICON *ptr, LONG mono_size)
{
    void *end;
    LONG color_size;

    end = (char *) ptr + sizeof(CICON);
    ptr->col_data = end;
    color_size = ptr->num_planes * mono_size;
    end = (char *) end + color_size;
    ptr->col_mask = end;
    end = (char *) end + mono_size;
    if (ptr->sel_data)
    {                                   /* there are some selected icons */
        ptr->sel_data = end;
        end = (char *) end + color_size;
        ptr->sel_mask = end;
        end = (char *) end + mono_size;
    }
    return end;
}


/*
 * fix up the pointers of all color icons in the resource
 */
static void fixup_cicons(CICONBLK *ptr, WORD tot_icons, CICONBLK **carray)
{
    WORD tot_resicons;
    WORD i, j;
    LONG mono_size;                     /* size of a single mono icon in bytes */
    CICON **next_res;
    CICON *cicon;
    WORD width, height;

    for (i = 0; i < tot_icons; i++)
    {
        carray[i] = ptr;
        width = ptr->monoblk.ib_wicon;
        height = ptr->monoblk.ib_hicon;
        /* in the file, first link contains number of CICON structures */
        tot_resicons = (WORD)(LONG)(ptr->mainlist);
        mono_size = calc_planesize(width, height);
        next_res = &ptr->mainlist;
        ptr = fix_mono(ptr, mono_size);
        if (tot_resicons)
        {
            cicon = (CICON *)ptr;
            for (j = 0; j < tot_resicons; j++)
            {
                *next_res = cicon;
                next_res = &cicon->next_res;
                cicon = fixup_cicon(cicon, mono_size);
            }
            *next_res = NULL;
            ptr = (CICONBLK *)cicon;
        }
    }
}


/*
 * Populate the G_CICON table
 */
static void fix_cicons(void)
{
    CICONBLK *ptr;
    CICONBLK **cicondata;
    CICONBLK **array_ptr;
    WORD totalicons, i;

    cicondata = get_coloricon_table(rs_hdr);
    if (cicondata)
    {
        totalicons = 0;
        array_ptr = cicondata;
        while (*array_ptr++ != (CICONBLK *)-1L)
            totalicons++;

        /*
         * the CICONBLK structures immediately follow the table
         */
        ptr = (CICONBLK *) array_ptr;

        /* fixup pointers */
        fixup_cicons(ptr, totalicons, cicondata);
        /* transform color icon data */
        for (i = 0; i < totalicons; i++)
            fix_coloricon_data(cicondata[i]);
    }
}


static void free_cicons(CICONBLK **carray)
{
    WORD i;
    CICONBLK *ciconblk;
    CICON *cicon;

    for (i = 0; ; i++)
    {
        ciconblk = carray[i];
        if (ciconblk == (CICONBLK *)-1L)
            break;
        cicon = ciconblk->mainlist;
        if (cicon && cicon->num_planes > 1)
        {
            if (cicon->num_planes != gl_nplanes)
            {
                KDEBUG(("free col_data %p\n", cicon->col_data));
                dos_free(cicon->col_data);
            }
            if (cicon->sel_data)
            {
                if (cicon->num_planes != gl_nplanes)
                {
                    dos_free(cicon->sel_data);
                    KDEBUG(("free sel_data %p\n", cicon->sel_data));
                }
            } else
            {
                /* free the darkening mask */
                KDEBUG(("free sel_mask %p\n", cicon->sel_mask));
                dos_free(cicon->sel_mask);
            }
        }
    }
}
#endif


/*
 *  Set global addresses that are used by the resource library subroutines
 */
static void rs_sglobe(AESGLOBAL *pglobal)
{
    rs_global = pglobal;
    rs_hdr = rs_global->ap_rscmem;
}


/*
 *  Free the memory associated with a particular resource load
 */
WORD rs_free(AESGLOBAL *pglobal)
{
#if CONF_WITH_COLORICONS
    RSHDR *header;
    CICONBLK **ctable;

    rs_sglobe(pglobal);                 /* set global values */

    header = rs_hdr;

    ctable = get_coloricon_table(header);
    if (ctable)
        free_cicons(ctable);

    return dos_free(header) == 0;
#else
    rs_global = pglobal;

    return !dos_free(rs_global->ap_rscmem);
#endif
}


/*
 *  Get a particular ADDRess out of a resource file that has been
 *  loaded into memory
 */
WORD rs_gaddr(AESGLOBAL *pglobal, UWORD rtype, UWORD rindex, void **rsaddr)
{
    rs_sglobe(pglobal);

    *rsaddr = get_addr(rtype, rindex);
    return (*rsaddr != (void *)-1L);
}


/*
 *  Set a particular ADDRess in a resource file that has been
 *  loaded into memory
 */
WORD rs_saddr(AESGLOBAL *pglobal, UWORD rtype, UWORD rindex, void *rsaddr)
{
    void **psubstruct;

    rs_sglobe(pglobal);

    psubstruct = (void **)get_addr(rtype, rindex);
    if (psubstruct != (void **)-1L)
    {
        *psubstruct = rsaddr;
        return TRUE;
    }

    return FALSE;
}


/*
 *  Read resource file into memory and fix everything up except the
 *  x,y,w,h, parts which depend upon a GSX open workstation.  In the
 *  case of the GEM resource file this workstation will not have
 *  been loaded into memory yet.
 */
static WORD rs_readit(AESGLOBAL *pglobal,UWORD fd)
{
    WORD ibcnt;
    ULONG rslsize;
    RSHDR hdr_buff;

    /* read the header */
    if (dos_read(fd, sizeof(hdr_buff), &hdr_buff) != sizeof(hdr_buff))
        return FALSE;           /* error or short read */

    /* get size of resource */
#if CONF_WITH_COLORICONS
    if (hdr_buff.rsh_vrsn & 0x0004) /* New format? */
    {
        /* seek to the 1st entry of the table */
        if (dos_lseek(fd, 0, (ULONG)hdr_buff.rsh_rssize) != hdr_buff.rsh_rssize)
            return FALSE;
        /* read the size */
        if (dos_read(fd, sizeof(ULONG), &rslsize) != sizeof(ULONG))
            return FALSE;
    } else
#endif
    {
        rslsize = hdr_buff.rsh_rssize;
    }

    /* allocate memory */
    rs_hdr = (RSHDR *)dos_alloc_anyram(rslsize);
    if (!rs_hdr)
        return FALSE;

    /* read it all in */
    if (dos_lseek(fd, 0, 0x0L) < 0L)    /* mode 0: absolute offset */
        return FALSE;
    if (dos_read(fd, rslsize, rs_hdr) != rslsize)
        return FALSE;           /* error or short read */

    /* init global */
    rs_global = pglobal;
    rs_global->ap_rscmem = rs_hdr;
    rs_global->ap_rsclen = rslsize;

    /*
     * transfer RT_TRINDEX to global and turn all offsets from
     * base of file into pointers
     */
#if CONF_WITH_COLORICONS
    fix_cicons();                       /* fix color icons */
#endif
    fix_trindex();
    fix_tedinfo();
    ibcnt = rs_hdr->rsh_nib;
    fix_nptrs(ibcnt, R_IBPMASK);
    fix_nptrs(ibcnt, R_IBPDATA);
    fix_nptrs(ibcnt, R_IBPTEXT);
    fix_nptrs(rs_hdr->rsh_nbb, R_BIPDATA);
    fix_nptrs(rs_hdr->rsh_nstring, R_FRSTR);
    fix_nptrs(rs_hdr->rsh_nimages, R_FRIMG);

    return TRUE;
}


/*
 *  Fix up objects separately so that we can read GEM resource before we
 *  do an open workstation, then once we know the character sizes we
 *  can fix up the objects accordingly.
 */
void rs_fixit(AESGLOBAL *pglobal)
{
    rs_sglobe(pglobal);
    fix_objects();
}


/*
 *  rs_load: the rsrc_load() implementation
 */
WORD rs_load(AESGLOBAL *pglobal, BYTE *rsfname)
{
    LONG  dosrc;
    WORD  ret;
    UWORD fd;

    /*
     * use shel_find() to get resource location
     */
    strcpy(tmprsfname,rsfname);
    if (!sh_find(tmprsfname))
        return FALSE;

    dosrc = dos_open((BYTE *)tmprsfname,0); /* mode 0: read only */
    if (dosrc < 0L)
        return FALSE;
    fd = (UWORD)dosrc;

    ret = rs_readit(pglobal,fd);
    if (ret)
        rs_fixit(pglobal);
    dos_close(fd);

    return ret;
}


/* Get a string from the GEM-RSC */
BYTE *rs_str(UWORD stnum)
{
    strcpy(free_str, gettext(rs_fstr[stnum]));
    return free_str;
}
