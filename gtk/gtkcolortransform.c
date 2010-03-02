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
 * SECTION:gtk-color-transform
 * @short_description: Base class for implementing ICC color transforms
 * @include: gtk/gtk.h
 * @title: GtkColorTransform
 * @see_also: #GtkColorProfile, #GtkColorEngine
 *
 * #GtkColorTransform represents an object which can apply an ICC color
 * transformation using one or two #GtkColorProfile objects.
 *
 * A #GtkColorTransform can tranform a #GdkPixbuf or #cairo_surface_t object
 * from one colorspace to another, and can apply color corrections using ICC
 * profiles.
 *
 * If a profile is not specified, e.g.
 * |[
 *  gtk_color_transform_set_output_profile(transform, NULL);
 * ]|
 * Then the output profile will use sRGB, the default built-in colorspace.
 *
 * The actual method used to map one gamut to another gamut is specified by the
 * #GtkColorIntent. Usually this should be set to %GTK_COLOR_INTENT_PERCEPTUAL
 * (which is also the default) unless full saturation colors are required.
 */

#include "config.h"
#include <gdk/gdk.h>

#include "gtkcolortransform.h"
#include "gtkintl.h"
#include "gtktypebuiltins.h"

static void gtk_color_transform_finalize (GObject * object);

#define GTK_COLOR_TRANSFORM_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GTK_TYPE_COLOR_TRANSFORM, GtkColorTransformPrivate))

/**
 * GtkColorTransformPrivate:
 *
 * Private #GtkColorTransform data
 **/
struct _GtkColorTransformPrivate
{
  GtkColorIntent     intent;
  gboolean           black_point_compensation;
  GtkColorProfile   *input_profile;
  GtkColorProfile   *output_profile;
};

enum
{
  PROP_0,
  PROP_INTENT,
  PROP_BLACK_POINT_COMPENSATION,
  PROP_INPUT_PROFILE,
  PROP_OUTPUT_PROFILE,
  PROP_LAST
};

G_DEFINE_TYPE (GtkColorTransform, gtk_color_transform, G_TYPE_OBJECT)

/**
 * gtk_color_transform_get_intent:
 * @color_transform: a #GtkColorTransform
 *
 * Gets the rendering intent to use for this transform.
 *
 * Return value: a #GtkColorIntent
 *
 * Since: 2.20
 **/
GtkColorIntent
gtk_color_transform_get_intent (GtkColorTransform *color_transform)
{
  g_return_val_if_fail (GTK_IS_COLOR_TRANSFORM (color_transform), GTK_COLOR_INTENT_INVALID);
  return color_transform->priv->intent;
}

/**
 * gtk_color_transform_set_intent:
 * @color_transform: a #GtkColorTransform
 * @intent: a #GtkColorIntent
 *
 * Sets the rendering intent to use for this transform.
 *
 * Since: 2.20
 **/
void
gtk_color_transform_set_intent (GtkColorTransform *color_transform,
                                GtkColorIntent     intent)
{
  g_return_if_fail (GTK_IS_COLOR_TRANSFORM (color_transform));
  color_transform->priv->intent = intent;
}

/**
 * gtk_color_transform_get_black_point_compensation:
 * @color_transform: a #GtkColorTransform
 *
 * Gets the black point compensation setting for this transform.
 *
 * Return value: %TRUE if black point compensation is enabled
 *
 * Since: 2.20
 **/
gboolean
gtk_color_transform_get_black_point_compensation (GtkColorTransform *color_transform)
{
  g_return_val_if_fail (GTK_IS_COLOR_TRANSFORM (color_transform), FALSE);
  return color_transform->priv->black_point_compensation;
}

/**
 * gtk_color_transform_set_black_point_compensation:
 * @color_transform: a #GtkColorTransform
 * @black_point_compensation: if black point compensation should be used
 *
 * Sets the black point compensation setting for this transform.
 *
 * Black point compensation (BPC) is a way of changing the print density to be
 * more rounded where the ink density is nearly at the maximum.
 * This means tonal detail is still present where density is greater than the
 * theoretical maximum. Some tones are lightened, but black level is preserved.
 *
 * Black point compensation is always on when using perceptual rendering intent.
 *
 * Since: 2.20
 **/
void
gtk_color_transform_set_black_point_compensation (GtkColorTransform *color_transform,
                                                  gboolean           black_point_compensation)
{
  g_return_if_fail (GTK_IS_COLOR_TRANSFORM (color_transform));
  color_transform->priv->black_point_compensation = black_point_compensation;
}

