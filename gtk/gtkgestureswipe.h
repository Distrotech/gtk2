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
#ifndef __GTK_GESTURE_SWIPE_H__
#define __GTK_GESTURE_SWIPE_H__

#if !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
#error "Only <gtk/gtk.h> can be included directly."
#endif

#include <gtk/gtkgesture.h>

G_BEGIN_DECLS

#define GTK_TYPE_GESTURE_SWIPE         (gtk_gesture_swipe_get_type ())
#define GTK_GESTURE_SWIPE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GTK_TYPE_GESTURE_SWIPE, GtkGestureSwipe))
#define GTK_GESTURE_SWIPE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k), GTK_TYPE_GESTURE_SWIPE, GtkGestureSwipeClass))
#define GTK_IS_GESTURE_SWIPE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GTK_TYPE_GESTURE_SWIPE))
#define GTK_IS_GESTURE_SWIPE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), GTK_TYPE_GESTURE_SWIPE))
#define GTK_GESTURE_SWIPE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GTK_TYPE_GESTURE_SWIPE, GtkGestureSwipeClass))

typedef struct _GtkGestureSwipe GtkGestureSwipe;
typedef struct _GtkGestureSwipeClass GtkGestureSwipeClass;
typedef struct _GtkGestureSwipePriv GtkGestureSwipePriv;

struct _GtkGestureSwipe
{
  GtkGesture parent_instance;
  GtkGestureSwipePriv *_priv;
};

struct _GtkGestureSwipeClass
{
  GtkGestureClass parent_class;

  void (* swipe) (GtkGestureSwipe *gesture,
                  gdouble          velocity_x,
                  gdouble          velocity_y);
};

GType        gtk_gesture_swipe_get_type  (void) G_GNUC_CONST;

GtkGesture * gtk_gesture_swipe_new       (void);

G_END_DECLS

#endif /* __GTK_GESTURE_SWIPE_H__ */
