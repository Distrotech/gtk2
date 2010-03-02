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

#ifndef __GTK_COLOR_ENGINE_H
#define __GTK_COLOR_ENGINE_H

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "gtkcolorprofile.h"
#include "gtkcolortransform.h"

G_BEGIN_DECLS

#define GTK_TYPE_COLOR_ENGINE               (gtk_color_engine_get_type ())
#define GTK_COLOR_ENGINE(o)                 (G_TYPE_CHECK_INSTANCE_CAST ((o), GTK_TYPE_COLOR_ENGINE, GtkColorEngine))
#define GTK_IS_COLOR_ENGINE(o)              (G_TYPE_CHECK_INSTANCE_TYPE ((o), GTK_TYPE_COLOR_ENGINE))
#define GTK_COLOR_ENGINE_GET_IFACE(obj)     (G_TYPE_INSTANCE_GET_INTERFACE ((obj), GTK_TYPE_COLOR_ENGINE, GtkColorEngineInterface))

#define GTK_COLOR_ENGINE_ERROR              (gtk_color_engine_error_quark ())

typedef struct _GtkColorEngine            GtkColorEngine;
typedef struct _GtkColorEngineInterface   GtkColorEngineInterface;

typedef enum
{
  GTK_COLOR_ENGINE_ERROR_IMAGE_FORMAT_NOT_SUPPORTED
} GtkColorEngineError;

#define GTK_COLOR_ENGINE_EXTENSION_POINT_NAME "gtk-color-engine"

/**
 * GtkColorEngineInterface:
 * @g_iface: The parent interface.
 * @create_profile: Creates a #GtkColorProfile.
 * @create_transform: Creates a #GtkColorTransform.
 *
 * An interface for creating color profiles and transforms.
 **/
struct _GtkColorEngineInterface
{
  GTypeInterface g_iface;
  /* Virtual Table */
  GtkColorProfile     *(*create_profile)    (GtkColorEngine *color_engine,
                                             const guint8   *data,
                                             gsize           length,
                                             GError        **error);
  GtkColorTransform   *(*create_transform)  (GtkColorEngine *color_engine);

};

GQuark                  gtk_color_engine_error_quark        (void);
GType                   gtk_color_engine_get_type           (void) G_GNUC_CONST;
GtkColorEngine         *gtk_color_engine_get_default        (void);

/* new profile */
GtkColorProfile        *gtk_color_engine_create_profile (GtkColorEngine    *color_engine,
                                                         const guint8      *data,
                                                         gsize              length,
                                                         GError           **error);

/* new transform */
GtkColorTransform      *gtk_color_engine_create_transform                (GtkColorEngine    *color_engine);
GtkColorTransform      *gtk_color_engine_create_transform_from_profiles  (GtkColorEngine    *color_engine,
                                                                          GtkColorProfile   *input_profile,
                                                                          GtkColorProfile   *output_profile);

G_END_DECLS

#endif /* __GTK_COLOR_ENGINE_H */

