/**
*      Copyright (c) 2017, neeasade <neeasade (at) gmail (dot) org>
*
*      Permission to use, copy, modify, and/or distribute this software for any
*      purpose with or without fee is hereby granted, provided that the above
*      copyright notice and this permission notice appear in all copies.
*
*      THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
*      WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
*      MERCHANTABILITY AND FITNESS IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
*      ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
*      WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
*      ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
*      OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**/

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xproto.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#include <unistd.h>

#include "arg.h"
#include "util.h"

char *argv0;
static xcb_connection_t *conn;
static xcb_screen_t *scr;

static void usage       (char *name);
static void set2border  (xcb_window_t, int, int);

static void
usage (char *name)
{
	fprintf(stderr, "usage: %s -c <color> -s <size> [wid...]\n", name);
	exit(1);
}

static void
set2border (win, color, size)
xcb_window_t win;
int color; 
int size; 
{
	uint32_t values[1];
	short w, h, b;

	xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(conn,
			xcb_get_geometry(conn, win), NULL);

	if (geom == NULL)
		return;

	//if (size > geom->border_width)
  //warnx("warning: pixmap is greater than border size");

	w = (short)geom->width;
	h = (short)geom->height;
	b = (unsigned short)size;

	xcb_rectangle_t regular[] = {
		{w, 0, b*2, h},
		{0, h, w, b*2},
	};

	xcb_rectangle_t transparent[] = {
		{w, h, b*2, b*2},
	};

  xcb_arc_t arcs[] = {
    { w-b-1, h-b-1, b*2, b*2, 270<<6, 90 << 6 },
    { w+b, h+b, b*2, b*2, 90 << 6, 360 << 6 },
    { w+b, h-b-1, b*2, b*2, 180 << 6, 90 << 6 },
    { w-b-1, h+b, b*2, b*2, 0, 90 << 6 },
	};

	xcb_pixmap_t pmap = xcb_generate_id(conn);

  // the issue..

  //printf("%i", scr->root_depth);
  /*
	xcb_create_pixmap(conn, scr->root_depth, pmap, win,
			geom->width  + (b*2),
			geom->height + (b*2));
  */

	xcb_create_pixmap(conn, 32, pmap, win,
                    geom->width  + (b*2),
                    geom->height + (b*2));


	xcb_gcontext_t gc = xcb_generate_id(conn);
	xcb_create_gc(conn, gc, pmap, 0, NULL);

	values[0] = color;
	xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, values);
	xcb_poly_fill_rectangle(conn, pmap, gc, 2, regular);

  // todo here: transparent color

  // transparent
	values[0] = 0x00000000U;
	//values[0] = 0x000000U;
	xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, values);
	xcb_poly_fill_rectangle(conn, pmap, gc, 1, transparent);

	xcb_aux_sync(conn);

	values[0] = color;
	xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, values);
	xcb_poly_fill_arc(conn, pmap, gc, 4, arcs);

	xcb_aux_sync(conn);

	values[0] = pmap;

	xcb_change_window_attributes(conn, win, XCB_CW_BORDER_PIXMAP, values);

	xcb_free_pixmap(conn, pmap);
	xcb_free_gc(conn, gc);
}

int
main (int argc, char **argv)
{
	int color, size;

	if (argc < 2)
		usage(argv[0]);

	color = size = -1;
	ARGBEGIN {
		case 'c':
			color = strtoul(EARGF(usage(argv0)), NULL, 16);
			break;
		case 's':
			size = strtoul(EARGF(usage(argv0)), NULL, 16);
			break;
		case 'h':
			usage(argv0);
			/* NOTREACHED */
	} ARGEND

	init_xcb(&conn);
	get_screen(conn, &scr);

	/* assume remaining arguments are windows */
	while (*argv)
		set2border(strtoul(*argv++, NULL, 16), color, size);

	xcb_aux_sync(conn);

	kill_xcb(&conn);

	return 0;
}
