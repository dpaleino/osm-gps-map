/* osm-gps-map-osd-classic.h
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

#ifndef __OSM_GPS_MAP_OSD_CLASSIC_H__
#define __OSM_GPS_MAP_OSD_CLASSIC_H__

#include <glib-object.h>
#include "osm-gps-map-osd.h"

G_BEGIN_DECLS

#define OSM_TYPE_GPS_MAP_OSD_CLASSIC            (osm_gps_map_osd_classic_get_type())
#define OSM_GPS_MAP_OSD_CLASSIC(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj),  OSM_TYPE_GPS_MAP_OSD_CLASSIC, OsmGpsMapOsdClassic))
#define OSM_GPS_MAP_OSD_CLASSIC_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),   OSM_TYPE_GPS_MAP_OSD_CLASSIC, OsmGpsMapOsdClassicClass))
#define OSM_IS_GPS_MAP_OSD_CLASSIC(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj),  OSM_TYPE_GPS_MAP_OSD_CLASSIC))
#define OSM_IS_GPS_MAP_OSD_CLASSIC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),   OSM_TYPE_GPS_MAP_OSD_CLASSIC))
#define OSM_GPS_MAP_OSD_CLASSIC_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   OSM_TYPE_GPS_MAP_OSD_CLASSIC, OsmGpsMapOsdClassicClass))

typedef struct _OsmGpsMapOsdClassic        OsmGpsMapOsdClassic;
typedef struct _OsmGpsMapOsdClassicClass   OsmGpsMapOsdClassicClass;
typedef struct _OsmGpsMapOsdClassicPrivate OsmGpsMapOsdClassicPrivate;

struct _OsmGpsMapOsdClassic
{
	OsmGpsMapOsd parent;

	/*< private >*/
	OsmGpsMapOsdClassicPrivate *priv;
};

struct _OsmGpsMapOsdClassicClass
{
	OsmGpsMapOsdClass parent_class;

	/* vtable */
	
};

GType                osm_gps_map_osd_classic_get_type (void);
OsmGpsMapOsdClassic* osm_gps_map_osd_classic_new      (void);

G_END_DECLS

#endif /* __OSM_GPS_MAP_OSD_CLASSIC_H__ */
