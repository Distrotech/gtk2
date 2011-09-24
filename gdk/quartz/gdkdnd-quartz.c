/* gdkdnd-quartz.c
 *
 * Copyright (C) 2005 Imendio AB
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "gdkdnd.h"
#include "gdkprivate-quartz.h"

static gpointer parent_class = NULL;

static void
gdk_drag_context_finalize (GObject *object)
{
  GdkDragContext *context = GDK_DRAG_CONTEXT (object);
  GdkDragContextPrivate *private = GDK_DRAG_CONTEXT_PRIVATE (context);
 
  g_free (private);
  
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gdk_drag_context_init (GdkDragContext *dragcontext)
{
  GdkDragContextPrivate *priv = g_new0 (GdkDragContextPrivate, 1);

  dragcontext->windowing_data = priv;
}

static void
gdk_drag_context_class_init (GdkDragContextClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize = gdk_drag_context_finalize;
}

GType
gdk_drag_context_get_type (void)
{
  static GType object_type = 0;

  if (!object_type)
    {
      const GTypeInfo object_info =
      {
        sizeof (GdkDragContextClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) gdk_drag_context_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (GdkDragContext),
        0,              /* n_preallocs */
        (GInstanceInitFunc) gdk_drag_context_init,
      };
      
      object_type = g_type_register_static (G_TYPE_OBJECT,
                                            "GdkDragContext",
                                            &object_info,
					    0);
    }
  
  return object_type;
}

GdkDragContext *
gdk_drag_context_new (void)
{
  return (GdkDragContext *)g_object_new (gdk_drag_context_get_type (), NULL);
}

void            
gdk_drag_context_ref (GdkDragContext *context)
{
  g_object_ref (context);
}

void            
gdk_drag_context_unref (GdkDragContext *context)
{
  g_object_unref (context);
}

GdkDragContext *_gdk_quartz_drag_source_context = NULL;

GdkDragContext *
gdk_quartz_drag_source_context ()
{
  return _gdk_quartz_drag_source_context;
}

GdkDragContext * 
gdk_drag_begin (GdkWindow     *window,
		GList         *targets)
{
  if (_gdk_quartz_drag_source_context != NULL)
    {
      /* Something is amiss with the existing drag, so log a message
	 and abort it */
      g_warning ("Drag begun with existing context; aborting the preexisting drag");
      gdk_drag_abort (_gdk_quartz_drag_source_context,
		      (guint32)g_get_real_time ());
    }

  
  /* Create fake context */
  _gdk_quartz_drag_source_context = gdk_drag_context_new ();
  _gdk_quartz_drag_source_context->is_source = TRUE;
  _gdk_quartz_drag_source_context->source_window = window;
  
  return _gdk_quartz_drag_source_context;
}

gboolean        
gdk_drag_motion (GdkDragContext *context,
		 GdkWindow      *dest_window,
		 GdkDragProtocol protocol,
		 gint            x_root, 
		 gint            y_root,
		 GdkDragAction   suggested_action,
		 GdkDragAction   possible_actions,
		 guint32         time)
{
  /* FIXME: Implement */
  return FALSE;
}

guint32
gdk_drag_get_protocol_for_display (GdkDisplay      *display,
				   guint32          xid,
				   GdkDragProtocol *protocol)
{
  /* FIXME: Implement */
  return 0;
}

void
gdk_drag_find_window_for_screen (GdkDragContext  *context,
				 GdkWindow       *drag_window,
				 GdkScreen       *screen,
				 gint             x_root,
				 gint             y_root,
				 GdkWindow      **dest_window,
				 GdkDragProtocol *protocol)
{
  /* FIXME: Implement */
}

static void
gdk_quartz_drag_end (GdkDragContext *context)
{
  GdkEvent event;

  g_assert (context != NULL);
  g_warning ("Gdk-quartz-drag-end\n");
  event.dnd.type = GDK_DROP_FINISHED;
  event.dnd.window = g_object_ref (context->source_window);
  event.dnd.send_event = FALSE;
  event.dnd.context = context;

  (*_gdk_event_func) (&event, _gdk_event_data);

  g_object_unref (event.dnd.window);

  g_object_unref (_gdk_quartz_drag_source_context);
  _gdk_quartz_drag_source_context = NULL;
}

void
gdk_drag_drop (GdkDragContext *context,
	       guint32         time)
{
  g_warning ("Gdk-quartz-drag-drop, ending\n");
  gdk_quartz_drag_end (context);
}

void
gdk_drag_abort (GdkDragContext *context,
		guint32         time)
{
  gdk_quartz_drag_end (context);
}

void             
gdk_drag_status (GdkDragContext   *context,
		 GdkDragAction     action,
		 guint32           time)
{
  context->action = action;
}

void 
gdk_drop_reply (GdkDragContext   *context,
		gboolean          ok,
		guint32           time)
{
  g_return_if_fail (context != NULL);

  /* FIXME: Implement */
}

void             
gdk_drop_finish (GdkDragContext   *context,
		 gboolean          success,
		 guint32           time)
{
  /* FIXME: Implement */
}

void            
gdk_window_register_dnd (GdkWindow *window)
{
  /* FIXME: Implement */
}

GdkAtom       
gdk_drag_get_selection (GdkDragContext *context)
{
  /* FIXME: Implement */
  return GDK_NONE;
}

gboolean 
gdk_drag_drop_succeeded (GdkDragContext *context)
{
  /* FIXME: Implement */
  return FALSE;
}

id
gdk_quartz_drag_context_get_dragging_info_libgtk_only (GdkDragContext *context)
{
  return GDK_DRAG_CONTEXT_PRIVATE (context)->dragging_info;
}
