/* osd-utils.c
 *
 * Copyright (C) 2010 John Stowers <john.stowers@gmail.com>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with This; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#include <glib.h>
#include <math.h>
#include <cairo.h>

#include "osd-utils.h"

/* these can be overwritten with versions that support
   localization */
#define OSD_COORDINATES_CHR_N  "N"
#define OSD_COORDINATES_CHR_S  "S"
#define OSD_COORDINATES_CHR_E  "E"
#define OSD_COORDINATES_CHR_W  "W"

/* this is the classic geocaching notation */
char 
*osd_latitude_str(float latitude) {
    char *c = OSD_COORDINATES_CHR_N;
    float integral, fractional;
    
    if(isnan(latitude)) 
        return NULL;
    
    if(latitude < 0) { 
        latitude = fabs(latitude); 
        c = OSD_COORDINATES_CHR_S; 
    }

    fractional = modff(latitude, &integral);
    
    return g_strdup_printf("%s %02d° %06.3f'", 
                           c, (int)integral, fractional*60.0);
}

char 
*osd_longitude_str(float longitude) {
    char *c = OSD_COORDINATES_CHR_E;
    float integral, fractional;
    
    if(isnan(longitude)) 
        return NULL;
    
    if(longitude < 0) { 
        longitude = fabs(longitude); 
        c = OSD_COORDINATES_CHR_W; 
    }

    fractional = modff(longitude, &integral);
    
    return g_strdup_printf("%s %03d° %06.3f'", 
                           c, (int)integral, fractional*60.0);
}

/* render a string at the given screen position */
int
osd_render_centered_text(cairo_t *cr, int y, int width, int font_size, char *text) {
    if(!text) return y;

    char *p = g_malloc(strlen(text)+4);  // space for "...\n"
    strcpy(p, text);

    cairo_text_extents_t extents;
    memset(&extents, 0, sizeof(cairo_text_extents_t));
    cairo_text_extents (cr, p, &extents);
    g_assert(extents.width != 0.0);

    /* check if text needs to be truncated */
    int trunc_at = strlen(text);
    while(extents.width > width) {

        /* cut off all utf8 multibyte remains so the actual */
        /* truncation only deals with one byte */
        while((p[trunc_at-1] & 0xc0) == 0x80) {
            trunc_at--;
            g_assert(trunc_at > 0);
        }

        trunc_at--;
        g_assert(trunc_at > 0);

        strcpy(p+trunc_at, "...");
        cairo_text_extents (cr, p, &extents);
    }

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_line_width (cr, font_size/6);
    cairo_move_to (cr, (width - extents.width)/2, y - extents.y_bearing);
    cairo_text_path (cr, p);
    cairo_stroke (cr);

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_move_to (cr, (width - extents.width)/2, y - extents.y_bearing);
    cairo_show_text (cr, p);

    g_free(p);

    /* skip + 1/5 line */
    return y + 6*font_size/5;
}

void
osd_render_crosshair_shape(cairo_t *cr, int w, int h, int r, int tick) {
    cairo_arc (cr, w/2, h/2,
               r, 0,  2*M_PI);

    cairo_move_to (cr, w/2 - r,
                   h/2);
    cairo_rel_line_to (cr, -tick, 0);
    cairo_move_to (cr, w/2 + r,
                   h/2);
    cairo_rel_line_to (cr,  tick, 0);

    cairo_move_to (cr, w/2,
                   h/2 - r);
    cairo_rel_line_to (cr, 0, -tick);
    cairo_move_to (cr, w/2,
                   h/2 + r);
    cairo_rel_line_to (cr, 0, tick);

    cairo_stroke (cr);
}

