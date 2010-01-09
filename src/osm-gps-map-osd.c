/* osm-gps-map-osd.c
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

#include "osm-gps-map-osd.h"

G_DEFINE_ABSTRACT_TYPE (OsmGpsMapOsd, osm_gps_map_osd, G_TYPE_OBJECT)


static void
osm_gps_map_osd_finalize (GObject *object)
{
	G_OBJECT_CLASS (osm_gps_map_osd_parent_class)->finalize (object);
}

static void
osm_gps_map_osd_class_init (OsmGpsMapOsdClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize     = osm_gps_map_osd_finalize;
}

static void
osm_gps_map_osd_init (OsmGpsMapOsd *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
	                                          OSM_TYPE_GPS_MAP_OSD,
	                                          OsmGpsMapOsdPrivate);
}

#if 0
/**
 * osm_gps_map_osd_new:
 *
 * Creates a new instance of #OsmGpsMapOsd.
 *
 * Return value: the newly created #OsmGpsMapOsd instance
 */
OsmGpsMapOsd*
osm_gps_map_osd_new (void)
{
	return g_object_new (OSM_TYPE_GPS_MAP_OSD, NULL);
}
#endif

/**
 * osm_gps_map_osd_render:
 * @self: A #OsmGpsMapOsd
 *
 *
 */
void
osm_gps_map_osd_render (OsmGpsMapOsd *self,
                        OsmGpsMap *map)
{
	OSM_GPS_MAP_OSD_GET_CLASS (self)->render(self, map);
}

/**
 * osm_gps_map_osd_draw:
 * @self: A #OsmGpsMapOsd
 *
 *
 */
void
osm_gps_map_osd_draw (OsmGpsMapOsd *self,
                      GtkAllocation *allocation,
                      GdkDrawable *drawable)
{
	OSM_GPS_MAP_OSD_GET_CLASS (self)->draw(self, allocation, drawable);
}

/**
 * osm_gps_map_osd_busy:
 * @self: A #OsmGpsMapOsd
 * @return: TRUE if busy animating
 *
 */
gboolean
osm_gps_map_osd_busy (OsmGpsMapOsd *self)
{
	return OSM_GPS_MAP_OSD_GET_CLASS (self)->busy(self);
}

/**
 * osm_gps_map_osd_render:
 * @self: A #OsmGpsMapOsd
 * @return: TRUE if we handle the event
 *
 */
gboolean
osm_gps_map_osd_button_press (OsmGpsMapOsd *self,
                              GdkEventButton *event)
{
	OSM_GPS_MAP_OSD_GET_CLASS (self)->button_press(self, event);
}


