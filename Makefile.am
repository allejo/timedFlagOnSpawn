lib_LTLIBRARIES = timedFlagOnSpawn.la

timedFlagOnSpawn_la_SOURCES = timedFlagOnSpawn.cpp
timedFlagOnSpawn_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
timedFlagOnSpawn_la_LDFLAGS = -module -avoid-version -shared
timedFlagOnSpawn_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.timedFlagOnSpawn.txt \
	README.md \
	timedFlagOnSpawn.def \
	timedFlagOnSpawn.sln \
	timedFlagOnSpawn.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
