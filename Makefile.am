## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src

osm_gps_mapdocdir = ${prefix}/doc/osm-gps-map
osm_gps_mapdoc_DATA =                           \
	README                                      \
	COPYING                                     \
	AUTHORS                                     \
	ChangeLog                                   \
	INSTALL                                     \
	NEWS

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = osmgpsmap.pc

EXTRA_DIST = $(openstreetmap_gps_mapdoc_DATA)   \
	osmgpsmap.pc.in                             \
	python/Makefile.am                          \
	python/Makefile.in                          \
	python/acinclude.m4                         \
	python/ChangeLog                            \
	python/osmgpsmap.defs                       \
	python/setup.py                             \
	python/aclocal.m4                           \
	python/configure                            \
	python/osmgpsmapmodule.c                    \
	python/mapviewer.py                         \
	python/AUTHORS                              \
	python/configure.ac                         \
	python/osmgpsmap.override                   \
	python/NEWS                                 \
	python/README

# ChangeLog generation from nautils
distclean-local:
	if test "$(srcdir)" = "."; then :; else \
		rm -f ChangeLog; \
	fi

ChangeLog:
	@echo Creating $@
	@if test -d "$(srcdir)/.git"; then \
	  (GIT_DIR=$(top_srcdir)/.git ./missing --run git log 0.4.. --stat -M -C --name-status  --date=short --no-color) | fmt --split-only > $@.tmp \
	  && mv -f $@.tmp $@ \
	  || ($(RM) $@.tmp; \
	      echo Failed to generate ChangeLog, your ChangeLog may be outdated >&2; \
	      (test -f $@ || echo git-log is required to generate this file >> $@)); \
	else \
	  test -f $@ || \
	  (echo A git checkout and git-log is required to generate ChangeLog >&2 && \
	  echo A git checkout and git-log is required to generate this file >> $@); \
	fi

release: dist
	scp @PACKAGE@-@VERSION@.tar.gz root@greenbirdsystems.com:/var/www/johnstowers.co.nz/files/osm-gps-map/

.PHONY: ChangeLog