/**
 * gtk_color_transform_set_input_profile:
 * @color_transform: a #GtkColorTransform
 * @profile: a #GtkColorProfile, or %NULL
 *
 * Sets an input profile to be used in this transform.
 * If %NULL is used as the @profile then the internal sRGB space is used.
 *
 * Since: 2.20
 **/
void
gtk_color_transform_set_input_profile (GtkColorTransform *color_transform,
                                       GtkColorProfile   *profile)
{
  GtkColorTransformPrivate *priv = color_transform->priv;

  g_return_if_fail (GTK_IS_COLOR_TRANSFORM (color_transform));
  g_return_if_fail (GTK_IS_COLOR_PROFILE (profile) || profile == NULL);

  /* close old profile */
  if (priv->input_profile)
    {
      g_object_unref (priv->input_profile);
      priv->input_profile = NULL;
    }

  /* set new profile */
  if (profile != NULL)
    priv->input_profile = g_object_ref (profile);

  g_object_notify (G_OBJECT (color_transform), "input-profile");
}

/**
 * gtk_color_transform_set_output_profile:
 * @color_transform: a #GtkColorTransform
 * @profile: a #GtkColorProfile, or %NULL
 *
 * Sets an output profile to be used in this transform.
 * If %NULL is used as the @profile then the internal sRGB space is used.
 *
 * Since: 2.20
 **/
void
gtk_color_transform_set_output_profile (GtkColorTransform *color_transform,
                                        GtkColorProfile   *profile)
{
  GtkColorTransformPrivate *priv = color_transform->priv;

  g_return_if_fail (GTK_IS_COLOR_TRANSFORM (color_transform));
  g_return_if_fail (GTK_IS_COLOR_PROFILE (profile) || profile == NULL);

  /* close old profile */
  if (priv->output_profile)
    {
      g_object_unref (priv->output_profile);
      priv->output_profile = NULL;
    }

  /* set new profile */
  if (profile != NULL)
    priv->output_profile = g_object_ref (profile);

  g_object_notify (G_OBJECT (color_transform), "output-profile");
}

/**
 * gtk_color_transform_set_input_profile:
 * @color_transform: a #GtkColorTransform
 *
 * Returns the input profile to be used in this transform.
 *
 * Return value: the #GtkColorProfile, or %NULL if there is no profile set
 *
 * Since: 2.20
 **/
GtkColorProfile *
gtk_color_transform_get_input_profile (GtkColorTransform *color_transform)
{
  GtkColorTransformPrivate *priv = color_transform->priv;

  g_return_val_if_fail (GTK_IS_COLOR_TRANSFORM (color_transform), NULL);

  if (priv->input_profile == NULL)
    return NULL;
  return priv->input_profile;
}

/**
 * gtk_color_transform_set_output_profile:
 * @color_transform: a #GtkColorTransform
 *
 * Returns the output profile to be used in this transform.
 *
 * Return value: the #GtkColorProfile, or %NULL if there is no profile set
 *
 * Since: 2.20
 **/
GtkColorProfile *
gtk_color_transform_get_output_profile (GtkColorTransform *color_transform)
{
  GtkColorTransformPrivate *priv = color_transform->priv;

  g_return_val_if_fail (GTK_IS_COLOR_TRANSFORM (color_transform), NULL);

  if (priv->output_profile == NULL)
    return NULL;
  return priv->output_profile;
}

/**
 * gtk_color_transform_apply_pixbuf_in_place:
 * @color_transform: a #GtkColorTransform
 * @pixbuf: a #GdkPixbuf
 * @error: a %GError, or %NULL
 *
 * Transform the pixmap in-place, destroying the old content.
 *
 * This function will be faster than gtk_color_transform_apply_pixbuf(), as a
 * new pixmap does not have to be allocated, and the conversion can be done
 * one row at a time.
 *
 * If the input profile or output profile have not been set then these are
 * assumed to be sRGB.
 *
 * Return value: %TRUE if the pixbuf was transformed, otherwise %FALSE and
 * @error is set.
 *
 * Since: 2.20
 **/
gboolean
gtk_color_transform_apply_pixbuf_in_place (GtkColorTransform *color_transform,
                                           GdkPixbuf         *pixbuf,
                                           GError           **error)
{
  gboolean ret;
  GtkColorTransformClass *klass = GTK_COLOR_TRANSFORM_GET_CLASS (color_transform);

  g_return_val_if_fail (pixbuf != NULL, FALSE);
  g_return_val_if_fail (klass->apply_pixbuf_in_place != NULL, FALSE);

  ret = klass->apply_pixbuf_in_place (color_transform,
                                      pixbuf,
                                      error);
  return ret;
}

