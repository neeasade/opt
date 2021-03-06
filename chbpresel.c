/** *      Copyright (c) 2014, Broseph <dcat (at) iotek (dot) org> * *      Permission to use, copy, modify, and/or distribute this software for any
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
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#include "arg.h"
#include "util.h"

char *argv0;
static xcb_connection_t *conn;
static xcb_screen_t *scr;

static void usage       (char *name);
static void set2border  (xcb_window_t, int, int, int);

static void
usage (char *name)
{
	fprintf(stderr, "usage: %s <-I color> <-O color> <-i size> <-o size> [wid...]\n", name);
	exit(1);
}

static void
set2border (win, c, C, direction)
xcb_window_t win;
int c; /* back color */
int C; /* forward color */
int direction; /* direction tlbr - 1 2 3 4 */
{
	//if (os < 0 || oc < 0 || is < 0 || ic < 0)
		//return;

	uint32_t values[1];
	short w, h, b;

	xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(conn,
			xcb_get_geometry(conn, win), NULL);

	if (geom == NULL)
		return;

	//if (is + os > geom->border_width)
		//warnx("warning: pixmap is greater than border size");

	w = (short)geom->width;
	h = (short)geom->height;
	b = (short)geom->border_width;

	xcb_rectangle_t left[] = {
	  { w + b, 0, b, h +b + b },
	  { 0, h, w/2, b},
	  { 0, h+b, w/2, b}
	};

	xcb_rectangle_t right[] = {
	  { w, 0, b, h +b + b },
	  { w/2, h, w/2, b},
	  { w/2, h + b, w/2, b}
	};

	xcb_rectangle_t up[] = {
	  { 0, h + b, w + b + b, b},
	  { w, 0, b, h/2},
	  { w+b, 0, b, h/2}
	};

	xcb_rectangle_t down[] = {
	  { 0, h, w + b + b, b},
	  { w, h/2, b, h/2},
	  { w+b, h/2, b, h/2}
	};

	xcb_pixmap_t pmap = xcb_generate_id(conn);
	xcb_create_pixmap(conn, scr->root_depth, pmap, win,
			geom->width  + (b*2),
			geom->height + (b*2));
	xcb_gcontext_t gc = xcb_generate_id(conn);
	xcb_create_gc(conn, gc, pmap, 0, NULL);

	// focus direction
	values[0] = C;
	xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, values);
	switch(direction)
	{
		case 1: xcb_poly_fill_rectangle(conn, pmap, gc, 3, up); break;
		case 2: xcb_poly_fill_rectangle(conn, pmap, gc, 3, left); break;
		case 3: xcb_poly_fill_rectangle(conn, pmap, gc, 3, down); break;
		case 4: xcb_poly_fill_rectangle(conn, pmap, gc, 3, right); break;
		default: break;
	}

	// behind
	values[0] = c;
	xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, values);
	switch(direction)
	{
		case 1: xcb_poly_fill_rectangle(conn, pmap, gc, 3, down); break;
		case 2: xcb_poly_fill_rectangle(conn, pmap, gc, 3, right); break;
		case 3: xcb_poly_fill_rectangle(conn, pmap, gc, 3, up); break;
		case 4: xcb_poly_fill_rectangle(conn, pmap, gc, 3, left); break;
		default: break;
	}

	values[0] = pmap;
	xcb_change_window_attributes(conn, win, XCB_CW_BORDER_PIXMAP, values);

	xcb_free_pixmap(conn, pmap);
	xcb_free_gc(conn, gc);
}

int
main (int argc, char **argv)
{
	int C, c, d;

	if (argc < 2)
		usage(argv[0]);

	C = c = d = -1;
	ARGBEGIN {
		case 'C':
			C = strtoul(EARGF(usage(argv0)), NULL, 16);
			break;
		case 'c':
			c = strtoul(EARGF(usage(argv0)), NULL, 16);
			break;
		case 'd':
			d = strtoul(EARGF(usage(argv0)), NULL, 10);
			break;
		case 'h':
			usage(argv0);
			/* NOTREACHED */
	} ARGEND

	init_xcb(&conn);
	get_screen(conn, &scr);

	/* assume remaining arguments are windows */
	while (*argv)
		set2border(strtoul(*argv++, NULL, 16), c, C, d);

	xcb_aux_sync(conn);

	kill_xcb(&conn);

	return 0;
}
