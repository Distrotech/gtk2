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
#include <gtk/gtkeventcontroller.h>
#include "gtkmarshalers.h"

enum {
  HANDLE_EVENT,
  N_SIGNALS
};

guint signals[N_SIGNALS] = { 0 };

G_DEFINE_ABSTRACT_TYPE (GtkEventController, gtk_event_controller, G_TYPE_OBJECT)

static gboolean
gtk_event_controller_handle_event_default (GtkEventController *controller,
                                           GdkEvent           *event)
{
  return FALSE;
}

static void
gtk_event_controller_class_init (GtkEventControllerClass *klass)
{
  klass->handle_event = gtk_event_controller_handle_event_default;

  signals[HANDLE_EVENT] =
    g_signal_new ("handle-event",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (GtkEventControllerClass, handle_event),
                  g_signal_accumulator_true_handled, NULL,
                  _gtk_marshal_BOOLEAN__BOXED,
                  G_TYPE_BOOLEAN, 1,
                  GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);
}

static void
gtk_event_controller_init (GtkEventController *controller)
{
}

/**
 * gtk_event_controller_handle_event:
 * @controller: a #GtkEventController
 * @event: a #GdkEvent
 *
 * Feeds an events into @controller, so it can be interpreted
 * and the controller actions triggered.
 *
 * Returns: %TRUE if the event was potentially useful to construct
 *          a gesture.
 *
 * Since: 3.8
 **/
gboolean
gtk_event_controller_handle_event (GtkEventController *controller,
                                   GdkEvent           *event)
{
  gboolean retval;

  g_return_val_if_fail (GTK_IS_EVENT_CONTROLLER (controller), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  g_signal_emit (controller, signals[HANDLE_EVENT], 0, event, &retval);

  return retval;
}
