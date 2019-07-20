#include "linkcanvasrenderdata.h"
#include "skia/skiahelpers.h"
#include "PixmapEffects/pixmapeffect.h"
#include "skia/skqtconversions.h"

void LinkCanvasRenderData::processTask() {
    updateGlobalRect();
    if(fOpacity < 0.001) return;

    const auto info = SkiaHelpers::getPremulRGBAInfo(fGlobalRect.width(),
                                                     fGlobalRect.height());
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    bitmap.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas rasterCanvas(bitmap);
    rasterCanvas.translate(toSkScalar(-fGlobalRect.left()),
                           toSkScalar(-fGlobalRect.top()));

    if(fClipToCanvas) {
        rasterCanvas.save();
        rasterCanvas.concat(toSkMatrix(fScaledTransform));
        SkPaint fillP;
        fillP.setAntiAlias(true);
        fillP.setColor(fBgColor);
        rasterCanvas.drawRect(toSkRect(fRelBoundingRect), fillP);
        rasterCanvas.restore();
    }

    drawSk(&rasterCanvas);
    if(fClipToCanvas) {
        rasterCanvas.save();
        rasterCanvas.concat(toSkMatrix(fScaledTransform));
        SkPaint paintT;
        paintT.setBlendMode(SkBlendMode::kDstIn);
        paintT.setColor(SK_ColorTRANSPARENT);
        paintT.setAntiAlias(true);
        SkPath path;
        path.addRect(toSkRect(fRelBoundingRect));
        path.toggleInverseFillType();
        rasterCanvas.drawPath(path, paintT);
        rasterCanvas.restore();
    }
    rasterCanvas.flush();

    if(!fRasterEffects.isEmpty()) {
        for(const auto& effect : fRasterEffects) {
            effect->applyEffectsSk(bitmap, fResolution);
        }
        clearPixmapEffects();
    }

    fRenderedImage = SkiaHelpers::transferDataToSkImage(bitmap);
}
