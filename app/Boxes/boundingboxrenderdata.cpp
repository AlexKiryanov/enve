#include "boundingboxrenderdata.h"
#include "boundingbox.h"
#include "PixmapEffects/rastereffects.h"
#include "GPUEffects/gpupostprocessor.h"
#include "skia/skiahelpers.h"
#include "PixmapEffects/pixmapeffect.h"

BoundingBoxRenderData::BoundingBoxRenderData(BoundingBox *parentBoxT) {
    fParentBox = parentBoxT;
}

void BoundingBoxRenderData::copyFrom(BoundingBoxRenderData *src) {
    fGlobalBoundingRect = src->fGlobalBoundingRect;
    fTransform = src->fTransform;
    fParentTransform = src->fParentTransform;
    fCustomRelFrame = src->fCustomRelFrame;
    fUseCustomRelFrame = src->fUseCustomRelFrame;
    fRelFrame = src->fRelFrame;
    fRelBoundingRect = src->fRelBoundingRect;
    fRelTransform = src->fRelTransform;
    fRenderedToImage = src->fRenderedToImage;
    fBlendMode = src->fBlendMode;
    fDrawPos = src->fDrawPos;
    fOpacity = src->fOpacity;
    fResolution = src->fResolution;
    fRenderedImage = SkiaHelpers::makeSkImageCopy(src->fRenderedImage);
    mFinished = true;
    fRelBoundingRectSet = true;
    fCopied = true;
}

stdsptr<BoundingBoxRenderData> BoundingBoxRenderData::makeCopy() {
    if(!fParentBox) return nullptr;
    stdsptr<BoundingBoxRenderData> copy = fParentBox->createRenderData();
    copy->copyFrom(this);
    return copy;
}

void BoundingBoxRenderData::updateRelBoundingRect() {
    if(!fParentBox) return;
    fRelBoundingRect = fParentBox->getRelBoundingRectAtRelFrame(fRelFrame);
}

void BoundingBoxRenderData::drawRenderedImageForParent(SkCanvas *canvas) {
    if(fOpacity < 0.001) return;
    canvas->save();
    SkScalar invScale = qrealToSkScalar(1/fResolution);
    canvas->scale(invScale, invScale);
    renderToImage();
    SkPaint paint;
    paint.setAlpha(static_cast<U8CPU>(qRound(fOpacity*2.55)));
    paint.setBlendMode(fBlendMode);
    //paint.setAntiAlias(true);
    //paint.setFilterQuality(kHigh_SkFilterQuality);
    if(fBlendMode == SkBlendMode::kDstIn ||
       fBlendMode == SkBlendMode::kSrcIn ||
       fBlendMode == SkBlendMode::kDstATop) {
        SkPaint paintT;
        paintT.setBlendMode(fBlendMode);
        paintT.setColor(SK_ColorTRANSPARENT);
        SkPath path;
        path.addRect(SkRect::MakeXYWH(fDrawPos.x(), fDrawPos.y(),
                                      fRenderedImage->width(),
                                      fRenderedImage->height()));
        path.toggleInverseFillType();
        canvas->drawPath(path, paintT);
    }
    canvas->drawImage(fRenderedImage,
                      fDrawPos.x(), fDrawPos.y(),
                      &paint);
    canvas->restore();
}

void BoundingBoxRenderData::renderToImage() {
    if(fRenderedToImage) return;
    fRenderedToImage = true;
    if(fOpacity < 0.001) return;
    QMatrix scale;
    scale.scale(fResolution, fResolution);
    fScaledTransform = fTransform*scale;
    //transformRes.scale(resolution, resolution);
    fGlobalBoundingRect = fScaledTransform.mapRect(fRelBoundingRect);
    foreach(const QRectF &rectT, fOtherGlobalRects) {
        fGlobalBoundingRect = fGlobalBoundingRect.united(rectT);
    }
    fGlobalBoundingRect = fGlobalBoundingRect.
            adjusted(-fEffectsMargin, -fEffectsMargin,
                     fEffectsMargin, fEffectsMargin);
    if(fMaxBoundsEnabled) {
        fGlobalBoundingRect = fGlobalBoundingRect.intersected(
                              scale.mapRect(fMaxBoundsRect));
    }
    QPointF transF = fGlobalBoundingRect.topLeft()/**resolution*/ -
            QPointF(qRound(fGlobalBoundingRect.left()/**resolution*/),
                    qRound(fGlobalBoundingRect.top()/**resolution*/));
    fGlobalBoundingRect.translate(-transF);

    const auto info = SkiaHelpers::getPremulBGRAInfo(
                qCeil(fGlobalBoundingRect.width()),
                qCeil(fGlobalBoundingRect.height()));
    fBitmapTMP.allocPixels(info);
    fBitmapTMP.eraseColor(SK_ColorTRANSPARENT);
    //sk_sp<SkSurface> rasterSurface(SkSurface::MakeRaster(info));
    SkCanvas rasterCanvas(fBitmapTMP);//rasterSurface->getCanvas();
    //rasterCanvas->clear(SK_ColorTRANSPARENT);

    rasterCanvas.translate(qrealToSkScalar(-fGlobalBoundingRect.left()),
                           qrealToSkScalar(-fGlobalBoundingRect.top()));
    rasterCanvas.concat(QMatrixToSkMatrix(fScaledTransform));

    drawSk(&rasterCanvas);
    // rasterCanvas.flush(); does not use gpu anyway

    fDrawPos = SkPoint::Make(qRound(fGlobalBoundingRect.left()),
                             qRound(fGlobalBoundingRect.top()));

    if(!fPixmapEffects.isEmpty()) {
        foreach(const auto& effect, fPixmapEffects) {
            effect->applyEffectsSk(fBitmapTMP, fResolution);
        }
        clearPixmapEffects();
    }
    fBitmapTMP.setImmutable();
    fRenderedImage = SkImage::MakeFromBitmap(fBitmapTMP);
    fBitmapTMP.reset();
}

