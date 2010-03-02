/* GTK - The GIMP Toolkit
 *
 * Copyright (C) 2010 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
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

/**
 * SECTION:gtk-color-engine
 * @short_description: Creates #GtkColorProfile and #GtkColorTransform objects
 * @include: gtk/gtk.h
 * @title: GtkColorEngine
 * @see_also: #GtkColorProfile, #GtkColorTransform
 *
 * Color management is an important part of a modern desktop experience.
 * It is basically the transformation of images between the color
 * representations of various devices, such as cameras, scanners, displays
 * and printers.
 *
 * The goal of a color management system is to obtain the best match across
 * these different devices.
 * This means that photgraphs taken on a digital camera should match as closely
 * as possible what is observed on the computer screen and what is printed out
 * on paper.
 *
 * Output devices may be capable of representing more or less colors than
 * the input device, and part of the color management system is to define a
 * rendering intent when mapping one device gamut to another.
 *
 * #GtkColorEngine is an object that manages the default color management system
 * (CMS) which may or may not be installed as plugins.
 * The #GtkColorEngine object automatically loads the default CMS the first time
 * that it is required.
 * If no CMM is available then a fallback is used, which does no color
 * correction to the image whatsoever.
 *
 * A #GtkColorEngine allows the user to create a #GtkColorProfile objects using
 * gtk_color_engine_create_profile(). Profiles are used to represent device
 * colorspaces.
 *
 * A #GtkColorEngine also allows creation of #GtkColorTransform objects by using
 * either gtk_color_engine_create_transform() or
 * gtk_color_engine_create_transform_from_profiles().
 * Transforms are used to represent the mapping of one colorspace to another.
 *
 * On way you can use #GtkColorEngine is the following code snippet, where a
 * #GtkImage is being converted from one defined colorspace into the internal
 * sRGB space.
 * |[
 *  GdkPixbuf *pixbuf;
 *  GtkColorEngine *engine;
 *  GtkColorProfile *profile;
 *  GtkColorTransform *transform;
 *  engine = gtk_color_engine_get_default ();
 *  profile = gtk_color_engine_create_profile (engine, icc_profile, len, error);
 *  transform = gtk_color_engine_create_transform_from_profiles (engine, profile, NULL);
 *  pixbuf = gtk_image_get_pixbuf (image);
 *  ret = gtk_color_transform_apply_pixbuf_in_place (transform, pixbuf, error);
 *  g_object_unref (transform);
 *  g_object_unref (profile);
 * ]|
 */

#include "config.h"

#include <glib-object.h>

#include "gtkprivate.h"
#include "gtkcolorengine.h"
#include "gtkcolorenginefallback.h"

G_DEFINE_INTERFACE (GtkColorEngine, gtk_color_engine, G_TYPE_OBJECT)

/**
 * gtk_color_engine_error_quark:
 *
 * Gets the error quark for #GtkColorEngine.
 *
 * Return value: an error quark
 *
 * Since: 2.20
 **/
GQuark
gtk_color_engine_error_quark (void)
{
  return g_quark_from_static_string ("gtk-color-error-quark");
}

static void
gtk_color_engine_default_init (GtkColorEngineInterface *iface)
{
}

/**
 * gtk_color_engine_create_transform:
 * @color_engine: a #GtkColorEngine
 *
 * Gets a new transform from the engine, which allows you to modify an image.
 * You will still need to call gtk_color_transform_set_input_profile() and/or
 * gtk_color_transform_set_output_profile() before the transform will be useful.
 *
 * Return value: a new #GtkColorTransform instance, free with g_object_unref()
 *
 * Since: 2.20
 **/
GtkColorTransform *
gtk_color_engine_create_transform (GtkColorEngine *color_engine)
{
  GtkColorEngineInterface *iface = GTK_COLOR_ENGINE_GET_IFACE (color_engine);

  g_return_val_if_fail (GTK_IS_COLOR_ENGINE (color_engine), NULL);
  g_return_val_if_fail ((*iface->create_transform) != NULL, NULL);

  return (*iface->create_transform) (color_engine);
}

/**
 * gtk_color_engine_create_transform_from_profiles:
 * @color_engine: a #GtkColorEngine
 * @input_profile: a #GtkColorProfile, or %NULL
 * @output_profile: a #GtkColorProfile, or %NULL
 *
 * Gets a new transform from the engine which is preset with two profiles.
 * This saves you calling gtk_color_engine_create_transform() and then
 * gtk_color_transform_set_input_profile() and/or
 * gtk_color_transform_set_output_profile().
 *
 * Return value: a new #GtkColorTransform instance, free with g_object_unref()
 *
 * Since: 2.20
 **/
GtkColorTransform *
gtk_color_engine_create_transform_from_profiles (GtkColorEngine  *color_engine,
                                                 GtkColorProfile *input_profile,
                                                 GtkColorProfile *output_profile)
{
  GtkColorTransform *color_transform;

  g_return_val_if_fail (GTK_IS_COLOR_ENGINE (color_engine), NULL);
  g_return_val_if_fail (GTK_IS_COLOR_PROFILE (input_profile) || input_profile == NULL, NULL);
  g_return_val_if_fail (GTK_IS_COLOR_PROFILE (output_profile) || output_profile == NULL, NULL);

  color_transform = gtk_color_engine_create_transform (color_engine);
  gtk_color_transform_set_input_profile (color_transform, input_profile);
  gtk_color_transform_set_output_profile (color_transform, output_profile);
  return color_transform;
}

/**
 * gtk_color_engine_create_profile:
 * @color_engine: a #GtkColorEngine
 * @data: the COLOR engine data
 * @length: the COLOR engine data
 * @error: a %GError, or %NULL
 *
 * Creates a profile and loads ICC data.
 *
 * Return value: a #GtkColorProfile object, otherwise %NULL and @error is set.
 *
 * Since: 2.20
 **/
GtkColorProfile *
gtk_color_engine_create_profile (GtkColorEngine *color_engine,
                                 const guint8   *data,
                                 gsize           length,
                                 GError        **error)
{
  GtkColorProfile *color_profile;
  GtkColorEngineInterface *iface = GTK_COLOR_ENGINE_GET_IFACE (color_engine);

  g_return_val_if_fail (GTK_IS_COLOR_ENGINE (color_engine), FALSE);
  g_return_val_if_fail (data != NULL, FALSE);
  g_return_val_if_fail ((*iface->create_profile) != NULL, NULL);

  color_profile = (*iface->create_profile) (color_engine, data, length, error);
  return color_profile;
}

static gpointer
gtk_color_engine_get_default_cb (gpointer data)
{
  GIOExtensionPoint *ep;
  GList *extensions;

  _gtk_io_modules_ensure_loaded ();
  ep = g_io_extension_point_lookup (GTK_COLOR_ENGINE_EXTENSION_POINT_NAME);
  extensions = g_io_extension_point_get_extensions (ep);
  if (extensions != NULL)
    return g_object_new (g_io_extension_get_type (extensions->data), NULL);
  return g_object_new (GTK_TYPE_COLOR_ENGINE_FALLBACK, NULL);
}

/**
 * gtk_color_engine_get_default:
 *
 * Gets the default color engine. If no engines are installed then a fallback
 * engine is used which just copies the image data without modification.
 *
 * Return value: a #GtkColorEngine
 *
 * Since: 2.20
 **/
GtkColorEngine *
gtk_color_engine_get_default (void)
{
  static GOnce once_init = G_ONCE_INIT;
  return g_once (&once_init, gtk_color_engine_get_default_cb, NULL);
}


#define __GTK_COLOR_ENGINE_C__
#include "gtkaliasdef.c"
