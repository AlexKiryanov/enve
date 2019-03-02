#ifndef SKIAHELPERS_H
#define SKIAHELPERS_H

#include "skiaincludes.h"

namespace SkiaHelpers {
    sk_sp<SkImage> makeSkImageCopy(const sk_sp<SkImage>& img);
    void drawImageGPU(SkCanvas* const canvas,
                      const sk_sp<SkImage>& image,
                      const SkScalar& x,
                      const SkScalar& y,
                      SkPaint * const paint,
                      GrContext* const context);
    SkImageInfo getPremulBGRAInfo(const int& width,
                                  const int& height);
}

#endif // SKIAHELPERS_H
