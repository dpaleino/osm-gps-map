AC_INIT(osm-gps-map, 0.5.0)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_CONFIG_SRCDIR(osmgpsmap.pc.in)
AM_CONFIG_HEADER(config.h)

AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC

AM_PROG_LIBTOOL

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

PKG_CHECK_MODULES(OPENSTREETMAP_GPS_MAP, [gtk+-2.0 glib-2.0 >= 2.18 cairo >= 1.8])

PKG_CHECK_MODULES(SOUP24, libsoup-2.4,
    soup_ver=2.4, [
        PKG_CHECK_MODULES(SOUP22, libsoup-2.2, [
            soup_ver=2.2
            AC_DEFINE(USE_LIBSOUP22, 1, Use libsoup 2.2 instead of libsoup-2.4 to download tiles)],
                AC_ERROR([libsoup-2.4 (preferred) or libsoup-2.2 not found]))])

OPENSTREETMAP_GPS_MAP_CFLAGS="$OPENSTREETMAP_GPS_MAP_CFLAGS $SOUP24_CFLAGS $SOUP22_CFLAGS"
OPENSTREETMAP_GPS_MAP_LIBS="$OPENSTREETMAP_GPS_MAP_LIBS $SOUP24_LIBS $SOUP22_LIBS"

AC_SUBST(OPENSTREETMAP_GPS_MAP_CFLAGS)
AC_SUBST(OPENSTREETMAP_GPS_MAP_LIBS)

CFLAGS="$CFLAGS -Wall -Wswitch-enum"

AC_OUTPUT([
osmgpsmap.pc
Makefile
src/Makefile
])

echo
echo $PACKAGE v$VERSION
echo
echo Prefix............... : $prefix
echo libsoup.............. : $soup_ver
echo
