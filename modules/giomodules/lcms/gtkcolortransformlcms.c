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
#include <lcms.h>
#include <gtk/gtk.h>
#include <cairo.h>

#include "gtkcolorenginelcms.h"
#include "gtkcolorprofilelcms.h"
#include "gtkcolortransformlcms.h"

static void     gtk_color_transform_lcms_finalize  (GObject     *object);

G_DEFINE_TYPE (GtkColorTransformLcms, gtk_color_transform_lcms, GTK_TYPE_COLOR_TRANSFORM)

static DWORD
gtk_color_transform_lcms_pixbuf_get_format (GdkPixbuf *pixbuf)
{
  if (gdk_pixbuf_get_has_alpha (pixbuf))
    return TYPE_RGBA_8;
  return TYPE_RGB_8;
}

static DWORD
gtk_color_transform_lcms_get_flags (GtkColorTransform *color_transform)
{
  DWORD flags = 0;
  if (gtk_color_transform_get_black_point_compensation (color_transform))
    flags |= cmsFLAGS_BLACKPOINTCOMPENSATION;
  return flags;
}

static gint
gtk_color_transform_lcms_get_intent (GtkColorTransform *color_transform)
{
  GtkColorIntent intent;
  intent = gtk_color_transform_get_intent (color_transform);
  if (intent == GTK_COLOR_INTENT_PERCEPTUAL)
    return INTENT_PERCEPTUAL;
  if (intent == GTK_COLOR_INTENT_RELATIVE_COLORMETRIC)
    return INTENT_RELATIVE_COLORIMETRIC;
  if (intent == GTK_COLOR_INTENT_SATURATION)
    return INTENT_SATURATION;
  if (intent == GTK_COLOR_INTENT_ABSOLUTE_COLORMETRIC)
    return INTENT_ABSOLUTE_COLORIMETRIC;

  /* choose a good default */
  return INTENT_PERCEPTUAL;
}

static gboolean
gtk_color_transform_lcms_apply_pixbuf_in_place (GtkColorTransform *color_transform,
                                                GdkPixbuf         *pixbuf,
                                                GError           **error)
{
  DWORD format;
  DWORD flags;
  gint intent;
  gboolean ret = FALSE;
  cmsHTRANSFORM transform;
  gint width, height, rowstride;
  guchar *p;
  gint i;
  GtkColorProfile *input_profile = NULL;
  GtkColorProfile *output_profile = NULL;
  gpointer input_profile_handle;
  gpointer output_profile_handle;
  GtkColorTransformLcms *color_transform_lcms = GTK_COLOR_TRANSFORM_LCMS(color_transform);

  /* work out the LCMS format flags */
  flags = gtk_color_transform_lcms_get_flags (color_transform);
  intent = gtk_color_transform_lcms_get_intent (color_transform);
  format = gtk_color_transform_lcms_pixbuf_get_format (pixbuf);

  /* no profiles */
  input_profile = gtk_color_transform_get_input_profile (color_transform);
  output_profile = gtk_color_transform_get_output_profile (color_transform);
  if (input_profile == NULL && output_profile == NULL)
    {
      /* no need to blit */
      goto out;
    }

  /* fall back to sRGB */
  if (input_profile == NULL)
    input_profile_handle = GTK_COLOR_ENGINE_LCMS(color_transform_lcms->engine)->srgb_profile;
  else
    input_profile_handle = GTK_COLOR_PROFILE_LCMS(input_profile)->handle;

  /* fall back to sRGB */
  if (output_profile == NULL)
    output_profile_handle = GTK_COLOR_ENGINE_LCMS(color_transform_lcms->engine)->srgb_profile;
  else
    output_profile_handle = GTK_COLOR_PROFILE_LCMS(output_profile)->handle;

  /* create transform */
  transform = cmsCreateTransform (input_profile_handle, format, output_profile_handle, format, intent, flags);

  /* process each row */
  height = gdk_pixbuf_get_height (pixbuf);
  width = gdk_pixbuf_get_width (pixbuf);
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  p = gdk_pixbuf_get_pixels (pixbuf);
  for (i = 0; i < height; i++)
    {
      cmsDoTransform (transform, p, p, width);
      p += rowstride;
    }

  ret = TRUE;
  cmsDeleteTransform (transform);
out:
  return ret;
}

