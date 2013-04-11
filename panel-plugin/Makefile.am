INCLUDES = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"xfce4-windowck-plugin\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# Sample plugin
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
	windowck-utils.h \
	windowck-utils.c
	

libwindowck_la_CFLAGS = \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(LIBXFCE4PANEL_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(PLATFORM_CFLAGS)

libwindowck_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       -export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
       $(PLATFORM_LDFLAGS)

libwindowck_la_LIBADD = \
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
	xfce4-windowck-plugin.desktop

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =								\
	windowck-dialogs.glade                                          \
	xfce4-windowck-plugin.desktop.in

CLEANFILES =								\
	$(desktop_DATA)



BUILT_SOURCES = \
	$(libwindowck_built_sources)

CLEANFILES += \
	$(libwindowck_built_sources)

windowck-dialogs_ui.h: windowck-dialogs.glade
	$(AM_V_GEN) exo-csource --static --strip-comments --strip-content --name=windowck_dialogs_ui $< >$@
	
# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
