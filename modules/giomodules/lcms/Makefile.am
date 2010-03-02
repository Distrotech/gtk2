include $(top_srcdir)/Makefile.decl

INCLUDES = \
	-I$(top_srcdir) 				\
	-I$(top_srcdir)/gdk				\
	-I$(top_builddir)/gdk				\
	-I$(top_srcdir)/gtk				\
	-I$(top_builddir)/gtk				\
	$(LCMS_CFLAGS)					\
	$(GTK_DEP_CFLAGS)				\
	$(GTK_DEBUG_FLAGS)

LDADDS = \
	$(top_builddir)/gtk/$(gtktargetlib)		\
	$(GTK_DEP_LIBS)

backenddir = $(libdir)/gtk-2.0/$(GTK_BINARY_VERSION)/giomodules

backend_LTLIBRARIES = libcolorenginelcms.la

libcolorenginelcms_la_SOURCES =				\
	gtkcolorenginelcms.c				\
	gtkcolorprofilelcms.c				\
	gtkcolortransformlcms.c

noinst_HEADERS =					\
	gtkcolorenginelcms.h				\
	gtkcolortransformlcms.h				\
	gtkcolorprofilelcms.h

libcolorenginelcms_la_LDFLAGS =  -avoid-version -module $(no_undefined) -export-symbols-regex '^g_io_module_(load|unload|query)'
libcolorenginelcms_la_LIBADD = $(LDADDS) $(LCMS_LIBS)

-include $(top_srcdir)/git.mk