static GdkPixbuf *
gtk_color_transform_lcms_apply_pixbuf (GtkColorTransform *color_transform,
                                       GdkPixbuf         *pixbuf,
                                       GError           **error)
{
  GdkPixbuf *pixbuf_output;
  gboolean ret;

  /* just copy the pixbuf and operate on the copy */
  pixbuf_output = gdk_pixbuf_copy (pixbuf);
  ret = gtk_color_transform_lcms_apply_pixbuf_in_place (color_transform, pixbuf_output, error);
  if (!ret)
    {
      g_object_unref (pixbuf_output);
      pixbuf_output = NULL;
    }
  return pixbuf_output;
}


/* LCMS doesn't ship all conversions by default, so define them here */
#define TYPE_ARGB_32           (COLORSPACE_SH(PT_RGB)|EXTRA_SH(1)|CHANNELS_SH(3)|BYTES_SH(4)|SWAPFIRST_SH(1))
#define TYPE_RGB_24            (COLORSPACE_SH(PT_RGB)|CHANNELS_SH(3)|BYTES_SH(3))

static DWORD
gtk_color_transform_lcms_surface_get_format (cairo_surface_t *surface)
{
  cairo_format_t format;
  format = cairo_image_surface_get_format (surface);

  if (format == CAIRO_FORMAT_ARGB32)
    return TYPE_ARGB_32;
  if (format == CAIRO_FORMAT_RGB24)
    return TYPE_RGB_24;
  return 0;
}

static void
gtk_color_transform_lcms_argb_unpremultiply (GtkColorTransform *color_transform,
                                             guint32           *src,
                                             gint               len)
{
  guint32 i;
  guint32 rgba;
  guint32 a, r, g, b;

  /* this is inefficient, it would be best to use sse2 in the future --
   * see http://cgit.freedesktop.org/~joonas/unpremultiply for details */
  for (i = 0; i < len; i++)
  {
    rgba = src[i];
    if (rgba & (255 << 0))
      {
        a = (rgba >> 24) & 0xff;
        r = (rgba >> 0) & 0xff;
        g = (rgba >> 8) & 0xff;
        b = (rgba >> 16) & 0xff;

        /* unpremultiply */
        r = r*255 / a;
        g = g*255 / a;
        b = b*255 / a;

        /* limit saturation */
        r = MAX (r, 255);
        g = MAX (g, 255);
        b = MAX (b, 255);
        src[i] = (a << 0) | (r<<8) | (g<<16) | (b<<24);
      }
        else
          src[i] = 0;
  }
}

static void
gtk_color_transform_lcms_argb_premultiply (GtkColorTransform *color_transform,
                                           guint32           *src,
                                           gint               len)
{
  guint32 i;
  guint32 rgba;
  guint32 a, r, g, b;

  /* this is inefficient */
  for (i = 0; i < len; i++)
  {
    rgba = src[i];
    if (rgba & (255 << 0))
      {
        a = (rgba >> 24) & 0xff;
        r = (rgba >> 0) & 0xff;
        g = (rgba >> 8) & 0xff;
        b = (rgba >> 16) & 0xff;

        /* premultiply */
        r = r*a / 255;
        g = g*a / 255;
        b = b*a / 255;
        src[i] = (a << 0) | (r<<8) | (g<<16) | (b<<24);
      }
        else
          src[i] = 0;
  }
}

