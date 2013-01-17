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
#include "config.h"
#include <gtk/gtkgesture.h>
#include "gtkprivate.h"
#include "gtkmarshalers.h"
#include "gtkintl.h"

typedef struct _PointData PointData;

enum {
  PROP_N_POINTS = 1
};

enum {
  CHECK,
  BEGIN,
  END,
  UPDATE,
  N_SIGNALS
};

struct _PointData
{
  GdkPoint point;
  GdkEventSequence *sequence;
  guint32 evtime;
};

struct _GtkGesturePriv
{
  GHashTable *points;
  gint n_points;
  guint recognized : 1;
};

static guint signals[N_SIGNALS] = { 0 };

G_DEFINE_ABSTRACT_TYPE (GtkGesture, gtk_gesture, GTK_TYPE_EVENT_CONTROLLER)

static void
gtk_gesture_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  GtkGesturePriv *priv = GTK_GESTURE (object)->_priv;

  switch (prop_id)
    {
    case PROP_N_POINTS:
      g_value_set_int (value, priv->n_points);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtk_gesture_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  GtkGesturePriv *priv = GTK_GESTURE (object)->_priv;

  switch (prop_id)
    {
    case PROP_N_POINTS:
      priv->n_points = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtk_gesture_finalize (GObject *object)
{
  GtkGesturePriv *priv = GTK_GESTURE (object)->_priv;

  g_hash_table_destroy (priv->points);

  G_OBJECT_CLASS (gtk_gesture_parent_class)->finalize (object);
}

static gboolean
gtk_gesture_check_impl (GtkGesture *gesture)
{
  return TRUE;
}

static void
_gtk_gesture_set_recognized (GtkGesture *gesture,
                             gboolean    recognized)
{
  if (gesture->_priv->recognized == recognized)
    return;

  gesture->_priv->recognized = recognized;

  if (recognized)
    g_signal_emit (gesture, signals[BEGIN], 0);
  else
    g_signal_emit (gesture, signals[END], 0);
}

static gboolean
_gtk_gesture_do_check (GtkGesture *gesture)
{
  gboolean retval;

  g_signal_emit (G_OBJECT (gesture), signals[CHECK], 0, &retval);

  return retval;
}

static void
_gtk_gesture_check_recognized (GtkGesture *gesture)
{
  GtkGesturePriv *priv = gesture->_priv;
  gint current_n_points;

  current_n_points = g_hash_table_size (priv->points);

  if (priv->recognized && current_n_points != priv->n_points)
    _gtk_gesture_set_recognized (gesture, FALSE);
  else if (!priv->recognized &&
           current_n_points == priv->n_points &&
           _gtk_gesture_do_check (gesture))
    _gtk_gesture_set_recognized (gesture, TRUE);
}

static gboolean
_gtk_gesture_update_point (GtkGesture *gesture,
                           GdkEvent   *event,
                           gboolean    add)
{
  GdkEventSequence *sequence;
  GtkGesturePriv *priv;
  PointData *data;
  guint32 evtime;
  gdouble x, y;

  if (!gdk_event_get_coords (event, &x, &y))
    return FALSE;

  priv = gesture->_priv;
  sequence = gdk_event_get_event_sequence (event);
  evtime = gdk_event_get_time (event);

  if (!g_hash_table_lookup_extended (priv->points, sequence,
                                     NULL, (gpointer *) &data))
    {
      if (!add)
        return FALSE;

      data = g_new0 (PointData, 1);
      g_hash_table_insert (priv->points, sequence, data);
    }

  data->point.x = x;
  data->point.y = y;
  data->sequence = sequence;
  data->evtime = evtime;

  return TRUE;
}

static gboolean
gtk_gesture_handle_event (GtkEventController *controller,
                          GdkEvent           *event)
{
  GtkGesture *gesture = GTK_GESTURE (controller);
  GtkGesturePriv *priv = gesture->_priv;
  GdkEventSequence *sequence;

  sequence = gdk_event_get_event_sequence (event);

  switch (event->type)
    {
    case GDK_TOUCH_BEGIN:
    case GDK_BUTTON_PRESS:
      _gtk_gesture_update_point (gesture, event, TRUE);
      _gtk_gesture_check_recognized (gesture);
      break;
    case GDK_TOUCH_END:
    case GDK_BUTTON_RELEASE:
      if (_gtk_gesture_update_point (gesture, event, FALSE))
        {
          if (priv->recognized)
            {
              g_signal_emit (gesture, signals[UPDATE], 0, sequence);
              _gtk_gesture_set_recognized (gesture, FALSE);
            }

          g_hash_table_remove (priv->points, sequence);
        }
      break;
    case GDK_TOUCH_UPDATE:
    case GDK_MOTION_NOTIFY:
      if (_gtk_gesture_update_point (gesture, event, FALSE))
        {
          _gtk_gesture_check_recognized (gesture);

          if (priv->recognized)
            g_signal_emit (gesture, signals[UPDATE], 0, sequence);
        }
      break;
    default:
      break;
    }

  return FALSE;
}

static void
gtk_gesture_class_init (GtkGestureClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkEventControllerClass *controller_class = GTK_EVENT_CONTROLLER_CLASS (klass);

  object_class->get_property = gtk_gesture_get_property;
  object_class->set_property = gtk_gesture_set_property;
  object_class->finalize = gtk_gesture_finalize;

  controller_class->handle_event = gtk_gesture_handle_event;

  klass->check = gtk_gesture_check_impl;

  g_object_class_install_property (object_class,
                                   PROP_N_POINTS,
                                   g_param_spec_int ("n-points",
                                                     P_("Number of points"),
                                                     P_("Number of points needed "
                                                        "to trigger the gesture"),
                                                     1, G_MAXINT, 1,
                                                     GTK_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT_ONLY));

  signals[CHECK] = g_signal_new ("check",
                                 G_TYPE_FROM_CLASS (klass),
                                 G_SIGNAL_RUN_LAST,
                                 G_STRUCT_OFFSET (GtkGestureClass, check),
                                 g_signal_accumulator_true_handled, NULL,
                                 _gtk_marshal_BOOLEAN__VOID,
                                 G_TYPE_BOOLEAN, 0);
  signals[BEGIN] = g_signal_new ("begin",
                                 G_TYPE_FROM_CLASS (klass),
                                 G_SIGNAL_RUN_LAST,
                                 G_STRUCT_OFFSET (GtkGestureClass, begin),
                                 NULL, NULL,
                                 g_cclosure_marshal_VOID__VOID,
                                 G_TYPE_NONE, 0);
  signals[END] = g_signal_new ("end",
                               G_TYPE_FROM_CLASS (klass),
                               G_SIGNAL_RUN_LAST,
                               G_STRUCT_OFFSET (GtkGestureClass, end),
                               NULL, NULL,
                               g_cclosure_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);
  signals[UPDATE] = g_signal_new ("update",
                                  G_TYPE_FROM_CLASS (klass),
                                  G_SIGNAL_RUN_LAST,
                                  G_STRUCT_OFFSET (GtkGestureClass, update),
                                  NULL, NULL,
                                  g_cclosure_marshal_VOID__POINTER,
                                  G_TYPE_NONE, 1, G_TYPE_POINTER);

  g_type_class_add_private (klass, sizeof (GtkGesturePriv));
}

static void
gtk_gesture_init (GtkGesture *gesture)
{
  GtkGesturePriv *priv;

  gesture->_priv = priv = G_TYPE_INSTANCE_GET_PRIVATE (gesture,
                                                       GTK_TYPE_GESTURE,
                                                       GtkGesturePriv);
  priv->points = g_hash_table_new_full (NULL, NULL, NULL,
                                        (GDestroyNotify) g_free);
}

/**
 * gtk_gesture_get_sequences:
 * @gesture: a #GtkGesture
 *
 * Returns the list of #GdkEventSequence<!-- -->s currently being interpreted
 * by @gesture
 *
 * Returns: (transfer container) (element-type:Gdk.EventSequence): A list
 *          of #GdkEventSequence<!-- -->s, the list elements are owned by GTK+
 *          and must not be freed or modified, the list itself must be deleted
 *          through g_list_free()
 **/
GList *
gtk_gesture_get_sequences (GtkGesture *gesture)
{
  g_return_val_if_fail (GTK_IS_GESTURE (gesture), NULL);

  return g_hash_table_get_keys (gesture->_priv->points);
}

/**
 * gtk_gesture_get_point:
 * @gesture: a #GtkGesture
 * @sequence: (allow-none): a #GdkEventSequence, or %NULL for pointer events
 * @x: (out) (allow-none): return location for X axis of the sequence coordinates
 * @y: (out) (allow-none): return location for Y axis of the sequence coordinates
 *
 * If @sequence is currently being interpreted by @gesture, this
 * function returns %TRUE and fills in @x and @y with the last coordinates
 * stored for that event sequence.
 *
 * Returns: %TRUE if @sequence is currently interpreted
 *
 * Since: 3.8
 **/
gboolean
gtk_gesture_get_point (GtkGesture       *gesture,
                       GdkEventSequence *sequence,
                       gdouble          *x,
                       gdouble          *y)
{
  GtkGesturePriv *priv;
  PointData *data;

  g_return_val_if_fail (GTK_IS_GESTURE (gesture), FALSE);

  priv = gesture->_priv;

  if (!g_hash_table_lookup_extended (priv->points, sequence,
                                     NULL, (gpointer *) &data))
    return FALSE;

  if (x)
    *x = data->point.x;

  if (y)
    *y = data->point.y;

  return TRUE;
}

/**
 * gtk_gesture_get_last_update_time:
 * @gesture: a #GtkGesture
 * @sequence: (allow-none): a #GdkEventSequence, or %NULL for pointer events
 * @evtime: (out) (allow-none): return location for last update time
 *
 * If @sequence is being interpreted by @gesture, this function
 * returns %TRUE and fills @evtime with the last event time it
 * received from that @sequence.
 *
 * Returns: %TRUE if @sequence is currently interpreted
 *
 * Since: 3.8
 **/
gboolean
gtk_gesture_get_last_update_time (GtkGesture       *gesture,
                                  GdkEventSequence *sequence,
                                  guint32          *evtime)
{
  GtkGesturePriv *priv;
  PointData *data;

  g_return_val_if_fail (GTK_IS_GESTURE (gesture), FALSE);

  priv = gesture->_priv;

  if (!g_hash_table_lookup_extended (priv->points, sequence,
                                     NULL, (gpointer *) &data))
    return FALSE;

  if (evtime)
    *evtime = data->evtime;

  return TRUE;
};

gboolean
gtk_gesture_get_bounding_box (GtkGesture            *gesture,
                              cairo_rectangle_int_t *rect)
{
  GtkGesturePriv *priv = gesture->_priv;
  gdouble x1, y1, x2, y2;
  GHashTableIter iter;
  PointData *data;

  g_return_val_if_fail (GTK_IS_GESTURE (gesture), FALSE);
  g_return_val_if_fail (rect != NULL, FALSE);

  if (g_hash_table_size (priv->points) == 0)
    return FALSE;

  x1 = y1 = G_MAXDOUBLE;
  x2 = y2 = -G_MAXDOUBLE;

  g_hash_table_iter_init (&iter, priv->points);

  while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &data))
    {
      x1 = MIN (x1, data->point.x);
      y1 = MIN (y1, data->point.y);
      x2 = MAX (x2, data->point.x);
      y2 = MAX (y2, data->point.y);
    }

  rect->x = x1;
  rect->y = y1;
  rect->width = x2 - x1;
  rect->height = y2 - y1;

  return TRUE;
}

/**
 * gtk_gesture_is_active:
 * @gesture: a #GtkGesture
 *
 * Returns %TRUE if the gesture is currently active.
 * A gesture is active if there are as many interacting
 * touch sequences as required by @gesture, and @gesture
 * is interpreting those to potentially trigger an action.
 *
 * Returns: %TRUE if gesture is active.
 *
 * Since: 3.8
 **/
gboolean
gtk_gesture_is_active (GtkGesture *gesture)
{
  g_return_val_if_fail (GTK_IS_GESTURE (gesture), FALSE);

  return gesture->_priv->recognized;
}
