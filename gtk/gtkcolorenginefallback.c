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

#include "config.h"

#include <glib-object.h>
#include <gtk/gtk.h>
#include <string.h>

#include "gtkcolorenginefallback.h"

static void gtk_color_engine_fallback_iface_init (GtkColorEngineInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GtkColorEngineFallback, gtk_color_engine_fallback, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_COLOR_ENGINE,
                                                gtk_color_engine_fallback_iface_init))

G_DEFINE_TYPE (GtkColorProfileFallback, gtk_color_profile_fallback, GTK_TYPE_COLOR_PROFILE)
G_DEFINE_TYPE (GtkColorTransformFallback, gtk_color_transform_fallback, GTK_TYPE_COLOR_TRANSFORM)

static void
gtk_color_engine_fallback_init (GtkColorEngineFallback *color_engine_fallback)
{
}

static void
gtk_color_engine_fallback_class_init (GtkColorEngineFallbackClass *klass)
{
}

static GtkColorTransform *
gtk_color_transform_fallback_create_profile (GtkColorEngine *color_engine)
{
  GtkColorTransformFallback *color_transform;
  color_transform = g_object_new (GTK_TYPE_COLOR_TRANSFORM_FALLBACK, NULL);
  return GTK_COLOR_TRANSFORM (color_transform);
}

/**
 * gtk_color_profile_fallback_create_profile:
 **/
GtkColorProfile *
gtk_color_profile_fallback_create_profile (GtkColorEngine *color_engine,
                                           const guint8   *data,
                                           gsize           length,
                                           GError        **error)
{
  GtkColorProfileFallback *color_profile;
  color_profile = g_object_new (GTK_TYPE_COLOR_PROFILE_FALLBACK, NULL);
  return GTK_COLOR_PROFILE (color_profile);
}

static void
gtk_color_engine_fallback_iface_init (GtkColorEngineInterface *iface)
{
  iface->create_profile = gtk_color_profile_fallback_create_profile;
  iface->create_transform = gtk_color_transform_fallback_create_profile;
}

static void
gtk_color_profile_fallback_class_init (GtkColorProfileFallbackClass *klass)
{
}

static void
gtk_color_profile_fallback_init (GtkColorProfileFallback *color_profile_fallback)
{
}

static GdkPixbuf *
gtk_color_transform_fallback_apply_pixbuf (GtkColorTransform *color_transform,
                                           GdkPixbuf         *pixbuf,
                                           GError           **error)
{
  return gdk_pixbuf_copy (pixbuf);
}

static gboolean
gtk_color_transform_fallback_apply_pixbuf_in_place (GtkColorTransform *color_transform,
                                                    GdkPixbuf         *pixbuf,
                                                    GError           **error)
{
  /* this is a NOP */
  return TRUE;
}

static cairo_surface_t *
gtk_color_transform_fallback_apply_surface (GtkColorTransform *color_transform,
                                            cairo_surface_t   *surface,
                                            GError           **error)
{
  return cairo_surface_create_similar (surface,
                                       cairo_surface_get_content (surface),
                                       cairo_image_surface_get_width (surface),
                                       cairo_image_surface_get_height (surface));
}

static gboolean
gtk_color_transform_fallback_apply_surface_in_place (GtkColorTransform *color_transform,
                                                     cairo_surface_t   *surface,
                                                     GError           **error)
{
  /* this is a NOP */
  return TRUE;
}

static void
gtk_color_transform_fallback_class_init (GtkColorTransformFallbackClass *klass)
{
  GtkColorTransformClass *parent_class = GTK_COLOR_TRANSFORM_CLASS (klass);
  parent_class->apply_pixbuf_in_place = gtk_color_transform_fallback_apply_pixbuf_in_place;
  parent_class->apply_pixbuf = gtk_color_transform_fallback_apply_pixbuf;
  parent_class->apply_surface_in_place = gtk_color_transform_fallback_apply_surface_in_place;
  parent_class->apply_surface = gtk_color_transform_fallback_apply_surface;
}

static void
gtk_color_transform_fallback_init (GtkColorTransformFallback *color_transform_fallback)
{
}

