#include <gtk/gtk.h>

static gboolean
destroy_cb (GtkWidget *win)
{
    gtk_widget_destroy (win);
    gtk_main_quit();
    return FALSE;
}

static void
show_window (void)
{
    GtkWidget *win;
    win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated (GTK_WINDOW (win), FALSE);
    gtk_window_set_title (GTK_WINDOW (win), "Naked");
    gtk_widget_show_all (win);
    g_timeout_add_seconds (5, (GSourceFunc)destroy_cb, win);
}

int
main(int argc, char** argv)
{
    gtk_init (&argc, &argv);
    show_window();
    gtk_main();
    return 0;
}
