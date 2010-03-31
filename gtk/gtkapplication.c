/* GTK - The GIMP Toolkit
 *
 * Copyright (C) 2010 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Colin Walters <walters@verbum.org>
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#include "config.h"
#include "gtkapplication.h"
#include "gtkmain.h"
#include "gtkintl.h"
#include "gtkprivate.h"

enum
{
  PROP_0,
  PROP_WINDOW
};

struct _GtkApplicationPrivate
{
  char *appid;
  GtkActionGroup *main_actions;
  
  GtkWindow *default_window;
};

G_DEFINE_TYPE (GtkApplication, gtk_application, G_TYPE_APPLICATION)

static gboolean
gtk_application_default_quit (GApplication *application, guint timestamp)
{
  gtk_main_quit ();
  return TRUE;
}

static void
gtk_application_default_run (GApplication *application)
{
  gtk_main ();
}

static void
gtk_application_default_action (GApplication *application, const char *action, guint timestamp)
{
  GtkApplication *app = GTK_APPLICATION (application);
  GList *actions, *iter;
  
  actions = gtk_action_group_list_actions (app->priv->main_actions);
  for (iter = actions; iter; iter = iter->next)
    {
      GtkAction *gtkaction = iter->data;
      if (strcmp (action, gtk_action_get_name (gtkaction)) == 0)
        {
          /* TODO set timestamp */
          gtk_action_activate (gtkaction);
          break;
        }
    }
  g_list_free (actions);
}

/**
 * gtk_application_new:
 * @argc: (allow-none) (inout): System argument count
 * @argv: (allow-none) (inout): System argument vector
 * @appid: System-dependent application identifier
 *
 * Create a new #GtkApplication, or if one has already been initialized
 * in this process, return the existing instance. This function will as a side effect
 * initialize the display system; see gtk_init().
 *
 * For the behavior if this application is running in another process,
 * see g_application_new().
 *
 * Returns: (transfer full): A newly-referenced #GtkApplication
 */
GtkApplication*
gtk_application_new (int                  *argc,
                     char               ***argv,
                     const char           *appid)
{  
  gtk_init (argc, argv);

  return g_object_new (GTK_TYPE_APPLICATION, "appid", appid, NULL);
}

static void
on_action_sensitive (GtkAction      *action,
		     GParamSpec     *pspec,
		     GtkApplication *app)
{
  
  g_application_set_action_enabled (G_APPLICATION (app), 
				    gtk_action_get_name (action),
				    gtk_action_get_sensitive (action));
}

/**
 * gtk_application_set_main_action_group:
 * @app: A #GtkApplication
 * @group: A #GtkActionGroup
 *
 * Set @group as this application's global action group.  This will
 * ensure the operating system interface uses these actions as follows:
 *
 * <itemizedlist>
 *   <listitem>In GNOME 2 this exposes the actions for scripting.<listitem>
 *   <listitem>In GNOME 3, this function populates the application menu.</listitem>
 *   <listitem>In Windows prior to version 7, this function does nothing.</listitem>
 *   <listitem>In Windows 7, this function adds "Tasks" to the Jump List.</listitem>
 *   <listitem>In Mac OS X, this function extends the Dock menu.</listitem>
 * </itemizedlist>
 *
 * It is an error to call this function more than once.
 */
void
gtk_application_set_main_action_group (GtkApplication *app,
                                       GtkActionGroup *group)
{
  GList *actions, *iter;

  g_return_if_fail (GTK_IS_APPLICATION (app));
  g_return_if_fail (app->priv->main_actions == NULL);
  
  app->priv->main_actions = g_object_ref (group);
  actions = gtk_action_group_list_actions (group);
  for (iter = actions; iter; iter = iter->next)
    {
      GtkAction *action = iter->data;
      g_application_add_action (G_APPLICATION (app),
                                gtk_action_get_name (action),
                                gtk_action_get_tooltip (action));
      g_signal_connect (action, "notify::sensitive", G_CALLBACK (on_action_sensitive), app);
    }
  g_list_free (actions);
}

static gboolean
gtk_application_on_window_destroy (GtkWidget  *window,
                                   gpointer    user_data)
{
  GtkApplication *app = GTK_APPLICATION (user_data);

  gtk_application_quit (app);
  return FALSE;
}

/**
 * gtk_application_get_window:
 * @app: a #GtkApplication
 *
 * A #GtkApplication has a "default window".  This window should act as
 * the primary user interaction point with your application.  This
 * window is of type #GTK_WINDOW_TYPE_TOPLEVEL and its properties such
 * as "title" and "icon-name" will be initialized as appropriate for the
 * platform.
 *
 * If the user closes this window, and your application hasn't created
 * any other windows, the default action will be to call gtk_application_quit().
 *
 * Returns: (transfer none): The default #GtkWindow for this application
 */
GtkWindow *
gtk_application_get_window (GtkApplication *app)
{
  if (app->priv->default_window != NULL)
    return app->priv->default_window;

  app->priv->default_window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
  g_object_ref_sink (app->priv->default_window);
  /* TODO look up .desktop file on freedesktop platform, initialize title etc. */
  g_signal_connect (app->priv->default_window, "destroy",
                    G_CALLBACK (gtk_application_on_window_destroy), app);

  return app->priv->default_window;
}

/**
 * gtk_application_run:
 * @app: a #GtkApplication
 *
 * Runs the main loop; see g_application_run().  The default
 * implementation for #GtkApplication uses gtk_main().
 */
void
gtk_application_run (GtkApplication  *app)
{
  g_application_run (G_APPLICATION (app));
}

/**
 * gtk_application_quit:
 * @app: a #GtkApplication
 *
 * Request the application exit.  By default, this
 * method will exit the main loop; see gtk_main_quit().
 */
void
gtk_application_quit (GtkApplication *app)
{
  g_application_quit (G_APPLICATION (app), gtk_get_current_event_time ());
}

static void
gtk_application_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  GtkApplication *app = GTK_APPLICATION (object);

  switch (prop_id)
    {
      case PROP_WINDOW:
        g_value_set_object (value, gtk_application_get_window (app));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtk_application_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  GtkApplication *app = GTK_APPLICATION (object);
  
  g_assert (app != NULL);

  switch (prop_id)
    {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
gtk_application_init (GtkApplication *application)
{
  application->priv = G_TYPE_INSTANCE_GET_PRIVATE (application, GTK_TYPE_APPLICATION, GtkApplicationPrivate);
}

static GObject*
gtk_application_constructor (GType                  type,
                             guint                  n_construct_properties,
                             GObjectConstructParam *construct_params)
{
  GObject *object;
  
  object = (* G_OBJECT_CLASS (gtk_application_parent_class)->constructor) (type,
                                                                           n_construct_properties,
                                                                           construct_params);
  
  return object;
}

static void
gtk_application_class_init (GtkApplicationClass *klass)
{
  GObjectClass *gobject_class;
  GApplicationClass *application_class;

  gobject_class = G_OBJECT_CLASS (klass);
  application_class = G_APPLICATION_CLASS (klass);

  gobject_class->constructor = gtk_application_constructor;
  gobject_class->get_property = gtk_application_get_property;
  gobject_class->set_property = gtk_application_set_property;
  
  application_class->run = gtk_application_default_run;
  application_class->quit = gtk_application_default_quit;
  application_class->action = gtk_application_default_action;

  g_type_class_add_private (gobject_class, sizeof (GtkApplicationPrivate));
}

#define __GTK_APPLICATION_C__
#include "gtkaliasdef.c"
