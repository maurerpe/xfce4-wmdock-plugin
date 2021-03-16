AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"xfce4-wmdock-plugin\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# Wmdock plugin
#
plugin_LTLIBRARIES = \
	libwmdock.la

plugindir = \
	$(libdir)/xfce4/panel/plugins

libwmdock_la_SOURCES = \
	tile.xpm \		  
	wmdock.c \
	wmdock.h \
	rcfile.c \
	rcfile.h \
	misc.c \
	misc.h \
	wmdock-dialogs.c \
	wmdock-dialogs.h

libwmdock_la_CFLAGS = \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(LIBXFCE4PANEL_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(PLATFORM_CFLAGS)

libwmdock_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       -export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
       $(PLATFORM_LDFLAGS)

libwmdock_la_LIBADD = \
	$(LIBXFCE4UTIL_LIBS) \
	$(LIBXFCE4UI_LIBS) \
	$(LIBXFCE4PANEL_LIBS) \
	$(LIBWNCK_LIBS)

#
# Desktop file
#
desktopdir =								\
	$(datadir)/xfce4/panel/plugins

desktop_DATA =								\
	wmdock.desktop

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =								\
	wmdock.desktop.in

CLEANFILES =								\
	$(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
