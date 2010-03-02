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
 * SECTION:gtk-color-profile
 * @short_description: Base class for implementing ICC color profiles
 * @include: gtk/gtk.h
 * @title: GtkColorProfile
 * @see_also: #GtkColorTransform, #GtkColorProfile
 *
 * #GtkColorProfile is a high level abstraction of a color profile. Typically
 * this will be an ICC profile loaded from disk, or embedded in a source image.
 * A color profile will allow you to change the colors of the image either to
 * correctly render the colors on a given output device, or to deliberatly
 * change the colors e.g. introducing a sepia effect on photographs.
 *
 * To construct a #GtkColorProfile, you have to get an instance of
 * #GtkColorEngine, and then call gtk_color_engine_create_profile().
 *
 * You cannot create "empty" #GtkColorProfile objects, you always have to
 * create them from ICC data.
 */

#include "config.h"

#include "gtkcolorprofile.h"

G_DEFINE_TYPE (GtkColorProfile, gtk_color_profile, G_TYPE_OBJECT)

static void
gtk_color_profile_class_init (GtkColorProfileClass *klass)
{
}

static void
gtk_color_profile_init (GtkColorProfile *color_profile)
{
}

#define __GTK_COLOR_PROFILE_C__
#include "gtkaliasdef.c"
