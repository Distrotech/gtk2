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

#include "gtkcolorprofilelcms.h"

static void     gtk_color_profile_lcms_finalize  (GObject     *object);

G_DEFINE_TYPE (GtkColorProfileLcms, gtk_color_profile_lcms, GTK_TYPE_COLOR_PROFILE)

/**
 * _gtk_color_profile_lcms_set_from_data:
 **/
gboolean
_gtk_color_profile_lcms_set_from_data (GtkColorProfile *color_profile,
                                       const guint8    *data,
                                       gsize            length,
                                       GError         **error)
{
  GtkColorProfileLcms *color_profile_lcms = GTK_COLOR_PROFILE_LCMS (color_profile);

  if (color_profile_lcms->handle != NULL)
    cmsCloseProfile (color_profile_lcms->handle);
  color_profile_lcms->handle = cmsOpenProfileFromMem ((LPVOID)data, length);
  return TRUE;
}

static void
gtk_color_profile_lcms_class_init (GtkColorProfileLcmsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = gtk_color_profile_lcms_finalize;
}

static void
gtk_color_profile_lcms_init (GtkColorProfileLcms *color_profile_lcms)
{
  color_profile_lcms->handle = NULL;
}

static void
gtk_color_profile_lcms_finalize (GObject *object)
{
  GtkColorProfileLcms *color_profile_lcms = GTK_COLOR_PROFILE_LCMS (object);

  if (color_profile_lcms->handle != NULL)
    cmsCloseProfile (color_profile_lcms->handle);
  g_object_unref (color_profile_lcms->engine);

  G_OBJECT_CLASS (gtk_color_profile_lcms_parent_class)->finalize (object);
}

/**
 * _gtk_color_profile_lcms_new:
 **/
GtkColorProfile *
_gtk_color_profile_lcms_new (void)
{
  GtkColorProfileLcms *color_profile;
  color_profile = g_object_new (GTK_TYPE_COLOR_PROFILE_LCMS, NULL);
  return GTK_COLOR_PROFILE (color_profile);
}

