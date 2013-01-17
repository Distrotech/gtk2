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
#include <gtk/gtkgestureswipe.h>
#include "gtkmarshalers.h"

#define CAPTURE_THRESHOLD_MS 150

typedef struct _EventData EventData;

struct _EventData
{
  guint32 evtime;
  GdkPoint point;
};

struct _GtkGestureSwipePriv
{
  GArray *events;
};

enum {
  SWIPE,
  N_SIGNALS
};

static guint signals[N_SIGNALS] = { 0 };

G_DEFINE_TYPE (GtkGestureSwipe, gtk_gesture_swipe, GTK_TYPE_GESTURE)

static void
gtk_gesture_swipe_finalize (GObject *object)
{
  GtkGestureSwipePriv *priv = GTK_GESTURE_SWIPE (object)->_priv;

  g_array_free (priv->events, TRUE);

  G_OBJECT_CLASS (gtk_gesture_swipe_parent_class)->finalize (object);
}

static void
_gtk_gesture_swipe_clear_backlog (GtkGestureSwipe *gesture,
                                  guint32          evtime)
{
  GtkGestureSwipePriv *priv = gesture->_priv;
  gint i, length = 0;

  for (i = 0; i < priv->events->len; i++)
    {
      EventData *data;

      data = &g_array_index (priv->events, EventData, i);

      if (data->evtime >= evtime - CAPTURE_THRESHOLD_MS)
        {
          length = i - 1;
          break;
        }
    }

  if (length > 0)
    g_array_remove_range (priv->events, 0, length);
}

static void
gtk_gesture_swipe_update (GtkGesture       *gesture,
                          GdkEventSequence *sequence)
{
  GtkGestureSwipe *swipe = GTK_GESTURE_SWIPE (gesture);
  GtkGestureSwipePriv *priv = swipe->_priv;
  EventData new;
  gdouble x, y;

  gtk_gesture_get_last_update_time (gesture, sequence, &new.evtime);
  gtk_gesture_get_point (gesture, sequence, &x, &y);

  new.point.x = x;
  new.point.y = y;

  _gtk_gesture_swipe_clear_backlog (swipe, new.evtime);
  g_array_append_val (priv->events, new);
}

static void
_gtk_gesture_swipe_calculate_velocity (GtkGestureSwipe *gesture,
                                       gdouble         *velocity_x,
                                       gdouble         *velocity_y)
{
  GtkGestureSwipePriv *priv = gesture->_priv;
  EventData *start, *end;
  gdouble diff_x, diff_y;
  guint32 diff_time;

  *velocity_x = *velocity_y = 0;

  if (priv->events->len == 0)
    return;

  start = &g_array_index (priv->events, EventData, 0);
  end = &g_array_index (priv->events, EventData, priv->events->len - 1);

  diff_time = end->evtime - start->evtime;
  diff_x = end->point.x - start->point.x;
  diff_y = end->point.y - start->point.y;

  if (diff_time == 0)
    return;

  /* Velocity in pixels/sec */
  *velocity_x = diff_x * 1000 / diff_time;
  *velocity_y = diff_y * 1000 / diff_time;
}

static void
gtk_gesture_swipe_end (GtkGesture *gesture)
{
  GtkGestureSwipe *swipe = GTK_GESTURE_SWIPE (gesture);
  GtkGestureSwipePriv *priv = swipe->_priv;
  gdouble velocity_x, velocity_y;
  guint32 evtime;

  gtk_gesture_get_last_update_time (gesture, 0, &evtime);
  _gtk_gesture_swipe_clear_backlog (swipe, evtime);
  _gtk_gesture_swipe_calculate_velocity (swipe, &velocity_x, &velocity_y);
  g_signal_emit (gesture, signals[SWIPE], 0, velocity_x, velocity_y);

  if (priv->events->len > 0)
    g_array_remove_range (priv->events, 0, priv->events->len);
}

static void
gtk_gesture_swipe_class_init (GtkGestureSwipeClass *klass)
{
  GtkGestureClass *gesture_class = GTK_GESTURE_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gtk_gesture_swipe_finalize;

  gesture_class->update = gtk_gesture_swipe_update;
  gesture_class->end = gtk_gesture_swipe_end;

  signals[SWIPE] =
    g_signal_new ("swipe",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GtkGestureSwipeClass, swipe),
                  NULL, NULL,
                  _gtk_marshal_VOID__DOUBLE_DOUBLE,
                  G_TYPE_NONE, 2, G_TYPE_DOUBLE, G_TYPE_DOUBLE);

  g_type_class_add_private (klass, sizeof (GtkGestureSwipePriv));
}

static void
gtk_gesture_swipe_init (GtkGestureSwipe *gesture)
{
  gesture->_priv = G_TYPE_INSTANCE_GET_PRIVATE (gesture,
                                                GTK_TYPE_GESTURE_SWIPE,
                                                GtkGestureSwipePriv);
  gesture->_priv->events = g_array_new (FALSE, FALSE, sizeof (EventData));
}

/**
 * gtk_gesture_swipe_new:
 *
 * Returns a newly created #GtkGesture that recognizes swipes
 *
 * Returns: a newly created #GtkGestureSwipe
 *
 * Since: 3.8
 **/
GtkGesture *
gtk_gesture_swipe_new (void)
{
  return g_object_new (GTK_TYPE_GESTURE_SWIPE, NULL);
}
