AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"xfce4-windowck-plugin\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# Windowck plugin
#
plugin_LTLIBRARIES = \
	libwindowck.la

plugindir = \
	$(libdir)/xfce4/panel/plugins

libwindowck_built_sources = \
	windowck-dialogs_ui.h
	
	
libwindowck_la_SOURCES = \
	$(libwindowck_built_sources) \
	windowck.c \
	windowck.h \
	windowck-dialogs.c \
	windowck-dialogs.h \
	windowck-title.h \
	windowck-title.c

libwindowck_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(LIBXFCE4PANEL_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(XFCONF_CFLAGS) \
	$(PLATFORM_CFLAGS)

libwindowck_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       -export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
       $(PLATFORM_LDFLAGS)

libwindowck_la_LIBADD = \
	$(top_builddir)/common/libwck-common.la \
	$(GTK_LIBS) \
	$(LIBXFCE4UTIL_LIBS) \
	$(LIBXFCE4UI_LIBS) \
	$(LIBXFCE4PANEL_LIBS) \
	$(LIBWNCK_LIBS) \
	$(XFCONF_LIBS)

libwindowck_la_DEPENDENCIES = \
	$(top_builddir)/common/libwck-common.la

#
# Desktop file
#
desktopdir =								\
	$(datadir)/xfce4/panel/plugins

desktop_DATA =								\
	windowck-plugin.desktop

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =								\
	windowck-dialogs.glade                                          \
	windowck-plugin.desktop.in

CLEANFILES =								\
	$(desktop_DATA)

BUILT_SOURCES = \
	$(libwindowck_built_sources)

CLEANFILES += \
	$(libwindowck_built_sources)

windowck-dialogs_ui.h: windowck-dialogs.glade
	$(AM_V_GEN) exo-csource --static --strip-comments --strip-content --name=windowck_dialogs_ui $< >$@
	
# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
