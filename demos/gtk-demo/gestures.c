/* Gestures
 *
 * Perform gestures on touchscreens and other input devices. This
 * demo reacts to long presses and swipes from all devices, plus
 * multi-touch rotate and zoom gestures.
 */

#include <gtk/gtk.h>

static GtkWidget *window = NULL;
static GtkGesture *swipe = NULL;
static GtkGesture *rotate = NULL;
static GtkGesture *zoom = NULL;
static GtkGesture *long_press = NULL;
static gdouble swipe_x = 0;
static gdouble swipe_y = 0;
static gboolean long_pressed = FALSE;

static void
swipe_gesture_swept (GtkGestureSwipe *gesture,
                     gdouble          velocity_x,
                     gdouble          velocity_y)
{
  swipe_x = velocity_x / 10;
  swipe_y = velocity_y / 10;
}

static void
long_press_gesture_pressed (GtkGestureLongPress *gesture,
                            gdouble              x,
                            gdouble              y)
{
  long_pressed = TRUE;
  gtk_widget_queue_draw (window);
}

static void
long_press_gesture_end (GtkGesture *gesture)
{
  long_pressed = FALSE;
  gtk_widget_queue_draw (window);
}

static gboolean
drawing_area_draw (GtkWidget *widget,
                   cairo_t   *cr)
{
  GtkAllocation allocation;

  gtk_widget_get_allocation (widget, &allocation);

  if (swipe_x != 0 || swipe_y != 0)
    {
      cairo_save (cr);
      cairo_set_line_width (cr, 6);
      cairo_move_to (cr, allocation.width / 2,
                     allocation.height / 2);
      cairo_rel_line_to (cr, swipe_x, swipe_y);
      cairo_set_source_rgba (cr, 1, 0, 0, 0.5);
      cairo_stroke (cr);
      cairo_restore (cr);
    }

  if (gtk_gesture_is_active (rotate) || gtk_gesture_is_active (zoom))
    {
      cairo_pattern_t *pat;
      cairo_matrix_t matrix;
      gdouble angle, scale;

      cairo_matrix_init_translate (&matrix,
                                   allocation.width / 2,
                                   allocation.height / 2);

      cairo_save (cr);

      if (gtk_gesture_rotate_get_angle_delta (GTK_GESTURE_ROTATE (rotate), &angle))
        cairo_matrix_rotate (&matrix, angle);

      if (gtk_gesture_zoom_get_scale_delta (GTK_GESTURE_ZOOM (zoom), &scale))
        cairo_matrix_scale (&matrix, scale, scale);

      cairo_set_matrix (cr, &matrix);
      cairo_rectangle (cr, -100, -100, 200, 200);

      pat = cairo_pattern_create_linear (-100, 0, 200, 0);
      cairo_pattern_add_color_stop_rgb (pat, 0, 0, 0, 1);
      cairo_pattern_add_color_stop_rgb (pat, 1, 1, 0, 0);
      cairo_set_source (cr, pat);
      cairo_fill (cr);

      cairo_restore (cr);

      cairo_pattern_destroy (pat);
    }

  if (long_pressed)
    {
      cairo_save (cr);
      cairo_arc (cr, allocation.width / 2,
                 allocation.height / 2,
                 50, 0, 2 * G_PI);

      cairo_set_source_rgba (cr, 0, 1, 0, 0.5);
      cairo_stroke (cr);

      cairo_restore (cr);
    }

  return TRUE;
}

static gboolean
drawing_area_event (GtkWidget *widget,
                    GdkEvent  *event)
{
  gtk_event_controller_handle_event (GTK_EVENT_CONTROLLER (swipe), event);
  gtk_event_controller_handle_event (GTK_EVENT_CONTROLLER (rotate), event);
  gtk_event_controller_handle_event (GTK_EVENT_CONTROLLER (zoom), event);
  gtk_event_controller_handle_event (GTK_EVENT_CONTROLLER (long_press), event);

  gtk_widget_queue_draw (widget);

  return FALSE;
}

GtkWidget *
do_gestures (GtkWidget *do_widget)
{
  GtkWidget *drawing_area;

  if (!window)
    {
      window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      gtk_window_set_default_size (GTK_WINDOW (window), 400, 400);
      gtk_window_set_title (GTK_WINDOW (window), "Gestures demo");
      g_signal_connect (window, "destroy",
                        G_CALLBACK (gtk_widget_destroyed), &window);

      drawing_area = gtk_drawing_area_new ();
      gtk_container_add (GTK_CONTAINER (window), drawing_area);
      gtk_widget_add_events (drawing_area,
                             GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                             GDK_POINTER_MOTION_MASK | GDK_TOUCH_MASK);

      g_signal_connect (drawing_area, "draw",
                        G_CALLBACK (drawing_area_draw), NULL);
      g_signal_connect (drawing_area, "event",
                        G_CALLBACK (drawing_area_event), NULL);

      swipe = gtk_gesture_swipe_new ();
      g_signal_connect (swipe, "swipe",
                        G_CALLBACK (swipe_gesture_swept), NULL);

      long_press = gtk_gesture_long_press_new (500);
      g_signal_connect (long_press, "pressed",
                        G_CALLBACK (long_press_gesture_pressed), NULL);
      g_signal_connect (long_press, "end",
                        G_CALLBACK (long_press_gesture_end), NULL);

      rotate = gtk_gesture_rotate_new ();
      zoom = gtk_gesture_zoom_new ();
    }

  if (!gtk_widget_get_visible (window))
    gtk_widget_show_all (window);
  else
    {
      gtk_widget_destroy (window);
      window = NULL;
    }

  return window;
}
