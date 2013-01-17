/* GTK - The GIMP Toolkit
 * Copyright (C) 2012, One Laptop Per Child.
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author(s): Carlos Garnacho <carlos@lanedo.com>
 */
#include <math.h>
#include <gtk/gtkgesturerotate.h>
#include "gtkmarshalers.h"

enum {
  ANGLE_CHANGED,
  LAST_SIGNAL
};

struct _GtkGestureRotatePriv
{
  gdouble initial_angle;
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (GtkGestureRotate, gtk_gesture_rotate, GTK_TYPE_GESTURE)

static void
gtk_gesture_rotate_init (GtkGestureRotate *gesture)
{
  gesture->_priv = G_TYPE_INSTANCE_GET_PRIVATE (gesture,
                                                GTK_TYPE_GESTURE_ROTATE,
                                                GtkGestureRotatePriv);
}

static GObject *
gtk_gesture_rotate_constructor (GType                  type,
                                guint                  n_construct_properties,
                                GObjectConstructParam *construct_properties)
{
  GObject *object;

  object = G_OBJECT_CLASS (gtk_gesture_rotate_parent_class)->constructor (type,
                                                                          n_construct_properties,
                                                                          construct_properties);
  g_object_set (object, "n-points", 2, NULL);

  return object;
}

static gboolean
_gtk_gesture_rotate_get_angle (GtkGestureRotate *rotate,
                               gdouble          *angle)
{
  gdouble x1, y1, x2, y2;
  GtkGesture *gesture;
  gdouble dx, dy;
  GList *sequences;

  gesture = GTK_GESTURE (rotate);

  if (!gtk_gesture_is_active (gesture))
    return FALSE;

  sequences = gtk_gesture_get_sequences (gesture);
  g_assert (sequences && sequences->next);

  gtk_gesture_get_point (gesture, sequences->data, &x1, &y1);
  gtk_gesture_get_point (gesture, sequences->next->data, &x2, &y2);
  g_list_free (sequences);

  dx = x1 - x2;
  dy = y1 - y2;

  *angle = atan2 (dx, dy);

  /* Invert angle */
  *angle = (2 * G_PI) - *angle;

  /* And constraint it to 0°-360° */
  *angle = fmod (*angle, 2 * G_PI);

  return TRUE;
}

static gboolean
_gtk_gesture_rotate_check_emit (GtkGestureRotate *gesture)
{
  GtkGestureRotatePriv *priv = gesture->_priv;
  gdouble angle;

  if (!_gtk_gesture_rotate_get_angle (gesture, &angle))
    return FALSE;

  g_signal_emit (gesture, signals[ANGLE_CHANGED], 0,
                 angle, angle - priv->initial_angle);
  return TRUE;
}

static void
gtk_gesture_rotate_begin (GtkGesture *gesture)
{
  GtkGestureRotate *rotate = GTK_GESTURE_ROTATE (gesture);
  GtkGestureRotatePriv *priv = rotate->_priv;

  _gtk_gesture_rotate_get_angle (rotate, &priv->initial_angle);
}

static void
gtk_gesture_rotate_update (GtkGesture       *gesture,
                           GdkEventSequence *sequence)
{
  _gtk_gesture_rotate_check_emit (GTK_GESTURE_ROTATE (gesture));
}

static void
gtk_gesture_rotate_class_init (GtkGestureRotateClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkGestureClass *gesture_class = GTK_GESTURE_CLASS (klass);

  object_class->constructor = gtk_gesture_rotate_constructor;

  gesture_class->begin = gtk_gesture_rotate_begin;
  gesture_class->update = gtk_gesture_rotate_update;

  /**
   * GtkGestureRotate::angle-changed:
   * @gesture: the object on which the signal is emitted
   * @angle: Current angle in radians
   * @angle_delta: Difference with the starting angle in radians
   */
  signals[ANGLE_CHANGED] =
    g_signal_new ("angle-changed",
                  GTK_TYPE_GESTURE_ROTATE,
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (GtkGestureRotateClass, angle_changed),
                  NULL, NULL,
                  _gtk_marshal_VOID__DOUBLE_DOUBLE,
                  G_TYPE_NONE, 2, G_TYPE_DOUBLE, G_TYPE_DOUBLE);

  g_type_class_add_private (klass, sizeof (GtkGestureRotatePriv));
}

/**
 * gtk_gesture_rotate_new:
 *
 * Returns a newly created #GtkGesture that recognizes 2-touch
 * rotation gestures.
 *
 * Returns: a newly created #GtkGestureRotate
 *
 * Since: 3.8
 **/
GtkGesture *
gtk_gesture_rotate_new (void)
{
  return g_object_new (GTK_TYPE_GESTURE_ROTATE, NULL);
}

/**
 * gtk_gesture_rotate_get_angle_delta:
 * @gesture: a #GtkGestureRotate
 * @delta: (out) (transfer none): angle delta
 *
 * If @gesture is active, this function returns %TRUE and fills
 * in @delta with the angle difference in radians since the
 * gesture was first recognized.
 *
 * Returns: %TRUE if @controller is recognizing a rotate gesture
 *
 * Since: 3.8
 **/
gboolean
gtk_gesture_rotate_get_angle_delta (GtkGestureRotate *gesture,
                                    gdouble          *delta)
{
  GtkGestureRotatePriv *priv;
  gdouble angle;

  g_return_val_if_fail (GTK_IS_GESTURE_ROTATE (gesture), FALSE);

  if (!_gtk_gesture_rotate_get_angle (gesture, &angle))
    return FALSE;

  priv = gesture->_priv;

  if (delta)
    *delta = angle - priv->initial_angle;

  return TRUE;
}
