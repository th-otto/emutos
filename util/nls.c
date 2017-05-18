/*
 * nls.c - Native Language Support
 *
 * Copyright (C) 2001 The Emutos Development Team
 *
 * Authors:
 *  LVL   Laurent Vogel
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#include "config.h"
#include "portab.h"

#include "nls.h"
#include "langs.h"
#include "string.h"
#include "i18nconf.h"

#if CONF_WITH_NLS

/* 1024 entries, means at least 8 KB, plus 8 bytes per string,
 * plus the lengths of strings
 */
#define TH_BITS 10
#define TH_SIZE (1 << TH_BITS)
#define TH_MASK (TH_SIZE - 1)
#define TH_BMASK ((1 << (16 - TH_BITS)) - 1)

static const char * const * const *nls_hash;

/* initialisation */

void nls_init(void)
{
  nls_hash = 0;
}


static unsigned int compute_th_value(const char *t)
{
    const unsigned char *u = (const unsigned char *) t;
    unsigned short a, b;

    a = 0;
    while (*u)
    {
        a = (a << 1) | ((a >> 15) & 1);
        a += *u++;
    }
    b = (a >> TH_BITS) & TH_BMASK;
    a &= TH_MASK;
    a ^= b;
    return a;
}


const char *gettext(const char *key)
{
    unsigned int hash;
    const char *const *chain;
    const char *cmp;

    /* check for empty string - often used in RSC - must return original address */
    if (key == NULL || *key == '\0' || nls_hash == NULL)
        return key;
    hash = compute_th_value(key);
    if ((chain = nls_hash[hash]) != NULL)
    {
        while ((cmp = *chain++) != NULL)
        {
            if (strcmp(cmp, key) == 0)
            {
                /* strings are equal, return next string */
                key = *chain;
                break;
            }
            /* the strings differ, next */
            chain++;
        }
    }
    /* not in hash, return original string */
    return key;
}

/* functions to query the lang database and to set the lang */

void nls_set_lang(const char *s)
{
  int i;

  for(i = 0 ; langs[i] ; i++) {
    if(!strcmp(s, langs[i]->name)) {
      nls_hash = langs[i]->hash;
      return;
    }
  }
}

#endif /* CONF_WITH_NLS */
