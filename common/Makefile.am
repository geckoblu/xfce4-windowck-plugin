
AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"libwck-common\" \
	-DLIBXFCE4PANEL_COMPILATION \
	-DHELPDIR=\"$(docdir)/html\" \
	$(PLATFORM_CPPFLAGS)

noinst_LTLIBRARIES = \
	libwck-common.la

libwck_common_la_SOURCES = \
	wck-utils.c \
	wck-utils.h

libwck_common_la_CFLAGS = \
	$(XFCONF_CFLAGS) \
	$(GTK_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(LIBXFCE4PANEL_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(PLATFORM_CFLAGS)

libwck_common_la_LDFLAGS = \
	-no-undefined \
	$(PLATFORM_LDFLAGS)

libwck_common_la_LIBADD = \
	$(XFCONF_LIBS) \
	$(GTK_LIBS) \
	$(LIBXFCE4UI_LIBS) \
	$(LIBXFCE4PANEL_LIBS) \
	$(LIBWNCK_LIBS)

EXTRA_DIST =

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