/**
 * gtk_color_transform_apply_pixbuf:
 * @color_transform: a #GtkColorTransform
 * @pixbuf_in: the source #GdkPixbuf
 * @pixbuf_out: the destination #GdkPixbuf
 * @error: a %GError, or %NULL
 *
 * Transform from one pixmap to another.
 *
 * This function will be slower than gtk_color_transform_apply_pixbuf_in_place(),
 * as a new pixmap will have to be allocated by your application.
 *
 * If the input profile or output profile have not been set then these are
 * assumed to be sRGB.
 *
 * Return value: a new #GdkPixbuf is the pixbuf was transformed, otherwise %NULL
 * and @error is set.
 *
 * Since: 2.20
 **/
GdkPixbuf *
gtk_color_transform_apply_pixbuf (GtkColorTransform *color_transform,
                           GdkPixbuf         *pixbuf,
                           GError           **error)
{
  GdkPixbuf *pixbuf_output;
  GtkColorTransformClass *klass = GTK_COLOR_TRANSFORM_GET_CLASS (color_transform);

  g_return_val_if_fail (pixbuf != NULL, NULL);
  g_return_val_if_fail (klass->apply_pixbuf != NULL, NULL);

  pixbuf_output = klass->apply_pixbuf (color_transform,
                                      pixbuf,
                                      error);
  return pixbuf_output;
}

/**
 * gtk_color_transform_apply_surface_in_place:
 * @color_transform: a #GtkColorTransform
 * @surface: a #cairo_surface_t
 * @error: a %GError, or %NULL
 *
 * Transform the surface in-place, destroying the old content.
 *
 * This function will be faster than gtk_color_transform_apply_surface(), as a
 * new surface does not have to be allocated, and the conversion can be done
 * one row at a time.
 *
 * If the input profile or output profile have not been set then these are
 * assumed to be sRGB.
 *
 * Return value: %TRUE if the surface was transformed, otherwise %FALSE and
 * @error is set.
 *
 * Since: 2.20
 **/
gboolean
gtk_color_transform_apply_surface_in_place (GtkColorTransform *color_transform,
                                            cairo_surface_t   *surface,
                                            GError           **error)
{
  gboolean ret;
  GtkColorTransformClass *klass = GTK_COLOR_TRANSFORM_GET_CLASS (color_transform);

  g_return_val_if_fail (surface != NULL, FALSE);
  g_return_val_if_fail (klass->apply_surface_in_place != NULL, FALSE);

  ret = klass->apply_surface_in_place (color_transform,
                                       surface,
                                       error);
  return ret;
}

/**
 * gtk_color_transform_apply_surface:
 * @color_transform: a #GtkColorTransform
 * @surface_in: the source #cairo_surface_t
 * @surface_out: the destination #cairo_surface_t
 * @error: a %GError, or %NULL
 *
 * Transform from one surface to another.
 *
 * This function will be slower than gtk_color_transform_apply_surface_in_place(),
 * as a new surface will have to be allocated by your application.
 *
 * If the input profile or output profile have not been set then these are
 * assumed to be sRGB.
 *
 * Return value: a new #cairo_surface_t is the surface was transformed, otherwise %NULL
 * and @error is set.
 *
 * Since: 2.20
 **/
cairo_surface_t *
gtk_color_transform_apply_surface (GtkColorTransform *color_transform,
                                   cairo_surface_t   *surface,
                                   GError           **error)
{
  cairo_surface_t *surface_output;
  GtkColorTransformClass *klass = GTK_COLOR_TRANSFORM_GET_CLASS (color_transform);

  g_return_val_if_fail (surface != NULL, NULL);
  g_return_val_if_fail (klass->apply_surface != NULL, NULL);

  surface_output = klass->apply_surface (color_transform,
                                         surface,
                                         error);
  return surface_output;
}

