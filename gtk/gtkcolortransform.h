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

#ifndef __GTK_COLOR_TRANSFORM_H
#define __GTK_COLOR_TRANSFORM_H

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo.h>

#include "gtkcolorprofile.h"

G_BEGIN_DECLS

#define GTK_TYPE_COLOR_TRANSFORM             (gtk_color_transform_get_type ())
#define GTK_COLOR_TRANSFORM(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), GTK_TYPE_COLOR_TRANSFORM, GtkColorTransform))
#define GTK_COLOR_TRANSFORM_CLASS(k)         (G_TYPE_CHECK_CLASS_CAST((k), GTK_TYPE_COLOR_TRANSFORM, GtkColorTransformClass))
#define GTK_IS_COLOR_TRANSFORM(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), GTK_TYPE_COLOR_TRANSFORM))
#define GTK_IS_COLOR_TRANSFORM_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), GTK_TYPE_COLOR_TRANSFORM))
#define GTK_COLOR_TRANSFORM_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), GTK_TYPE_COLOR_TRANSFORM, GtkColorTransformClass))

typedef struct _GtkColorTransformPrivate       GtkColorTransformPrivate;
typedef struct _GtkColorTransform              GtkColorTransform;
typedef struct _GtkColorTransformClass         GtkColorTransformClass;

struct _GtkColorTransform
{
  GObject parent;
  GtkColorTransformPrivate *priv;
};

typedef enum {
  GTK_COLOR_INTENT_INVALID,
  GTK_COLOR_INTENT_PERCEPTUAL,
  GTK_COLOR_INTENT_RELATIVE_COLORMETRIC,
  GTK_COLOR_INTENT_SATURATION,
  GTK_COLOR_INTENT_ABSOLUTE_COLORMETRIC,
  GTK_COLOR_INTENT_LAST
} GtkColorIntent;

struct _GtkColorTransformClass
{
  GObjectClass parent_class;
  gboolean         (*apply_pixbuf_in_place)  (GtkColorTransform  *color_transform,
                                              GdkPixbuf          *pixbuf,
                                              GError            **error);
  GdkPixbuf       *(*apply_pixbuf)           (GtkColorTransform  *color_transform,
                                              GdkPixbuf          *pixbuf,
                                              GError            **error);
  gboolean         (*apply_surface_in_place) (GtkColorTransform  *color_transform,
                                              cairo_surface_t    *surface,
                                              GError            **error);
  cairo_surface_t *(*apply_surface)          (GtkColorTransform  *color_transform,
                                              cairo_surface_t    *surface,
                                              GError            **error);
  /* padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
  void (*_gtk_reserved5) (void);
};

GType            gtk_color_transform_get_type             (void) G_GNUC_CONST;
void             gtk_color_transform_set_input_profile    (GtkColorTransform    *color_transform,
                                                           GtkColorProfile      *profile);
void             gtk_color_transform_set_output_profile   (GtkColorTransform    *color_transform,
                                                           GtkColorProfile      *profile);
GtkColorProfile *gtk_color_transform_get_input_profile    (GtkColorTransform    *color_transform);
GtkColorProfile *gtk_color_transform_get_output_profile   (GtkColorTransform    *color_transform);

gboolean         gtk_color_transform_apply_pixbuf_in_place      (GtkColorTransform    *color_transform,
                                                                 GdkPixbuf            *pixbuf,
                                                                 GError              **error);
GdkPixbuf       *gtk_color_transform_apply_pixbuf               (GtkColorTransform    *color_transform,
                                                                 GdkPixbuf            *pixbuf,
                                                                 GError              **error);

gboolean         gtk_color_transform_apply_surface_in_place     (GtkColorTransform    *color_transform,
                                                                 cairo_surface_t      *surface,
                                                                 GError              **error);
cairo_surface_t *gtk_color_transform_apply_surface              (GtkColorTransform    *color_transform,
                                                                 cairo_surface_t      *surface,
                                                                 GError              **error);

GtkColorIntent   gtk_color_transform_get_intent                   (GtkColorTransform *color_transform);
void             gtk_color_transform_set_intent                   (GtkColorTransform *color_transform,
                                                                   GtkColorIntent    intent);
gboolean         gtk_color_transform_get_black_point_compensation (GtkColorTransform *color_transform);
void             gtk_color_transform_set_black_point_compensation (GtkColorTransform *color_transform,
                                                                   gboolean           black_point_compensation);

G_END_DECLS

#endif /* __GTK_COLOR_TRANSFORM_H */

