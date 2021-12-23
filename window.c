#include "window.h"

#include "pw-media-stream.h"

#include <libportal/portal.h>
#include <libportal-gtk4/portal-gtk4.h>

struct _PmsWindow
{
  AdwApplicationWindow  parent_instance;

  GtkStack *stack;
  GtkVideo *video;

  GCancellable *cancellable;
  XdpPortal *portal;
  XdpSession *session;
};

G_DEFINE_TYPE (PmsWindow, pms_window, ADW_TYPE_APPLICATION_WINDOW)

static void
on_session_closed_cb (XdpSession *session,
                      PmsWindow  *self)
{
  gtk_stack_set_visible_child_name (self->stack, "start");
  gtk_video_set_media_stream (self->video, NULL);
}

static void
on_screencast_started (GObject      *source,
                       GAsyncResult *result,
                       gpointer      user_data)
{
  g_autoptr (GtkMediaStream) media_stream = NULL;
  g_autoptr (GVariant) stream_properties = NULL;
  g_autoptr (GError) error = NULL;
	GVariantIter iter;
  PmsWindow *self;
  GVariant *streams;
  uint32_t node_id;
  int fd;

  xdp_session_start_finish (XDP_SESSION (source), result, &error);
  if (error)
    {
      g_warning ("Error starting screencast session: %s", error->message);
      return;
    }

  self = PMS_WINDOW (user_data);
  gtk_stack_set_visible_child_name (self->stack, "video");

  fd = xdp_session_open_pipewire_remote (self->session);
  streams = xdp_session_get_streams (self->session);

	g_variant_iter_init (&iter, streams);
	g_variant_iter_loop (&iter, "(u@a{sv})", &node_id, &stream_properties);

  media_stream = pw_media_stream_new (fd, node_id, &error);
  gtk_video_set_media_stream (self->video, media_stream);
}

static void
on_screencast_created (GObject      *source,
                       GAsyncResult *result,
                       gpointer      user_data)
{
  g_autoptr (XdpSession) session = NULL;
  g_autoptr (GError) error = NULL;
  XdpParent *parent;
  PmsWindow *self;

  session = xdp_portal_create_screencast_session_finish (XDP_PORTAL (source), result, &error);
  if (error)
    {
      g_warning ("Error creating screencast session: %s", error->message);
      return;
    }

  self = PMS_WINDOW (user_data);
  self->session = g_steal_pointer (&session);
  g_signal_connect (self->session, "closed", G_CALLBACK (on_session_closed_cb), self);

  parent = xdp_parent_new_gtk (GTK_WINDOW (self));

  xdp_session_start (self->session,
                     parent,
                     self->cancellable,
                     on_screencast_started,
                     self);
  xdp_parent_free (parent);
}

static void
on_start_screencast_button_clicked_cb (GtkButton *button,
                                       PmsWindow *self)
{
  xdp_portal_create_screencast_session (self->portal,
                                        XDP_OUTPUT_WINDOW | XDP_OUTPUT_MONITOR,
                                        XDP_SCREENCAST_FLAG_NONE,
                                        XDP_CURSOR_MODE_METADATA,
                                        XDP_PERSIST_MODE_NONE,
                                        NULL,
                                        self->cancellable,
                                        on_screencast_created,
                                        self);
}

static void
on_go_previous_button_clicked_cb (GtkButton *button,
                                  PmsWindow *self)
{
  gtk_stack_set_visible_child_name (self->stack, "start");
  gtk_video_set_media_stream (self->video, NULL);
}

static gboolean
pms_window_close_request (GtkWindow *window)
{
  PmsWindow *self = PMS_WINDOW (window);

  if (self->session)
    {
      xdp_session_close (self->session);
      g_clear_object (&self->session);
    }

  return GTK_WINDOW_CLASS (pms_window_parent_class)->close_request (window);
}

static void
pms_window_dispose (GObject *object)
{
  PmsWindow *self = PMS_WINDOW (object);

  g_cancellable_cancel (self->cancellable);
  g_clear_object (&self->cancellable);
  g_clear_object (&self->portal);

  G_OBJECT_CLASS (pms_window_parent_class)->dispose (object);
}

static void
pms_window_class_init (PmsWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkWindowClass *window_class = GTK_WINDOW_CLASS (klass);

  object_class->dispose = pms_window_dispose;

  window_class->close_request = pms_window_close_request;

  gtk_widget_class_set_template_from_resource (widget_class, "/com/feaneron/example/PipeWireMediaStream/window.ui");
  gtk_widget_class_bind_template_child (widget_class, PmsWindow, stack);
  gtk_widget_class_bind_template_child (widget_class, PmsWindow, video);
  gtk_widget_class_bind_template_callback (widget_class, on_go_previous_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, on_start_screencast_button_clicked_cb);
}

static void
pms_window_init (PmsWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->portal = xdp_portal_new ();
  self->cancellable = g_cancellable_new ();
}