void BoundingBoxRenderData::_processUpdate() {
    renderToImage();
}

void BoundingBoxRenderData::beforeProcessingStarted() {
    if(!mDataSet) dataSet();

    _ScheduledTask::beforeProcessingStarted();

    if(!fParentBox || !fParentIsTarget) return;
    if(nullifyBeforeProcessing())
        fParentBox->nullifyCurrentRenderData(fRelFrame);
}

void BoundingBoxRenderData::afterProcessingFinished() {
    if(fMotionBlurTarget) {
        fMotionBlurTarget->fOtherGlobalRects << fGlobalBoundingRect;
    }
    if(fParentBox && fParentIsTarget) {
        //qDebug() << fParentBox->prp_getName();
        fParentBox->renderDataFinished(this);
    }
}

void BoundingBoxRenderData::taskQued() {
    if(fParentBox) {
        if(fUseCustomRelFrame) {
            fParentBox->setupBoundingBoxRenderDataForRelFrameF(
                        fCustomRelFrame, this);
        } else {
            fParentBox->setupBoundingBoxRenderDataForRelFrameF(
                        fRelFrame, this);
        }
        foreach(const auto& customizer, mRenderDataCustomizerFunctors) {
            (*customizer)(this);
        }
    }
    mDataSet = false;
    if(!mDelayDataSet) dataSet();
    _ScheduledTask::taskQued();
}

void BoundingBoxRenderData::scheduleTaskNow() {
    if(!fParentBox) {
        clear();
        return;
    }
    fParentBox->scheduleTask(GetAsSPtr(this, _ScheduledTask));
}

void BoundingBoxRenderData::dataSet() {
    if(allDataReady()) {
        mDataSet = true;
        if(!fRelBoundingRectSet) {
            fRelBoundingRectSet = true;
            updateRelBoundingRect();
        }
        if(!fParentBox || !fParentIsTarget) return;
        fParentBox->updateCurrentPreviewDataFromRenderData(this);
    }
}

bool BoundingBoxRenderData::nullifyBeforeProcessing() {
    return fReason != BoundingBox::USER_CHANGE &&
            fReason != BoundingBox::CHILD_USER_CHANGE;
}

RenderDataCustomizerFunctor::RenderDataCustomizerFunctor() {}

void RenderDataCustomizerFunctor::operator()(BoundingBoxRenderData* data) {
    customize(data);
}

ReplaceTransformDisplacementCustomizer::ReplaceTransformDisplacementCustomizer(
        const qreal &dx, const qreal &dy) {
    mDx = dx;
    mDy = dy;
}

void ReplaceTransformDisplacementCustomizer::customize(
        BoundingBoxRenderData* data) {
    QMatrix transformT = data->fTransform;
    data->fTransform.setMatrix(transformT.m11(), transformT.m12(),
                              transformT.m21(), transformT.m22(),
                              mDx, mDy);
}

MultiplyTransformCustomizer::MultiplyTransformCustomizer(
        const QMatrix &transform, const qreal &opacity) {
    mTransform = transform;
    mOpacity = opacity;
}

void MultiplyTransformCustomizer::customize(BoundingBoxRenderData *data) {
    data->fTransform = mTransform*data->fTransform;
    data->fOpacity *= mOpacity;
}

MultiplyOpacityCustomizer::MultiplyOpacityCustomizer(const qreal &opacity) {
    mOpacity = opacity;
}

void MultiplyOpacityCustomizer::customize(BoundingBoxRenderData *data) {
    data->fOpacity *= mOpacity;
}
