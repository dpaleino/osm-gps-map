AC_INIT(osm-gps-map, 0.6.0)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_CONFIG_SRCDIR(osmgpsmap.pc.in)
AM_CONFIG_HEADER(config.h)

AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC

AC_LIBTOOL_WIN32_DLL
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

AC_MSG_CHECKING([for Win32])
case "$host" in
  *-*-mingw*)
    os_win32=yes
    ;;
  *)
    os_win32=no
    ;;
esac
AC_MSG_RESULT([$os_win32])
AM_CONDITIONAL(OS_WIN32, [test $os_win32 = yes])

# Before making a release, the LT_VERSION_INFO string should be modified.
# The string is of the form C:R:A.
# - If interfaces have been changed or added, but binary compatibility has
#   been preserved, change to C+1:0:A+1
# - If binary compatibility has been broken (eg removed or changed interfaces)
#   change to C+1:0:0
# - If the interface is the same as the previous version, change to C:R+1:A
LT_VERSION_INFO=1:0:1
AC_SUBST(LT_VERSION_INFO)

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
