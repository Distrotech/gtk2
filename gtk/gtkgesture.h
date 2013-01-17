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
#ifndef __GTK_GESTURE_H__
#define __GTK_GESTURE_H__

#if !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
#error "Only <gtk/gtk.h> can be included directly."
#endif

#include <gtk/gtkeventcontroller.h>

G_BEGIN_DECLS

#define GTK_TYPE_GESTURE         (gtk_gesture_get_type ())
#define GTK_GESTURE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GTK_TYPE_GESTURE, GtkGesture))
#define GTK_GESTURE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k), GTK_TYPE_GESTURE, GtkGestureClass))
#define GTK_IS_GESTURE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GTK_TYPE_GESTURE))
#define GTK_IS_GESTURE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), GTK_TYPE_GESTURE))
#define GTK_GESTURE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GTK_TYPE_GESTURE, GtkGestureClass))

typedef struct _GtkGesture GtkGesture;
typedef struct _GtkGestureClass GtkGestureClass;
typedef struct _GtkGesturePriv GtkGesturePriv;

struct _GtkGesture
{
  GtkEventController parent_instance;

  /*< private >*/
  GtkGesturePriv *_priv;
};

struct _GtkGestureClass
{
  GtkEventControllerClass parent_class;

  gboolean (* check)  (GtkGesture       *gesture);

  void     (* begin)  (GtkGesture       *gesture);
  void     (* update) (GtkGesture       *gesture,
                       GdkEventSequence *sequence);
  void     (* end)    (GtkGesture       *gesture);
};

GType      gtk_gesture_get_type             (void) G_GNUC_CONST;

GList    * gtk_gesture_get_sequences        (GtkGesture            *gesture);

gboolean   gtk_gesture_get_point            (GtkGesture            *gesture,
                                             GdkEventSequence      *sequence,
                                             gdouble               *x,
                                             gdouble               *y);
gboolean   gtk_gesture_get_last_update_time (GtkGesture            *gesture,
                                             GdkEventSequence      *sequence,
                                             guint32               *evtime);
gboolean   gtk_gesture_get_bounding_box     (GtkGesture            *gesture,
                                             cairo_rectangle_int_t *rect);
gboolean   gtk_gesture_is_active            (GtkGesture            *gesture);


G_END_DECLS

#endif /* __GTK_GESTURE_H__ */
