#include <glib/gi18n.h>
#include <pipewire/pipewire.h>

#include "window.h"

static void
on_startup (GtkApplication *app)
{
  (void) app;

  adw_init ();
}

static void
on_activate (GtkApplication *app)
{
  static GtkWindow *window = NULL;

	if (!window)
    {
      window = g_object_new (PMS_TYPE_WINDOW,
                             "application", app,
                             NULL);
    }

	gtk_window_present (window);
}

int
main (int   argc,
      char *argv[])
{
	g_autoptr(GtkApplication) app = NULL;

  g_setenv ("G_MESSAGES_DEBUG", "all", TRUE);

  pw_init (&argc, &argv);

	app = gtk_application_new ("com.feaneron.example.PipeWireMediaStream", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "startup", G_CALLBACK (on_startup), NULL);
	g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);

	return g_application_run (G_APPLICATION (app), argc, argv);
}
