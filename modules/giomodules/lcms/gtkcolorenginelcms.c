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

#include "gtkcolorenginelcms.h"
#include "gtkcolorprofilelcms.h"
#include "gtkcolortransformlcms.h"

static void gtk_color_engine_lcms_iface_init (GtkColorEngineInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (GtkColorEngineLcms,
                                gtk_color_engine_lcms,
                                G_TYPE_OBJECT,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (GTK_TYPE_COLOR_ENGINE,
                                                               gtk_color_engine_lcms_iface_init))

static GtkColorProfile *
_gtk_color_engine_lcms_create_profile (GtkColorEngine *color_engine,
                                       const guint8   *data,
                                       gsize           length,
                                       GError        **error)
{
  GtkColorProfile *color_profile;
  gboolean ret;
  color_profile = _gtk_color_profile_lcms_new ();
  GTK_COLOR_PROFILE_LCMS(color_profile)->engine = g_object_ref (color_engine);
  ret = _gtk_color_profile_lcms_set_from_data (color_profile, data, length, error);
  if (!ret)
    {
      g_object_unref (color_profile);
      color_profile = NULL;
    }
  return color_profile;
}

static GtkColorTransform *
_gtk_color_engine_lcms_create_transform (GtkColorEngine *color_engine)
{
  GtkColorTransform *color_transform;
  color_transform = _gtk_color_transform_lcms_new ();
  GTK_COLOR_TRANSFORM_LCMS(color_transform)->engine = g_object_ref (color_engine);
  return color_transform;
}

static void
gtk_color_engine_lcms_iface_init (GtkColorEngineInterface *iface)
{
  iface->create_profile = _gtk_color_engine_lcms_create_profile;
  iface->create_transform = _gtk_color_engine_lcms_create_transform;
}

static int
gtk_color_engine_lcms_error_cb (gint         error_code,
                                const gchar *error_text)
{
  g_warning ("LCMS error %i: %s", error_code, error_text);
  return LCMS_ERRC_WARNING;
}

static void
gtk_color_engine_lcms_library_init (void)
{
  static gboolean inited = FALSE;
  if (inited)
    return;

  cmsSetErrorHandler (gtk_color_engine_lcms_error_cb);
  cmsErrorAction (LCMS_ERROR_SHOW);

  /* this is the default fallback language */
  cmsSetLanguage ("en", "US");

  inited = TRUE;
}

static void
gtk_color_engine_lcms_init (GtkColorEngineLcms *color_engine_lcms)
{
  gtk_color_engine_lcms_library_init ();
  color_engine_lcms->srgb_profile = cmsCreate_sRGBProfile ();
}

static void
gtk_color_engine_lcms_class_init (GtkColorEngineLcmsClass *klass)
{
}

static void
gtk_color_engine_lcms_class_finalize (GtkColorEngineLcmsClass *klass)
{
}

/**
 * gtk_color_engine_lcms_register:
 **/
void
gtk_color_engine_lcms_register (GIOModule *module)
{
  gtk_color_engine_lcms_register_type (G_TYPE_MODULE (module));
  g_io_extension_point_implement (GTK_COLOR_ENGINE_EXTENSION_POINT_NAME,
                                  GTK_TYPE_COLOR_ENGINE_LCMS,
                                  "lcms",
                                  10);
}

/**
 * g_io_module_load:
 **/
void
g_io_module_load (GIOModule *module)
{
  gtk_color_engine_lcms_register (module);
}

/**
 * g_io_module_unload:
 **/
void
g_io_module_unload (GIOModule *module)
{
  GtkColorEngineLcms *color_engine_lcms = GTK_COLOR_ENGINE_LCMS (module);
  cmsCloseProfile (color_engine_lcms->srgb_profile);
}

