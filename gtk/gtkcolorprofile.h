/* GTK - The GIMP Toolkit
 *
 * Copyright (C) 2010 Richard Hughes <richard@hughsie.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#if defined(GTK_DISABLE_SINGLE_INCLUDES) && !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
#error "Only <gtk/gtk.h> can be included directly."
#endif

#ifndef __GTK_COLOR_PROFILE_H
#define __GTK_COLOR_PROFILE_H

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

#define GTK_TYPE_COLOR_PROFILE               (gtk_color_profile_get_type ())
#define GTK_COLOR_PROFILE(o)                 (G_TYPE_CHECK_INSTANCE_CAST ((o), GTK_TYPE_COLOR_PROFILE, GtkColorProfile))
#define GTK_COLOR_PROFILE_CLASS(k)           (G_TYPE_CHECK_CLASS_CAST((k), GTK_TYPE_COLOR_PROFILE, GtkColorProfileClass))
#define GTK_IS_COLOR_PROFILE(o)              (G_TYPE_CHECK_INSTANCE_TYPE ((o), GTK_TYPE_COLOR_PROFILE))
#define GTK_IS_COLOR_PROFILE_CLASS(k)        (G_TYPE_CHECK_CLASS_TYPE ((k), GTK_TYPE_COLOR_PROFILE))
#define GTK_COLOR_PROFILE_GET_CLASS(o)       (G_TYPE_INSTANCE_GET_CLASS ((o), GTK_TYPE_COLOR_PROFILE, GtkColorProfileClass))

typedef struct _GtkColorProfilePrivate       GtkColorProfilePrivate;
typedef struct _GtkColorProfile              GtkColorProfile;
typedef struct _GtkColorProfileClass         GtkColorProfileClass;

struct _GtkColorProfile
{
  GObject parent;
  GtkColorProfilePrivate *priv;
};

struct _GtkColorProfileClass
{
  GObjectClass parent_class;
  /* padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
  void (*_gtk_reserved5) (void);
};

GType                    gtk_color_profile_get_type       (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __GTK_COLOR_PROFILE_H */

