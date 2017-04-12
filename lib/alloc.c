/* -*-  mode:c; tab-width:8; c-basic-offset:8; indent-tabs-mode:nil;  -*- */
/*
   Copyright (C) 2017 by Ronnie Sahlberg <ronniesahlberg@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifdef STDC_HEADERS
#include <stddef.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <smb2.h>
#include <libsmb2.h>
#include "libsmb2-private.h"

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

struct smb2_alloc_entry {
        struct smb2_alloc_entry *next;
        size_t len;
        char buf[0];
};

struct smb2_alloc_header {
        struct smb2_alloc_entry *mem;
        char buf[0];
};

void *
smb2_alloc_init(struct smb2_context *smb2, size_t size)
{
        struct smb2_alloc_header *ptr;

        ptr = malloc(offsetof(struct smb2_alloc_header, buf) + size);
        if (ptr == NULL) {
                return NULL;
        }
        memset(ptr, 0, sizeof(*ptr));

        return &ptr->buf[0];
}

void *
smb2_alloc_data(struct smb2_context *smb2, void *data, size_t size)
{
        struct smb2_alloc_header *hdr;
        struct smb2_alloc_entry *ptr;

        ptr = malloc(offsetof(struct smb2_alloc_entry, buf)+ size);
        if (ptr == NULL) {
                smb2_set_error(smb2, "Failed to alloc %zu bytes", size);
                return NULL;
        }
        memset(ptr, 0, sizeof(*ptr));

        hdr = container_of(data, struct smb2_alloc_header, buf);

        ptr->next = hdr->mem;
        hdr->mem = ptr;

        return &ptr->buf[0];
}

void
smb2_free_data(struct smb2_context *smb2, void *ptr)
{
        struct smb2_alloc_header *hdr;
        struct smb2_alloc_entry *ent;

        if (ptr == NULL) {
                return;
        }

        hdr = container_of(ptr, struct smb2_alloc_header, buf);

        while ((ent = hdr->mem)) {
                hdr->mem = ent->next;
                free(ent);
        }
        free(hdr);
}
