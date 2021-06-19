#pragma once

#include <gtk/gtk.h>

GdkPaintable * import_dmabuf_egl (GdkGLContext   *context,
                                  uint32_t        format,
                                  unsigned int    width,
                                  unsigned int    height,
                                  uint32_t        n_planes,
                                  const int      *fds,
                                  const uint32_t *strides,
                                  const uint32_t *offsets,
                                  const uint64_t *modifiers);
