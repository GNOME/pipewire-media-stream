#include <gtk/gtk.h>

#ifdef GDK_WINDOWING_X11
#include <gdk/x11/gdkx.h>
#endif
#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/wayland/gdkwayland.h>
#endif

#include <epoxy/egl.h>

GdkPaintable *
import_dmabuf_egl (GdkGLContext   *context,
                   uint32_t        format,
                   unsigned int    width,
                   unsigned int    height,
                   uint32_t        n_planes,
                   const int      *fds,
                   const uint32_t *strides,
                   const uint32_t *offsets,
                   const uint64_t *modifiers)
{
  GdkDisplay *display;
  EGLDisplay egl_display;
  EGLint attribs[2 * (3 + 4 * 5) + 1];
  EGLImage image;
  guint texture_id;
  int i;

  display = gdk_gl_context_get_display (context);
  egl_display = NULL;

#ifdef GDK_WINDOWING_WAYLAND
  if (GDK_IS_WAYLAND_DISPLAY (display))
    egl_display = gdk_wayland_display_get_egl_display (display);
#endif
#ifdef GDK_WINDOWING_X11
  if (GDK_IS_X11_DISPLAY (display))
    egl_display = gdk_x11_display_get_egl_display (display);
#endif

  if (!egl_display)
    {
      g_warning ("Can't import DMA-BUF when not using EGL");
      return NULL;
    }

  i = 0;
  attribs[i++] = EGL_WIDTH;
  attribs[i++] = width;
  attribs[i++] = EGL_HEIGHT;
  attribs[i++] = height;
  attribs[i++] = EGL_LINUX_DRM_FOURCC_EXT;
  attribs[i++] = format;

  if (n_planes > 0)
    {
      attribs[i++] = EGL_DMA_BUF_PLANE0_FD_EXT;
      attribs[i++] = fds[0];
      attribs[i++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
      attribs[i++] = offsets[0];
      attribs[i++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
      attribs[i++] = strides[0];
      if (modifiers)
        {
          attribs[i++] = EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT;
          attribs[i++] = modifiers[0] & 0xFFFFFFFF;
          attribs[i++] = EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT;
          attribs[i++] = modifiers[0] >> 32;
        }
    }

  if (n_planes > 1)
    {
      attribs[i++] = EGL_DMA_BUF_PLANE1_FD_EXT;
      attribs[i++] = fds[1];
      attribs[i++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
      attribs[i++] = offsets[1];
      attribs[i++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
      attribs[i++] = strides[1];
      if (modifiers)
        {
          attribs[i++] = EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT;
          attribs[i++] = modifiers[1] & 0xFFFFFFFF;
          attribs[i++] = EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT;
          attribs[i++] = modifiers[1] >> 32;
        }
    }

  if (n_planes > 2)
    {
      attribs[i++] = EGL_DMA_BUF_PLANE2_FD_EXT;
      attribs[i++] = fds[2];
      attribs[i++] = EGL_DMA_BUF_PLANE2_OFFSET_EXT;
      attribs[i++] = offsets[2];
      attribs[i++] = EGL_DMA_BUF_PLANE2_PITCH_EXT;
      attribs[i++] = strides[2];
      if (modifiers)
        {
          attribs[i++] = EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT;
          attribs[i++] = modifiers[2] & 0xFFFFFFFF;
          attribs[i++] = EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT;
          attribs[i++] = modifiers[2] >> 32;
        }
    }
  if (n_planes > 3)
    {
      attribs[i++] = EGL_DMA_BUF_PLANE3_FD_EXT;
      attribs[i++] = fds[3];
      attribs[i++] = EGL_DMA_BUF_PLANE3_OFFSET_EXT;
      attribs[i++] = offsets[3];
      attribs[i++] = EGL_DMA_BUF_PLANE3_PITCH_EXT;
      attribs[i++] = strides[3];
      if (modifiers)
        {
          attribs[i++] = EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT;
          attribs[i++] = modifiers[3] & 0xFFFFFFFF;
          attribs[i++] = EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT;
          attribs[i++] = modifiers[3] >> 32;
        }
    }

  attribs[i++] = EGL_NONE;

  image = eglCreateImageKHR (egl_display,
                             EGL_NO_CONTEXT,
                             EGL_LINUX_DMA_BUF_EXT,
                             (EGLClientBuffer)NULL,
                             attribs);
  if (image == EGL_NO_IMAGE)
    {
      g_warning ("Failed to create EGL image: %d\n", eglGetError ());
      return 0;
    }

  gdk_gl_context_make_current (context);

  glGenTextures (1, &texture_id);
  glBindTexture (GL_TEXTURE_2D, texture_id);
  glEGLImageTargetTexture2DOES (GL_TEXTURE_2D, image);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  eglDestroyImageKHR (egl_display, image);

  return GDK_PAINTABLE (gdk_gl_texture_new (context,
                                            texture_id,
                                            width, height,
                                            NULL, NULL));
}
