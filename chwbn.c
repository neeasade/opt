/* See LICENSE file for copyright and license details. */

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#include "arg.h"
#include "util.h"

#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))

static xcb_connection_t *conn;

static void usage(char *name);
static void set_border(xcb_window_t);

static unsigned int borders[100];  /* widths */
static unsigned int colors[100];  /* colors */
static unsigned int num_borders;
static int border_width;

static int border_index;
static int color_index;

static xcb_screen_t *scr;

static void
usage(char *name)
{
	fprintf(stderr, "usage: %s <-sc ...> <wid> [wid...]\n", name);
	exit(1);
}

static void
get_rectangles(xcb_rectangle_t rectangles[], short offset, short cbw, short bw, short h, short w)
{
  /* outside in */
  /* right, bottom, left, top */
  rectangles[0] = (xcb_rectangle_t){ w + bw - offset - cbw, 0, cbw, h + bw - offset };
  rectangles[1] = (xcb_rectangle_t){ 0, h + bw - offset - cbw, w + bw - offset, cbw};
  rectangles[2] = (xcb_rectangle_t){ w + bw + offset, 0, cbw, h + bw - offset };
  rectangles[3] = (xcb_rectangle_t){ 0, h+bw+offset, w + bw - offset, cbw };

  /* lower left corner fix */
  rectangles[6] = (xcb_rectangle_t){ w + bw + offset, h + bw -offset-cbw, bw-offset, cbw};
  /* upper right corner fix */
  rectangles[7] = (xcb_rectangle_t){ w + bw - offset -cbw, h+bw+offset, cbw, bw - offset };

  /* corner */
  rectangles[4] = (xcb_rectangle_t){ w + bw + offset, h + bw + offset, cbw, h + bw + bw - (h + bw + offset) };
  rectangles[5] = (xcb_rectangle_t){ w + bw + offset, h + bw + offset, w + bw + bw - (w + bw + offset), cbw };
}

static void
set_border(xcb_window_t win)
{
	uint32_t values[1];
	short w, h, bw, cbw;

	xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(conn,
			xcb_get_geometry(conn, win), NULL);

	if (geom == NULL)
		return;

	w = (short)geom->width;
	h = (short)geom->height;
	bw = border_width;

	xcb_pixmap_t pmap = xcb_generate_id(conn);
	xcb_create_pixmap(conn, scr->root_depth, pmap, win,
			geom->width  + (bw*2),
			geom->height + (bw*2));

	xcb_gcontext_t gc = xcb_generate_id(conn);
	xcb_create_gc(conn, gc, pmap, 0, NULL);

	short offset = 0;

	xcb_rectangle_t rectangles[] =
	  {
	   {-1, -1, -1, -1},
	   {-1, -1, -1, -1},
	   {-1, -1, -1, -1},
	   {-1, -1, -1, -1},
	   {-1, -1, -1, -1},
	   {-1, -1, -1, -1},
	   {-1, -1, -1, -1},
	   {-1, -1, -1, -1}
	  };

	for (int i = 0; i < num_borders; i++ )
	  {
	    cbw = borders[i];
	    if (cbw != -1)
	      {
		values[0] = colors[i];

		get_rectangles(rectangles, offset, cbw, bw, h, w);

		short rects_length = 8; // todo
		xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, values);
		xcb_poly_fill_rectangle(conn, pmap, gc, rects_length, rectangles);

		xcb_aux_sync(conn);
		offset += cbw;
	      }
	  }

	/* values[0] = colbase; */
	/* xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, values); */
	/* xcb_poly_fill_rectangle(conn, pmap, gc, 8, base); */


	values[0] = pmap;
	xcb_change_window_attributes(conn, win, XCB_CW_BORDER_PIXMAP, values);

	xcb_free_pixmap(conn, pmap);
	xcb_free_gc(conn, gc);
}

int
main(int argc, char **argv)
{
	char *argv0;
	num_borders = 100;
	border_width = 0;

	// init
	for (int i =0; i< num_borders;i++)
	{
	  colors[i] = -1;
	  borders[i] = -1;
	}

	border_index = 0;
	color_index = 0;

	int temp;

	if (argc < 2)
		usage(argv[0]);

	ARGBEGIN {
	  /*
		case 'n':
		  count = strtoul(EARGF(usage(argv0)), NULL, 10);
		  num_borders = count;
		  borders = new int[count];
		  colors = new long[count];
		  break;
	  */
		case 'b':
		  temp = strtoul(EARGF(usage(argv0)), NULL, 10);
		  border_width += temp;

		  borders[border_index++] = temp;
		  break;
		case 'c':
		  colors[color_index++] = strtoul(EARGF(usage(argv0)), NULL, 16);
		  break;
		default:
			usage(argv0);
			/* NOTREACHED */
	} ARGEND

	init_xcb(&conn);
	get_screen(conn, &scr);

	/* assume remaining arguments are windows */
	while (*argv)
		set_border(strtoul(*argv++, NULL, 16));

	xcb_aux_sync(conn);

	kill_xcb(&conn);

	return 0;
}