static void
gtk_color_transform_get_property (GObject *object, guint prop_id,
                                  GValue *value, GParamSpec *pspec)
{
  GtkColorTransform *color_transform = GTK_COLOR_TRANSFORM (object);
  GtkColorTransformPrivate *priv = color_transform->priv;

  switch (prop_id)
    {
    case PROP_INTENT:
      g_value_set_enum (value, priv->intent);
      break;
    case PROP_BLACK_POINT_COMPENSATION:
      g_value_set_boolean (value, priv->black_point_compensation);
      break;
    case PROP_INPUT_PROFILE:
      g_value_set_object (value, priv->input_profile);
      break;
    case PROP_OUTPUT_PROFILE:
      g_value_set_object (value, priv->output_profile);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gtk_color_transform_set_property (GObject *object, guint prop_id,
                                  const GValue *value, GParamSpec *pspec)
{
  GtkColorTransform *color_transform = GTK_COLOR_TRANSFORM (object);
  GtkColorTransformPrivate *priv = color_transform->priv;

  switch (prop_id)
    {
    case PROP_INTENT:
      priv->intent = g_value_get_enum (value);
      break;
    case PROP_BLACK_POINT_COMPENSATION:
      priv->black_point_compensation = g_value_get_boolean (value);
      break;
    case PROP_INPUT_PROFILE:
      gtk_color_transform_set_input_profile (color_transform, g_value_get_object (value));
      break;
    case PROP_OUTPUT_PROFILE:
      gtk_color_transform_set_output_profile (color_transform, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gtk_color_transform_class_init (GtkColorTransformClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = gtk_color_transform_finalize;
  object_class->get_property = gtk_color_transform_get_property;
  object_class->set_property = gtk_color_transform_set_property;

  /**
    * GtkColorTransform:intent:
    *
    * The rendering intent used when applying a transform.
    * If this is not set, it defaults to %GTK_COLOR_INTENT_PERCEPTUAL.
    *
    * Since: 2.20
    */
  g_object_class_install_property (object_class,
                                   PROP_INTENT,
                                   g_param_spec_enum ("intent",
                                                       P_("Rendering intent"),
                                                       P_("Which rendering intent that should be used"),
                                                       GTK_TYPE_COLOR_INTENT,
                                                       GTK_COLOR_INTENT_PERCEPTUAL,
                                                       G_PARAM_READWRITE));

  /**
   * GtkColorTransform:black-point-compensation:
    *
    * Whether to use the black point compensation black smoothing funtionality.
    * If this is not set, it defaults to %FALSE.
    *
    * Since: 2.20
   */
  g_object_class_install_property (object_class,
                                   PROP_BLACK_POINT_COMPENSATION,
                                   g_param_spec_boolean ("black-point-compensation",
                                                         P_("Black point compensation"),
                                                         P_("If to use a compensating algorithm to preserve black"),
                                                         FALSE,
                                                         G_PARAM_READWRITE));

  /**
   * GtkColorTransform:input-profile:
    *
    * The #GtkColorProfile used as the input color profile.
    *
    * Since: 2.20
   */
  g_object_class_install_property (object_class,
                                   PROP_INPUT_PROFILE,
                                   g_param_spec_object ("input-profile",
                                                        P_("Input ICC profile"),
                                                        P_("The input ICC profile to use in the transform"),
                                                        GTK_TYPE_COLOR_PROFILE,
                                                        G_PARAM_READWRITE));

  /**
   * GtkColorTransform:output-profile:
    *
    * The #GtkColorProfile used as the output color profile.
    *
    * Since: 2.20
   */
  g_object_class_install_property (object_class,
                                   PROP_OUTPUT_PROFILE,
                                   g_param_spec_object ("output-profile",
                                                        P_("Output ICC profile"),
                                                        P_("The output ICC profile to use in the transform"),
                                                        GTK_TYPE_COLOR_PROFILE,
                                                        G_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (GtkColorTransformPrivate));
}

static void
gtk_color_transform_init (GtkColorTransform *color_transform)
{
  color_transform->priv = GTK_COLOR_TRANSFORM_GET_PRIVATE (color_transform);
  color_transform->priv->intent = GTK_COLOR_INTENT_PERCEPTUAL;
  color_transform->priv->black_point_compensation = FALSE;
  color_transform->priv->input_profile = NULL;
  color_transform->priv->output_profile = NULL;
}

static void
gtk_color_transform_finalize (GObject *object)
{
  GtkColorTransform *color_transform = GTK_COLOR_TRANSFORM (object);
  GtkColorTransformPrivate *priv = color_transform->priv;

  if (priv->input_profile != NULL)
    g_object_unref (priv->input_profile);
  if (priv->output_profile != NULL)
    g_object_unref (priv->output_profile);

  G_OBJECT_CLASS (gtk_color_transform_parent_class)->finalize (object);
}

#define __GTK_COLOR_TRANSFORM_C__
#include "gtkaliasdef.c"
