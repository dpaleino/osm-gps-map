/* osm-gps-map-osd-classic.c
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

#include <math.h>
#include <cairo.h>
#include "osd-utils.h"
#include "osm-gps-map-osd-classic.h"

G_DEFINE_TYPE (OsmGpsMapOsdClassic, osm_gps_map_osd_classic, OSM_TYPE_GPS_MAP_OSD)

enum
{
	PROP_0,
	PROP_DPAD_RADIUS,
	PROP_SHOW_SCALE,
	PROP_SHOW_COORDINATES,
	PROP_SHOW_CROSSHAIR,
	PROP_SHOW_DPAD,
	PROP_SHOW_ZOOM,
	PROP_SHOW_GPS_IN_DPAD,
	PROP_SHOW_GPS_IN_ZOOM
};

typedef struct _OsdScale {
    cairo_surface_t *surface;
    int zoom;
} OsdScale_t;

typedef struct _OsdCoordinates {
    cairo_surface_t *surface;
    float lat, lon;
} OsdCoordinates_t;

typedef struct _OsdCorosshair {
    cairo_surface_t *surface;
    gboolean rendered;
} OsdCrosshair_t;

typedef struct _OsdControls {
    cairo_surface_t *surface;
    gboolean rendered;
    gint gps_enabled;
} OsdControls_t;

struct _OsmGpsMapOsdClassicPrivate
{
    OsdScale_t          *scale;
    OsdCoordinates_t    *coordinates;
    OsdCrosshair_t      *crosshair;
    OsdControls_t       *controls;
	guint               dpad_radius;
	gboolean            show_scale;
	gboolean            show_coordinates;
	gboolean            show_crosshair;
	gboolean            show_dpad;
	gboolean            show_zoom;
	gboolean            show_gps_in_dpad;
	gboolean            show_gps_in_zoom;
};

static void                 osm_gps_map_osd_classic_render       (OsmGpsMapOsdClassic *self, OsmGpsMap *map);
static void                 osm_gps_map_osd_classic_draw         (OsmGpsMapOsdClassic *self, OsmGpsMap *map, GdkDrawable *drawable);
static gboolean             osm_gps_map_osd_classic_busy         (OsmGpsMapOsdClassic *self);
static gboolean             osm_gps_map_osd_classic_button_press (OsmGpsMapOsd *self, OsmGpsMap *map, GdkEventButton *event);

static void                 scale_render                         (OsmGpsMapOsdClassic *self, OsmGpsMap *map);
static void                 scale_draw                           (OsmGpsMapOsdClassic *self, GtkAllocation *allocation, cairo_t *cr);
static void                 coordinates_render                   (OsmGpsMapOsdClassic *self, OsmGpsMap *map);
static void                 coordinates_draw                     (OsmGpsMapOsdClassic *self, GtkAllocation *allocation, cairo_t *cr);
static void                 crosshair_render                     (OsmGpsMapOsdClassic *self, OsmGpsMap *map);
static void                 crosshair_draw                       (OsmGpsMapOsdClassic *self, GtkAllocation *allocation, cairo_t *cr);
static void                 controls_render                      (OsmGpsMapOsdClassic *self, OsmGpsMap *map);
static void                 controls_draw                        (OsmGpsMapOsdClassic *self, GtkAllocation *allocation, cairo_t *cr);

//FIXME: These should be private properties
#define OSD_SCALE_FONT_SIZE (12.0)
#define OSD_SCALE_W   (10*OSD_SCALE_FONT_SIZE)
#define OSD_SCALE_H   (5*OSD_SCALE_FONT_SIZE/2)
#define OSD_X         (10)
#define OSD_Y         (10)

#define OSD_SCALE_H2   (OSD_SCALE_H/2)
#define OSD_SCALE_TICK (2*OSD_SCALE_FONT_SIZE/3)
#define OSD_SCALE_M    (OSD_SCALE_H2 - OSD_SCALE_TICK)
#define OSD_SCALE_I    (OSD_SCALE_H2 + OSD_SCALE_TICK)
#define OSD_SCALE_FD   (OSD_SCALE_FONT_SIZE/4)

#define OSD_COORDINATES_FONT_SIZE (12.0)
#define OSD_COORDINATES_OFFSET (OSD_COORDINATES_FONT_SIZE/6)
#define OSD_COORDINATES_W  (8*OSD_COORDINATES_FONT_SIZE+2*OSD_COORDINATES_OFFSET)
#define OSD_COORDINATES_H  (2*OSD_COORDINATES_FONT_SIZE+2*OSD_COORDINATES_OFFSET+OSD_COORDINATES_FONT_SIZE/4)

#define OSD_CROSSHAIR_RADIUS 10
#define OSD_CROSSHAIR_TICK  (OSD_CROSSHAIR_RADIUS/2)
#define OSD_CROSSHAIR_BORDER (OSD_CROSSHAIR_TICK + OSD_CROSSHAIR_RADIUS/4)
#define OSD_CROSSHAIR_W  ((OSD_CROSSHAIR_RADIUS+OSD_CROSSHAIR_BORDER)*2)
#define OSD_CROSSHAIR_H  ((OSD_CROSSHAIR_RADIUS+OSD_CROSSHAIR_BORDER)*2)

/* parameters of dpad */
#define D_RAD  (30)