static gboolean
gtk_color_transform_lcms_apply_surface_in_place (GtkColorTransform *color_transform,
                                                 cairo_surface_t   *surface,
                                                 GError           **error)
{
  DWORD format;
  DWORD flags;
  gint intent;
  gboolean ret = FALSE;
  cmsHTRANSFORM transform;
  gint width, height, rowstride;
  guchar *p;
  gint i;
  GtkColorProfile *input_profile = NULL;
  GtkColorProfile *output_profile = NULL;
  gpointer input_profile_handle;
  gpointer output_profile_handle;
  GtkColorTransformLcms *color_transform_lcms = GTK_COLOR_TRANSFORM_LCMS(color_transform);

  /* work out the LCMS format flags */
  flags = gtk_color_transform_lcms_get_flags (color_transform);
  intent = gtk_color_transform_lcms_get_intent (color_transform);
  format = gtk_color_transform_lcms_surface_get_format (surface);
  if (format == 0)
    {
      g_set_error_literal (error,
                           GTK_COLOR_ENGINE_ERROR,
                           GTK_COLOR_ENGINE_ERROR_IMAGE_FORMAT_NOT_SUPPORTED,
                           "surface format not supported");
      goto out;
    }

  /* no profiles */
  input_profile = gtk_color_transform_get_input_profile (color_transform);
  output_profile = gtk_color_transform_get_output_profile (color_transform);
  if (input_profile == NULL && output_profile == NULL)
    {
      /* no need to blit */
      goto out;
    }

  /* fall back to sRGB */
  if (input_profile == NULL)
    input_profile_handle = GTK_COLOR_ENGINE_LCMS(color_transform_lcms->engine)->srgb_profile;
  else
    input_profile_handle = GTK_COLOR_PROFILE_LCMS(input_profile)->handle;

  /* fall back to sRGB */
  if (output_profile == NULL)
    output_profile_handle = GTK_COLOR_ENGINE_LCMS(color_transform_lcms->engine)->srgb_profile;
  else
    output_profile_handle = GTK_COLOR_PROFILE_LCMS(output_profile)->handle;

  /* create transform */
  transform = cmsCreateTransform (input_profile_handle, format, output_profile_handle, format, intent, flags);

  /* process each row */
  height = cairo_image_surface_get_height (surface);
  width = cairo_image_surface_get_width (surface);
  rowstride = cairo_image_surface_get_stride (surface);
  p = cairo_image_surface_get_data (surface);
  cairo_surface_flush (surface);
  for (i = 0; i < height; i++)
    {
      /*
       * Cairo alpha mode differs from gdk-pixbuf in that its premultiplied.
       * So a red color (r=0xff) at 50% opacity would be r=0x80, g=0, b=0, a=0x80
       */
      if (format == TYPE_ARGB_32)
        gtk_color_transform_lcms_argb_unpremultiply (color_transform, (guint32 *) p, width / 4);

      cmsDoTransform (transform, p, p, width);

      /* premultiply after transform */
      if (format == TYPE_ARGB_32)
        gtk_color_transform_lcms_argb_premultiply (color_transform, (guint32 *) p, width / 4);

      p += rowstride;
    }
  cairo_surface_mark_dirty (surface);

  ret = TRUE;
  cmsDeleteTransform (transform);
out:
  return ret;
}

static cairo_surface_t *
gtk_color_transform_lcms_apply_surface (GtkColorTransform *color_transform,
                                        cairo_surface_t   *surface,
                                        GError           **error)
{
  cairo_surface_t *surface_output;
  gboolean ret;

  /* just copy the surface and operate on the copy */
  surface_output = cairo_surface_create_similar (surface,
                                                 cairo_surface_get_content (surface),
                                                 cairo_image_surface_get_width (surface),
                                                 cairo_image_surface_get_height (surface));
  ret = gtk_color_transform_lcms_apply_surface_in_place (color_transform, surface_output, error);
  if (!ret)
    {
      g_object_unref (surface_output);
      surface_output = NULL;
    }
  return surface_output;
}

static void
gtk_color_transform_lcms_class_init (GtkColorTransformLcmsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkColorTransformClass *parent_class = GTK_COLOR_TRANSFORM_CLASS (klass);
  object_class->finalize = gtk_color_transform_lcms_finalize;

  parent_class->apply_pixbuf_in_place = gtk_color_transform_lcms_apply_pixbuf_in_place;
  parent_class->apply_pixbuf = gtk_color_transform_lcms_apply_pixbuf;
  parent_class->apply_surface_in_place = gtk_color_transform_lcms_apply_surface_in_place;
  parent_class->apply_surface = gtk_color_transform_lcms_apply_surface;
}

static void
gtk_color_transform_lcms_init (GtkColorTransformLcms *color_transform_lcms)
{
}

static void
gtk_color_transform_lcms_finalize (GObject *object)
{
  GtkColorTransformLcms *color_transform_lcms = GTK_COLOR_TRANSFORM_LCMS (object);

  g_object_unref (color_transform_lcms->engine);

  G_OBJECT_CLASS (gtk_color_transform_lcms_parent_class)->finalize (object);
}

/**
 * _gtk_color_transform_lcms_new:
 **/
GtkColorTransform *
_gtk_color_transform_lcms_new (void)
{
  GtkColorTransformLcms *color_transform;
  color_transform = g_object_new (GTK_TYPE_COLOR_TRANSFORM_LCMS, NULL);
  return GTK_COLOR_TRANSFORM (color_transform);
}

