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
#include <gtk/gtkgesturelongpress.h>
#include "gtkmarshalers.h"
#include "gtkprivate.h"
#include "gtkintl.h"

#define DEFAULT_THRESHOLD     32
#define DEFAULT_TRIGGER_DELAY 600

enum {
  PROP_THRESHOLD = 1,
  PROP_TRIGGER_DELAY
};

enum {
  PRESSED,
  N_SIGNALS
};

struct _GtkGestureLongPressPriv
{
  gdouble initial_x;
  gdouble initial_y;

  guint timeout_id;
  guint threshold;
  guint delay;
  guint cancelled : 1;
  guint triggered : 1;
};

static guint signals[N_SIGNALS] = { 0 };

G_DEFINE_TYPE (GtkGestureLongPress, gtk_gesture_long_press, GTK_TYPE_GESTURE)

static void
gtk_gesture_long_press_init (GtkGestureLongPress *gesture)
{
  gesture->_priv = G_TYPE_INSTANCE_GET_PRIVATE (gesture,
                                                GTK_TYPE_GESTURE_LONG_PRESS,
                                                GtkGestureLongPressPriv);
}

static gboolean
gtk_gesture_long_press_check (GtkGesture *gesture)
{
  GtkGestureLongPressPriv *priv;

  priv = GTK_GESTURE_LONG_PRESS (gesture)->_priv;

  if (priv->cancelled)
    return FALSE;

  return TRUE;
}

static gboolean
_gtk_gesture_long_press_timeout (gpointer user_data)
{
  GtkGestureLongPress *gesture = user_data;
  GtkGestureLongPressPriv *priv = gesture->_priv;
  GList *sequences;
  gdouble x, y;

  sequences = gtk_gesture_get_sequences (GTK_GESTURE (gesture));
  g_assert (sequences != NULL);

  gtk_gesture_get_point (GTK_GESTURE (gesture), sequences->data, &x, &y);
  g_list_free (sequences);

  priv->timeout_id = 0;
  priv->triggered = TRUE;
  g_signal_emit (gesture, signals[PRESSED], 0, x, y);

  return FALSE;
}

static void
gtk_gesture_long_press_begin (GtkGesture *gesture)
{
  GtkGestureLongPressPriv *priv = GTK_GESTURE_LONG_PRESS (gesture)->_priv;
  GList *sequences;

  sequences = gtk_gesture_get_sequences (gesture);
  g_assert (sequences);

  gtk_gesture_get_point (gesture, sequences->data,
                         &priv->initial_x, &priv->initial_y);
  g_list_free (sequences);

  priv->timeout_id =
    gdk_threads_add_timeout (priv->delay,
                             _gtk_gesture_long_press_timeout,
                             gesture);
}

static void
gtk_gesture_long_press_update (GtkGesture       *gesture,
                               GdkEventSequence *sequence)
{
  GtkGestureLongPressPriv *priv = GTK_GESTURE_LONG_PRESS (gesture)->_priv;
  gdouble x, y;

  gtk_gesture_get_point (gesture, sequence, &x, &y);

  if (ABS (priv->initial_x - x) > priv->threshold ||
      ABS (priv->initial_y - y) > priv->threshold)
    {
      if (priv->timeout_id)
        {
          g_source_remove (priv->timeout_id);
          priv->timeout_id = 0;
        }

      priv->cancelled = TRUE;
    }
}

static void
gtk_gesture_long_press_end (GtkGesture *gesture)
{
  GtkGestureLongPressPriv *priv = GTK_GESTURE_LONG_PRESS (gesture)->_priv;

  priv->cancelled = priv->triggered = FALSE;

  if (priv->timeout_id)
    {
      g_source_remove (priv->timeout_id);
      priv->timeout_id = 0;
    }
}

static void
gtk_gesture_long_press_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  GtkGestureLongPressPriv *priv = GTK_GESTURE_LONG_PRESS (object)->_priv;

  switch (prop_id)
    {
    case PROP_THRESHOLD:
      g_value_set_uint (value, priv->threshold);
      break;
    case PROP_TRIGGER_DELAY:
      g_value_set_uint (value, priv->delay);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gtk_gesture_long_press_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  GtkGestureLongPressPriv *priv = GTK_GESTURE_LONG_PRESS (object)->_priv;

  switch (prop_id)
    {
    case PROP_THRESHOLD:
      priv->threshold = g_value_get_uint (value);
      break;
    case PROP_TRIGGER_DELAY:
      priv->delay = g_value_get_uint (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gtk_gesture_long_press_finalize (GObject *object)
{
  GtkGestureLongPressPriv *priv = GTK_GESTURE_LONG_PRESS (object)->_priv;

  if (priv->timeout_id)
    g_source_remove (priv->timeout_id);

  G_OBJECT_CLASS (gtk_gesture_long_press_parent_class)->finalize (object);
}

static void
gtk_gesture_long_press_class_init (GtkGestureLongPressClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkGestureClass *gesture_class = GTK_GESTURE_CLASS (klass);

  object_class->get_property = gtk_gesture_long_press_get_property;
  object_class->set_property = gtk_gesture_long_press_set_property;
  object_class->finalize = gtk_gesture_long_press_finalize;

  gesture_class->check = gtk_gesture_long_press_check;
  gesture_class->begin = gtk_gesture_long_press_begin;
  gesture_class->update = gtk_gesture_long_press_update;
  gesture_class->end = gtk_gesture_long_press_end;

  g_object_class_install_property (object_class,
                                   PROP_THRESHOLD,
                                   g_param_spec_uint ("threshold",
                                                      P_("Threshold"),
                                                      P_("Threshold in pixels where the long "
                                                         "press operation remains valid"),
                                                      0, G_MAXUINT, DEFAULT_THRESHOLD,
                                                      GTK_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class,
                                   PROP_TRIGGER_DELAY,
                                   g_param_spec_uint ("trigger-delay",
                                                      P_("Trigger delay"),
                                                      P_("delay in milliseconds before "
                                                         "the gesture is triggered"),
                                                      0, G_MAXUINT, DEFAULT_TRIGGER_DELAY,
                                                      GTK_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT));
  signals[PRESSED] =
    g_signal_new ("pressed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GtkGestureLongPressClass, pressed),
                  NULL, NULL,
                  _gtk_marshal_VOID__DOUBLE_DOUBLE,
                  G_TYPE_NONE, 2, G_TYPE_DOUBLE, G_TYPE_DOUBLE);

  g_type_class_add_private (klass, sizeof (GtkGestureLongPressPriv));
}

/**
 * gtk_gesture_long_press_new:
 *
 * Returns a newly created #GtkGesture that recognizes long presses
 *
 * Returns: a newly created #GtkGestureSwipe
 *
 * Since: 3.8
 **/
GtkGesture *
gtk_gesture_long_press_new (guint delay)
{
  return g_object_new (GTK_TYPE_GESTURE_LONG_PRESS,
                       "trigger-delay", delay,
                       NULL);
}
