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

#ifndef __GTK_COLOR_ENGINE_LCMS_H
#define __GTK_COLOR_ENGINE_LCMS_H

#include <glib-object.h>
#include <lcms.h>

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_COLOR_ENGINE_LCMS          (gtk_color_engine_lcms_get_type ())
#define GTK_COLOR_ENGINE_LCMS(o)            (G_TYPE_CHECK_INSTANCE_CAST ((o), GTK_TYPE_COLOR_ENGINE_LCMS, GtkColorEngineLcms))
#define GTK_IS_COLOR_ENGINE_LCMS(o)         (G_TYPE_CHECK_INSTANCE_TYPE ((o), GTK_TYPE_COLOR_ENGINE_LCMS))

typedef struct _GtkColorEngineLcms        GtkColorEngineLcms;
typedef struct _GtkColorEngineLcmsClass   GtkColorEngineLcmsClass;

struct _GtkColorEngineLcms
{
  GObject parent;
  cmsHPROFILE srgb_profile;
};

struct _GtkColorEngineLcmsClass
{
  GObjectClass parent_class;
};

GType      gtk_color_engine_lcms_get_type    (void) G_GNUC_CONST;
void       gtk_color_engine_lcms_register    (GIOModule *module);

G_END_DECLS

#endif /* __GTK_COLOR_ENGINE_LCMS_H */

