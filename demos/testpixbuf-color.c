/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

#include "config.h"
#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>

#define ICC_PROFILE             "/usr/share/color/icc/bluish.icc"
#define ICC_PROFILE_SIZE        3966

static gboolean
save_image_png (const gchar *filename, GdkPixbuf *pixbuf, GError **error)
{
	gchar *contents = NULL;
	gchar *contents_encode = NULL;
	gsize length;
	gboolean ret;
	gint len;

	/* get icc file */
	ret = g_file_get_contents (ICC_PROFILE, &contents, &length, error);
	if (!ret)
		goto out;
	contents_encode = g_base64_encode ((const guchar *) contents, length);
	ret = gdk_pixbuf_save (pixbuf, filename, "png", error,
			       "tEXt::Software", "Hello my name is dave",
			       "icc-profile", contents_encode,
			       NULL);
	len = strlen (contents_encode);
	g_debug ("ICC profile was %i bytes", len);
out:
	g_free (contents);
	g_free (contents_encode);
	return ret;
}

static gboolean
save_image_tiff (const gchar *filename, GdkPixbuf *pixbuf, GError **error)
{
	gchar *contents = NULL;
	gchar *contents_encode = NULL;
	gsize length;
	gboolean ret;
	gint len;

	/* get icc file */
	ret = g_file_get_contents (ICC_PROFILE, &contents, &length, error);
	if (!ret)
		goto out;
	contents_encode = g_base64_encode ((const guchar *) contents, length);
	ret = gdk_pixbuf_save (pixbuf, filename, "tiff", error,
			       "icc-profile", contents_encode,
			       NULL);
	len = strlen (contents_encode);
	g_debug ("ICC profile was %i bytes", len);
out:
	g_free (contents);
	g_free (contents_encode);
	return ret;
}

static gboolean
save_image_verify (const gchar *filename, GError **error)
{
	gboolean ret = FALSE;
	GdkPixbuf *pixbuf = NULL;
	const gchar *option;
	gchar *icc_profile = NULL;
	gsize len = 0;

	/* load */
	pixbuf = gdk_pixbuf_new_from_file (filename, error);
	if (pixbuf == NULL)
		goto out;

	/* check values */
	option = gdk_pixbuf_get_option (pixbuf, "icc-profile");
	if (option == NULL) {
		*error = g_error_new (1, 0, "no profile set");
		goto out;
	}

	/* decode base64 */
	icc_profile = (gchar *) g_base64_decode (option, &len);
	if (len != ICC_PROFILE_SIZE) {
		*error = g_error_new (1, 0,
		                      "profile length invalid, got %" G_GSIZE_FORMAT,
		                      len);
		g_file_set_contents ("error.icc", icc_profile, len, NULL);
		goto out;
	}

	/* success */
	ret = TRUE;
out:
	if (pixbuf != NULL)
		g_object_unref (pixbuf);
	g_free (icc_profile);
	return ret;
}

static gboolean
transform_image (const gchar *filename, GError **error)
{
	GtkWidget *image_test;
	GtkWidget *dialog;
	GtkWidget *vbox;
	gint response;
	GdkPixbuf *pixbuf;
	GtkColorEngine *engine = NULL;
	GtkColorProfile *profile = NULL;
	GtkColorTransform *transform = NULL;
	gchar *icc_profile = NULL;
	gsize len = 0;
	gboolean ret;
	const gchar *option;

	image_test = gtk_image_new_from_file (filename);
	pixbuf = gtk_image_get_pixbuf (GTK_IMAGE(image_test));

	/* check values */
	option = gdk_pixbuf_get_option (pixbuf, "icc-profile");
	if (option == NULL) {
		*error = g_error_new (1, 0, "no profile set");
		goto out;
	}

	/* decode base64 */
	icc_profile = (gchar *) g_base64_decode (option, &len);
	if (len == 0) {
		*error = g_error_new (1, 0, "no embedded profile");
		goto out;
	}

        /* transform this image with the embedded profile */
        engine = gtk_color_engine_get_default ();
        profile = gtk_color_engine_create_profile (engine, (const guint8 *)icc_profile, len, error);
        if (profile == NULL) {
                ret = FALSE;
                goto out;
        }
        transform = gtk_color_engine_create_transform_from_profiles (engine, profile, NULL);
        ret = gtk_color_transform_apply_pixbuf_in_place (transform, pixbuf, error);
        if (!ret)
                goto out;

	/* show in a dialog as an example */
	dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "Are the fish yellow?");
	vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	gtk_box_pack_end (GTK_BOX(vbox), image_test, TRUE, TRUE, 12);
	gtk_widget_set_size_request (GTK_WIDGET(image_test), 300, 300);
	gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);
	gtk_widget_show (image_test);

        /* ask the user to ensure the image was converted */
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response != GTK_RESPONSE_YES) {
		ret = FALSE;
		*error = g_error_new (1, 0, "failed to convert fish");
		goto out;
	}
out:
        if (transform != NULL)
                g_object_unref (transform);
        if (profile != NULL)
                g_object_unref (profile);
        if (engine != NULL)
                g_object_unref (engine);
	g_free (icc_profile);
	return ret;
}

int
main (int argc, char **argv)
{
	GdkWindow *root;
	GdkPixbuf *pixbuf;
	gboolean ret;
	gint retval = 1;
	GError *error = NULL;

	gtk_init (&argc, &argv);

	gtk_widget_set_default_colormap (gdk_rgb_get_colormap ());

	root = gdk_get_default_root_window ();
	pixbuf = gdk_pixbuf_get_from_drawable (NULL, root, NULL,
					       0, 0, 0, 0, 150, 160);

	/* PASS */
	g_debug ("try to render color corrected image");
	ret = transform_image ("icc-profile-fish.png", &error);
	if (!ret) {
		g_warning ("FAILED: did not load image: %s", error->message);
		g_error_free (error);
		goto out;
	}

	/* PASS */
	g_debug ("try to save PNG with a profile");
	ret = save_image_png ("icc-profile.png", pixbuf, &error);
	if (!ret) {
		g_warning ("FAILED: did not save image: %s", error->message);
		g_error_free (error);
		goto out;
	}

	/* PASS */
	g_debug ("try to save TIFF with a profile");
	ret = save_image_tiff ("icc-profile.tiff", pixbuf, &error);
	if (!ret) {
		g_warning ("FAILED: did not save image: %s", error->message);
		g_error_free (error);
		goto out;
	}

	/* PASS */
	g_debug ("try to load PNG and get color attributes");
	ret = save_image_verify ("icc-profile.png", &error);
	if (!ret) {
		g_warning ("FAILED: did not load image: %s", error->message);
		g_error_free (error);
		goto out;
	}

	/* PASS */
	g_debug ("try to load TIFF and get color attributes");
	ret = save_image_verify ("icc-profile.tiff", &error);
	if (!ret) {
		g_warning ("FAILED: did not load image: %s", error->message);
		g_error_free (error);
		goto out;
	}

	/* success */
	retval = 0;
	g_debug ("ALL OKAY!");
out:
	return retval;
}