/* parameters of the "zoom" pad */
#define Z_STEP   (D_RAD/4)  // distance between dpad and zoom
#define Z_RAD    (D_RAD/2)  // radius of "caps" of zoom bar

/* shadow also depends on control size */
#define OSD_SHADOW (D_RAD/8)
#define OSD_LBL_SHADOW (OSD_SHADOW/2)

/* total width and height of controls incl. shadow */
#define OSD_W    (2*D_RAD + OSD_SHADOW + 2 * Z_RAD)
#define OSD_H    (2*D_RAD + Z_STEP + 2*Z_RAD + OSD_SHADOW)

static void
osm_gps_map_osd_classic_get_property (GObject    *object,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
	switch (property_id) {
	case PROP_DPAD_RADIUS:
		g_value_set_uint (value, OSM_GPS_MAP_OSD_CLASSIC (object)->priv->dpad_radius);
		break;
	case PROP_SHOW_SCALE:
		g_value_set_boolean (value, OSM_GPS_MAP_OSD_CLASSIC (object)->priv->show_scale);
		break;
	case PROP_SHOW_COORDINATES:
		g_value_set_boolean (value, OSM_GPS_MAP_OSD_CLASSIC (object)->priv->show_coordinates);
		break;
	case PROP_SHOW_CROSSHAIR:
		g_value_set_boolean (value, OSM_GPS_MAP_OSD_CLASSIC (object)->priv->show_crosshair);
		break;
	case PROP_SHOW_DPAD:
		g_value_set_boolean (value, OSM_GPS_MAP_OSD_CLASSIC (object)->priv->show_dpad);
		break;
	case PROP_SHOW_ZOOM:
		g_value_set_boolean (value, OSM_GPS_MAP_OSD_CLASSIC (object)->priv->show_zoom);
		break;
	case PROP_SHOW_GPS_IN_DPAD:
		g_value_set_boolean (value, OSM_GPS_MAP_OSD_CLASSIC (object)->priv->show_gps_in_dpad);
		break;
	case PROP_SHOW_GPS_IN_ZOOM:
		g_value_set_boolean (value, OSM_GPS_MAP_OSD_CLASSIC (object)->priv->show_gps_in_zoom);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
osm_gps_map_osd_classic_set_property (GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
	switch (property_id) {
	case PROP_DPAD_RADIUS:
		OSM_GPS_MAP_OSD_CLASSIC (object)->priv->dpad_radius = g_value_get_uint (value);
		break;
	case PROP_SHOW_SCALE:
		OSM_GPS_MAP_OSD_CLASSIC (object)->priv->show_scale = g_value_get_boolean (value);
		break;
	case PROP_SHOW_COORDINATES:
		OSM_GPS_MAP_OSD_CLASSIC (object)->priv->show_coordinates = g_value_get_boolean (value);
		break;
	case PROP_SHOW_CROSSHAIR:
		OSM_GPS_MAP_OSD_CLASSIC (object)->priv->show_crosshair = g_value_get_boolean (value);
		break;
	case PROP_SHOW_DPAD:
		OSM_GPS_MAP_OSD_CLASSIC (object)->priv->show_dpad = g_value_get_boolean (value);
		break;
	case PROP_SHOW_ZOOM:
		OSM_GPS_MAP_OSD_CLASSIC (object)->priv->show_zoom = g_value_get_boolean (value);
		break;
	case PROP_SHOW_GPS_IN_DPAD:
		OSM_GPS_MAP_OSD_CLASSIC (object)->priv->show_gps_in_dpad = g_value_get_boolean (value);
		break;
	case PROP_SHOW_GPS_IN_ZOOM:
		OSM_GPS_MAP_OSD_CLASSIC (object)->priv->show_gps_in_zoom = g_value_get_boolean (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static GObject *
osm_gps_map_osd_classic_constructor (GType gtype, guint n_properties, GObjectConstructParam *properties)
{
    GObject *object;
    OsmGpsMapOsdClassicPrivate *priv;

    /* Always chain up to the parent constructor */
    object = G_OBJECT_CLASS(osm_gps_map_osd_classic_parent_class)->constructor(gtype, n_properties, properties);
    priv = OSM_GPS_MAP_OSD_CLASSIC(object)->priv;

    priv->scale = g_new0(OsdScale_t, 1);
    priv->scale->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, OSD_SCALE_W, OSD_SCALE_H);
    priv->scale->zoom = -1;

    priv->coordinates = g_new0(OsdCoordinates_t, 1);
    priv->coordinates->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, OSD_COORDINATES_W, OSD_COORDINATES_H);
    priv->coordinates->lat = priv->coordinates->lon = OSM_GPS_MAP_INVALID;

    priv->crosshair = g_new0(OsdCrosshair_t, 1);
    priv->crosshair->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, OSD_CROSSHAIR_W, OSD_CROSSHAIR_H);
    priv->crosshair->rendered = FALSE;

    priv->controls = g_new0(OsdControls_t, 1);
    //FIXME: SIZE DEPENDS ON IF DPAD AND ZOOM IS THERE OR NOT
    priv->controls->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, OSD_W+2, OSD_H+2);
    priv->controls->rendered = FALSE;
    priv->controls->gps_enabled = -1;

    return object;
}

#define OSD_STRUCT_DESTROY(_x)                                  \
    if ((_x)) {                                                 \
        if ((_x)->surface)                                      \
            cairo_surface_destroy((_x)->surface);               \
        g_free((_x));                                           \
    }

static void
osm_gps_map_osd_classic_finalize (GObject *object)
{
    OsmGpsMapOsdClassicPrivate *priv = OSM_GPS_MAP_OSD_CLASSIC(object)->priv;

    OSD_STRUCT_DESTROY(priv->scale)
    OSD_STRUCT_DESTROY(priv->coordinates)
    OSD_STRUCT_DESTROY(priv->crosshair)
    OSD_STRUCT_DESTROY(priv->controls)

	G_OBJECT_CLASS (osm_gps_map_osd_classic_parent_class)->finalize (object);
}

static void
osm_gps_map_osd_classic_class_init (OsmGpsMapOsdClassicClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
    OsmGpsMapOsdClass *parent = OSM_GPS_MAP_OSD_CLASS (klass);

	g_type_class_add_private (klass, sizeof (OsmGpsMapOsdClassicPrivate));

	object_class->get_property = osm_gps_map_osd_classic_get_property;
	object_class->set_property = osm_gps_map_osd_classic_set_property;
	object_class->constructor = osm_gps_map_osd_classic_constructor;
	object_class->finalize     = osm_gps_map_osd_classic_finalize;

	parent->render = osm_gps_map_osd_classic_render;
	parent->draw = osm_gps_map_osd_classic_draw;
	parent->busy = osm_gps_map_osd_classic_busy;
	parent->button_press = osm_gps_map_osd_classic_button_press;

	/**
	 * OsmGpsMapOsdClassic:dpad-radius:
	 *
	 * The dpad radius property.
	 */
	g_object_class_install_property (object_class,
	                                 PROP_DPAD_RADIUS,
	                                 g_param_spec_uint ("dpad-radius",
	                                                     "dpad-radius",
	                                                     "dpad radius",
	                                                     0,
	                                                     G_MAXUINT,
	                                                     30,
	                                                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	/**
	 * OsmGpsMapOsdClassic:show-scale:
	 *
	 * The show scale on the map property.
	 */
	g_object_class_install_property (object_class,
	                                 PROP_SHOW_SCALE,
	                                 g_param_spec_boolean ("show-scale",
	                                                       "show-scale",
	                                                       "show scale on the map",
	                                                       TRUE,
	                                                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	/**
	 * OsmGpsMapOsdClassic:show-coordinates:
	 *
	 * The show coordinates of map centre property.
	 */
	g_object_class_install_property (object_class,
	                                 PROP_SHOW_COORDINATES,
	                                 g_param_spec_boolean ("show-coordinates",
	                                                       "show-coordinates",
	                                                       "show coordinates of map centre",
	                                                       TRUE,
	                                                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	/**
	 * OsmGpsMapOsdClassic:show-crosshair:
	 *
	 * The show crosshair at map centre property.
	 */
	g_object_class_install_property (object_class,
	                                 PROP_SHOW_CROSSHAIR,
	                                 g_param_spec_boolean ("show-crosshair",
	                                                       "show-crosshair",
	                                                       "show crosshair at map centre",
	                                                       FALSE,
	                                                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	/**
	 * OsmGpsMapOsdClassic:show-dpad:
	 *
	 * The show dpad for map navigation property.
	 */
	g_object_class_install_property (object_class,
	                                 PROP_SHOW_DPAD,
	                                 g_param_spec_boolean ("show-dpad",
	                                                       "show-dpad",
	                                                       "show dpad for map navigation",
	                                                       FALSE,
	                                                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	/**
	 * OsmGpsMapOsdClassic:show-zoom:
	 *
	 * The show zoom control for map navigation property.
	 */
	g_object_class_install_property (object_class,
	                                 PROP_SHOW_ZOOM,
	                                 g_param_spec_boolean ("show-zoom",
	                                                       "show-zoom",
	                                                       "show zoom control for map navigation",
	                                                       FALSE,
	                                                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	/**
	 * OsmGpsMapOsdClassic:show-gps-in-dpad:
	 *
	 * The show gps indicator in middle of dpad property.
	 */
	g_object_class_install_property (object_class,
	                                 PROP_SHOW_GPS_IN_DPAD,
	                                 g_param_spec_boolean ("show-gps-in-dpad",
	                                                       "show-gps-in-dpad",
	                                                       "show gps indicator in middle of dpad",
	                                                       FALSE,
	                                                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	/**
	 * OsmGpsMapOsdClassic:show-gps-in-zoom:
	 *
	 * The show gps indicator in middle of zoom control property.
	 */
	g_object_class_install_property (object_class,
	                                 PROP_SHOW_GPS_IN_ZOOM,
	                                 g_param_spec_boolean ("show-gps-in-zoom",
	                                                       "show-gps-in-zoom",
	                                                       "show gps indicator in middle of zoom control",
	                                                       FALSE,
	                                                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void
osm_gps_map_osd_classic_init (OsmGpsMapOsdClassic *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
	                                          OSM_TYPE_GPS_MAP_OSD_CLASSIC,
	                                          OsmGpsMapOsdClassicPrivate);
}

static void
osm_gps_map_osd_classic_render (OsmGpsMapOsdClassic *self,
                                OsmGpsMap *map)
{
    OsmGpsMapOsdClassicPrivate *priv;

    g_return_if_fail(OSM_IS_GPS_MAP_OSD_CLASSIC(self));
    priv = self->priv;

    if (priv->show_scale)
        scale_render(self, map);
    if (priv->show_coordinates)
        coordinates_render(self, map);
    if (priv->show_crosshair)
        crosshair_render(self, map);
    if (priv->show_zoom || priv->show_dpad)
        controls_render(self, map);

}

static void
osm_gps_map_osd_classic_draw (OsmGpsMapOsdClassic *self,
                              OsmGpsMap *map,
                              GdkDrawable *drawable)
{
    cairo_t *cr;
    OsmGpsMapOsdClassicPrivate *priv;
    GtkAllocation *allocation;

    g_return_if_fail(OSM_IS_GPS_MAP_OSD_CLASSIC(self));

    priv = self->priv;
    allocation = &(GTK_WIDGET(map)->allocation);

    cr = gdk_cairo_create(drawable);

    if (priv->show_scale)
        scale_draw(self, allocation, cr);
    if (priv->show_coordinates)
        coordinates_draw(self, allocation, cr);
    if (priv->show_crosshair)
        crosshair_draw(self, allocation, cr);
    if (priv->show_zoom || priv->show_dpad)
        controls_draw(self, allocation, cr);

    cairo_destroy(cr);
}

static gboolean
osm_gps_map_osd_classic_busy (OsmGpsMapOsdClassic *self)
{
	return FALSE;
}

static gboolean
osm_gps_map_osd_classic_button_press (OsmGpsMapOsd *self,
                                      OsmGpsMap *map,
                                      GdkEventButton *event)
{
    gboolean handled = FALSE;
    OsdControlPress_t but = OSD_NONE;
    OsmGpsMapOsdClassicPrivate *priv = self->priv;
    GtkAllocation *allocation = &(GTK_WIDGET(map)->allocation);

    if ((event->button == 1) && (event->type == GDK_BUTTON_PRESS)) {
        gint mx = event->x - OSD_X;
        gint my = event->y - OSD_Y;

        if(OSD_X < 0)
            mx -= (allocation->width - OSD_W);
    
        if(OSD_Y < 0)
            my -= (allocation->height - OSD_H);

        if (priv->show_dpad) {
            /* first do a rough test for the OSD area. */
            /* this is just to avoid an unnecessary detailed test */
            if(mx > 0 && mx < OSD_W && my > 0 && my < OSD_H) {
                but = osd_check_dpad(mx, my, D_RAD, priv->show_gps_in_dpad);
            }
        }
    }

    switch (but) {
        case OSD_LEFT:
            osm_gps_map_scroll(map, -5, 0);
            handled = TRUE;
            break;
        case OSD_RIGHT:
            osm_gps_map_scroll(map, 5, 0);
            handled = TRUE;
            break;
        case OSD_UP:
            osm_gps_map_scroll(map, 0, -5);
            handled = TRUE;
            break;
        case OSD_DOWN:
            osm_gps_map_scroll(map, 0, 5);
            handled = TRUE;
            break;
        case OSD_GPS:
        default:
            handled = FALSE;
            break;
    }

    return handled;
}

/**
 * osm_gps_map_osd_classic_new:
 *
 * Creates a new instance of #OsmGpsMapOsdClassic.
 *
 * Return value: the newly created #OsmGpsMapOsdClassic instance
 */
OsmGpsMapOsdClassic*
osm_gps_map_osd_classic_new (void)
{
	return g_object_new (OSM_TYPE_GPS_MAP_OSD_CLASSIC, NULL);
}

static void
scale_render(OsmGpsMapOsdClassic *self, OsmGpsMap *map)
{
    OsdScale_t *scale = self->priv->scale;

    if(!scale->surface)
        return;

    /* this only needs to be rendered if the zoom has changed */
    gint zoom;
    g_object_get(G_OBJECT(map), "zoom", &zoom, NULL);
    if(zoom == scale->zoom)
        return;

    scale->zoom = zoom;

    float m_per_pix = osm_gps_map_get_scale(map);

    /* first fill with transparency */
    g_assert(scale->surface);
    cairo_t *cr = cairo_create(scale->surface);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.0);
    // pink for testing:    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.2);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    /* determine the size of the scale width in meters */
    float width = (OSD_SCALE_W-OSD_SCALE_FONT_SIZE/6) * m_per_pix;

    /* scale this to useful values */
    int exp = logf(width)*M_LOG10E;
    int mant = width/pow(10,exp);
    int width_metric = mant * pow(10,exp);
    char *dist_str = NULL;
    if(width_metric<1000)
        dist_str = g_strdup_printf("%u m", width_metric);
    else
        dist_str = g_strdup_printf("%u km", width_metric/1000);
    width_metric /= m_per_pix;

    /* and now the hard part: scale for useful imperial values :-( */
    /* try to convert to feet, 1ft == 0.3048 m */
    width /= 0.3048;
    float imp_scale = 0.3048;
    char *dist_imp_unit = "ft";

    if(width >= 100) {
        /* 1yd == 3 feet */
        width /= 3.0;
        imp_scale *= 3.0;
        dist_imp_unit = "yd";

        if(width >= 1760.0) {
            /* 1mi == 1760 yd */
            width /= 1760.0;
            imp_scale *= 1760.0;
            dist_imp_unit = "mi";
        }
    }

    /* also convert this to full tens/hundreds */
    exp = logf(width)*M_LOG10E;
    mant = width/pow(10,exp);
    int width_imp = mant * pow(10,exp);
    char *dist_str_imp = g_strdup_printf("%u %s", width_imp, dist_imp_unit);

    /* convert back to pixels */
    width_imp *= imp_scale;
    width_imp /= m_per_pix;

    cairo_select_font_face (cr, "Sans",
                            CAIRO_FONT_SLANT_NORMAL,
                            CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, OSD_SCALE_FONT_SIZE);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);

    cairo_text_extents_t extents;
    cairo_text_extents (cr, dist_str, &extents);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_line_width (cr, OSD_SCALE_FONT_SIZE/6);
    cairo_move_to (cr, 2*OSD_SCALE_FD, OSD_SCALE_H2-OSD_SCALE_FD);
    cairo_text_path (cr, dist_str);
    cairo_stroke (cr);
    cairo_move_to (cr, 2*OSD_SCALE_FD,
                   OSD_SCALE_H2+OSD_SCALE_FD + extents.height);
    cairo_text_path (cr, dist_str_imp);
    cairo_stroke (cr);

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_move_to (cr, 2*OSD_SCALE_FD, OSD_SCALE_H2-OSD_SCALE_FD);
    cairo_show_text (cr, dist_str);
    cairo_move_to (cr, 2*OSD_SCALE_FD,
                   OSD_SCALE_H2+OSD_SCALE_FD + extents.height);
    cairo_show_text (cr, dist_str_imp);

    g_free(dist_str);
    g_free(dist_str_imp);

    /* draw white line */
    cairo_set_line_cap  (cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_set_line_width (cr, OSD_SCALE_FONT_SIZE/3);
    cairo_move_to (cr, OSD_SCALE_FONT_SIZE/6, OSD_SCALE_M);
    cairo_rel_line_to (cr, 0,  OSD_SCALE_TICK);
    cairo_rel_line_to (cr, width_metric, 0);
    cairo_rel_line_to (cr, 0, -OSD_SCALE_TICK);
    cairo_stroke(cr);
    cairo_move_to (cr, OSD_SCALE_FONT_SIZE/6, OSD_SCALE_I);
    cairo_rel_line_to (cr, 0, -OSD_SCALE_TICK);
    cairo_rel_line_to (cr, width_imp, 0);
    cairo_rel_line_to (cr, 0, +OSD_SCALE_TICK);
    cairo_stroke(cr);

    /* draw black line */
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_set_line_width (cr, OSD_SCALE_FONT_SIZE/6);
    cairo_move_to (cr, OSD_SCALE_FONT_SIZE/6, OSD_SCALE_M);
    cairo_rel_line_to (cr, 0,  OSD_SCALE_TICK);
    cairo_rel_line_to (cr, width_metric, 0);
    cairo_rel_line_to (cr, 0, -OSD_SCALE_TICK);
    cairo_stroke(cr);
    cairo_move_to (cr, OSD_SCALE_FONT_SIZE/6, OSD_SCALE_I);
    cairo_rel_line_to (cr, 0, -OSD_SCALE_TICK);
    cairo_rel_line_to (cr, width_imp, 0);
    cairo_rel_line_to (cr, 0, +OSD_SCALE_TICK);
    cairo_stroke(cr);

    cairo_destroy(cr);
}

static void
scale_draw(OsmGpsMapOsdClassic *self, GtkAllocation *allocation, cairo_t *cr)
{
    OsdScale_t *scale = self->priv->scale;

    gint x, y;

    x =  OSD_X;
    y = -OSD_Y;
    if(x < 0) x += allocation->width - OSD_SCALE_W;
    if(y < 0) y += allocation->height - OSD_SCALE_H;

    cairo_set_source_surface(cr, scale->surface, x, y);
    cairo_paint(cr);
}

static void
coordinates_render(OsmGpsMapOsdClassic *self, OsmGpsMap *map)
{
    OsdCoordinates_t *coordinates = self->priv->coordinates;

    if(!coordinates->surface)
        return;

    /* get current map position */
    gfloat lat, lon;
    g_object_get(G_OBJECT(map), "latitude", &lat, "longitude", &lon, NULL);

    /* check if position has changed enough to require redraw */
    if(!isnan(coordinates->lat) && !isnan(coordinates->lon))
        /* 1/60000 == 1/1000 minute */
        if((fabsf(lat - coordinates->lat) < 1/60000) &&
           (fabsf(lon - coordinates->lon) < 1/60000))
            return;

    coordinates->lat = lat;
    coordinates->lon = lon;

    /* first fill with transparency */

    g_assert(coordinates->surface);
    cairo_t *cr = cairo_create(coordinates->surface);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    //    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.5);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    cairo_select_font_face (cr, "Sans",
                            CAIRO_FONT_SLANT_NORMAL,
                            CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, OSD_COORDINATES_FONT_SIZE);

    char *latitude = osd_latitude_str(lat);
    char *longitude = osd_longitude_str(lon);
    
    int y = OSD_COORDINATES_OFFSET;
    y = osd_render_centered_text(cr, y, OSD_COORDINATES_W, OSD_COORDINATES_FONT_SIZE, latitude);
    y = osd_render_centered_text(cr, y, OSD_COORDINATES_W, OSD_COORDINATES_FONT_SIZE, longitude);
    
    g_free(latitude);
    g_free(longitude);

    cairo_destroy(cr);
}

static void
coordinates_draw(OsmGpsMapOsdClassic *self, GtkAllocation *allocation, cairo_t *cr)
{
    OsdCoordinates_t *coordinates = self->priv->coordinates;
    gint x,y;

    x = -OSD_X;
    y = -OSD_Y;
    if(x < 0) x += allocation->width - OSD_COORDINATES_W;
    if(y < 0) y += allocation->height - OSD_COORDINATES_H;

    cairo_set_source_surface(cr, coordinates->surface, x, y);
    cairo_paint(cr);
}


static void
crosshair_render(OsmGpsMapOsdClassic *self, OsmGpsMap *map)
{
    OsdCrosshair_t *crosshair = self->priv->crosshair;

    if(!crosshair->surface || crosshair->rendered)
        return;

    crosshair->rendered = TRUE;

    /* first fill with transparency */
    g_assert(crosshair->surface);
    cairo_t *cr = cairo_create(crosshair->surface);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    cairo_set_line_cap  (cr, CAIRO_LINE_CAP_ROUND);

    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.5);
    cairo_set_line_width (cr, OSD_CROSSHAIR_RADIUS/2);
    osd_render_crosshair_shape(cr, OSD_CROSSHAIR_W, OSD_CROSSHAIR_H, OSD_CROSSHAIR_RADIUS, OSD_CROSSHAIR_TICK);

    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.5);
    cairo_set_line_width (cr, OSD_CROSSHAIR_RADIUS/4);
    osd_render_crosshair_shape(cr, OSD_CROSSHAIR_W, OSD_CROSSHAIR_H, OSD_CROSSHAIR_RADIUS, OSD_CROSSHAIR_TICK);

    cairo_destroy(cr);
}


static void
crosshair_draw(OsmGpsMapOsdClassic *self, GtkAllocation *allocation, cairo_t *cr)
{
    OsdCrosshair_t *crosshair = self->priv->crosshair;
    gint x,y;

    x = (allocation->width - OSD_CROSSHAIR_W)/2;
    y = (allocation->height - OSD_CROSSHAIR_H)/2;

    cairo_set_source_surface(cr, crosshair->surface, x, y);
    cairo_paint(cr);
}

static void
controls_render(OsmGpsMapOsdClassic *self, OsmGpsMap *map)
{
    OsmGpsMapOsdClassicPrivate *priv = self->priv;
    OsdControls_t *controls = self->priv->controls;

    if(!controls->surface || controls->rendered)
        return;

    controls->rendered = TRUE;

    GdkColor bg = GTK_WIDGET(map)->style->bg[GTK_STATE_NORMAL];
    GdkColor fg = GTK_WIDGET(map)->style->fg[GTK_STATE_NORMAL];
    GdkColor da = GTK_WIDGET(map)->style->fg[GTK_STATE_INSENSITIVE];

    /* first fill with transparency */
    g_assert(controls->surface);
    cairo_t *cr = cairo_create(controls->surface);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.0);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    gint x = 1;
    gint y = 1;
    gint zoom_w = 2*D_RAD;
    gint zoom_h = D_RAD;

    /* --------- draw dpad ----------- */
    if (priv->show_dpad) {
        gint gps_w = (priv->show_gps_in_dpad ? D_RAD/2 : 0);
        osd_render_dpad(cr, x, y, D_RAD, gps_w, OSD_SHADOW /*shadow*/, &bg, &fg);
        if (priv->show_gps_in_dpad) {
            gint gps_x = x+D_RAD-(gps_w/2);
            gint gps_y = y+D_RAD-(gps_w/2);
            osd_render_gps(cr, gps_x, gps_y, gps_w, &bg, &fg);
        }
        y += (2*D_RAD);
        y += Z_STEP;    /* padding */
    }

    /* --------- draw zoom ----------- */
    if (priv->show_zoom) {
        gint gps_w = (priv->show_gps_in_zoom ? D_RAD/2 : 0);
        osd_render_zoom(cr, x, y, zoom_w, zoom_h, gps_w, OSD_SHADOW /*shadow*/, &bg, &fg);
        if (priv->show_gps_in_zoom) {
            gint gps_x = x+(zoom_w/2);
            gint gps_y = y+(zoom_h/2)-(gps_w/2);
            osd_render_gps(cr, gps_x, gps_y, gps_w, &bg, &fg);
        }
        y += zoom_h;
    }

}

static void
controls_draw(OsmGpsMapOsdClassic *self, GtkAllocation *allocation, cairo_t *cr)
{
    OsdControls_t *controls = self->priv->controls;

    gint x,y;

    x = OSD_X;
    if(x < 0)
        x += allocation->width - OSD_W;

    y = OSD_Y;
    if(y < 0)
        y += allocation->height - OSD_H;

    cairo_set_source_surface(cr, controls->surface, x, y);
    cairo_paint(cr);
}

