#include "sampledmotionblureffect.h"
#include "Animators/qrealanimator.h"
#include "Boxes/boundingbox.h"

SampledMotionBlurEffect::SampledMotionBlurEffect(BoundingBox *box) :
    PixmapEffect("motion blur", EFFECT_MOTION_BLUR) {
    mParentBox = box;
    mOpacity = SPtrCreate(QrealAnimator)("opacity");
    mOpacity->qra_setValueRange(0., 100.);
    mOpacity->qra_setCurrentValue(100.);

    mNumberSamples = SPtrCreate(QrealAnimator)("number of samples");
    mNumberSamples->qra_setValueRange(0.1, 99.);
    mNumberSamples->qra_setCurrentValue(1.);

    mFrameStep = SPtrCreate(QrealAnimator)("frame step");
    mFrameStep->qra_setValueRange(.1, 9.9);
    mFrameStep->qra_setCurrentValue(1.);

    ca_addChildAnimator(mOpacity);
    ca_addChildAnimator(mNumberSamples);
    ca_addChildAnimator(mFrameStep);
}

stdsptr<PixmapEffectRenderData> SampledMotionBlurEffect::
getPixmapEffectRenderDataForRelFrameF(const qreal &relFrame,
                                     BoundingBoxRenderData* data) {
    if(!data->fParentIsTarget) return nullptr;
    auto renderData = SPtrCreate(SampledMotionBlurEffectRenderData)();
    renderData->opacity =
            mOpacity->getCurrentEffectiveValueAtRelFrameF(relFrame)*0.01;
    renderData->numberSamples =
            mNumberSamples->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->boxData = data;

    int numberFrames = qCeil(renderData->numberSamples);
    qreal frameStep = mFrameStep->getCurrentEffectiveValueAtRelFrameF(relFrame);
    qreal relFrameT = relFrame - numberFrames*frameStep;
    for(int i = 0; i < numberFrames; i++) {
        if(!mParentBox->isRelFrameFVisibleAndInVisibleDurationRect(relFrameT)) {
            if(i == numberFrames - 1) {
                renderData->numberSamples = qRound(renderData->numberSamples - 0.500001);
            } else {
                renderData->numberSamples -= 1;
            }
            relFrameT += frameStep;
            continue;
        }
        stdsptr<BoundingBoxRenderData> sampleRenderData =
                mParentBox->createRenderData();
        //mParentBox->setupBoundingBoxRenderDataForRelFrameF(i, sampleRenderData);
        sampleRenderData->fParentIsTarget = false;
        sampleRenderData->fUseCustomRelFrame = true;
        sampleRenderData->fCustomRelFrame = relFrameT;
        sampleRenderData->fMotionBlurTarget = data;
        sampleRenderData->scheduleTask();
        sampleRenderData->addDependent(data);
        renderData->samples << sampleRenderData;

        relFrameT += frameStep;
    }
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

FrameRange SampledMotionBlurEffect::getParentBoxFirstLastMarginAjusted(const int &relFrame) {
    auto boxRange = mParentBox->getFirstAndLastIdenticalForMotionBlur(relFrame);
    int margin = qCeil(mNumberSamples->getCurrentEffectiveValueAtRelFrame(relFrame)*
                       mFrameStep->getCurrentEffectiveValueAtRelFrame(relFrame));
    if(boxRange.min == INT_MIN) {
        if(boxRange.max != INT_MAX) {
            if(boxRange.max - margin < relFrame) {
                boxRange.min = relFrame;
            }
        }
    } else {
        boxRange.min += margin;
    }
    if(boxRange.max == INT_MAX) {
        if(boxRange.min != INT_MIN) {
            if(boxRange.min > relFrame) {
                boxRange.max = relFrame;
            }
        }
    } else {
        boxRange.max -= margin;
    }
    return boxRange;
}

FrameRange SampledMotionBlurEffect::prp_getFirstAndLastIdenticalRelFrame(const int &relFrame) {
    auto boxRange = getParentBoxFirstLastMarginAjusted(relFrame);
    auto effectRange = PixmapEffect::prp_getFirstAndLastIdenticalRelFrame(relFrame);
    return boxRange*effectRange;
}

void replaceIfHigherAlpha(const int &x0, const int &y0,
                          const SkBitmap &dst,
                          const sk_sp<SkImage> &src,
                          const qreal &alphaT) {
    SkPixmap dstP;
    SkPixmap srcP;
    dst.peekPixels(&dstP);
    src->peekPixels(&srcP);
    uint8_t *dstD = static_cast<uint8_t*>(dstP.writable_addr());
    uint8_t *srcD = static_cast<uint8_t*>(srcP.writable_addr());
    for(int y = 0; y < src->height() && y + y0 < dst.height(); y++) {
        for(int x = 0; x < src->width() && x + x0 < dst.width(); x++) {
            int dstRId = ((y + y0) * dst.width() + (x + x0))*4;
            int srcRId = (y * src->width() + x)*4;
            uchar dstAlpha = dstD[dstRId + 3];
            uchar srcAlpha =
                    static_cast<uchar>(qRound(srcD[srcRId + 3]*alphaT));
            if(dstAlpha >= srcAlpha) continue;
            dstD[dstRId] =
                    static_cast<uint8_t>(qRound(srcD[srcRId]*alphaT));
            dstD[dstRId + 1] =
                    static_cast<uint8_t>(qRound(srcD[srcRId + 1]*alphaT));
            dstD[dstRId + 2] =
                    static_cast<uint8_t>(qRound(srcD[srcRId + 2]*alphaT));
            dstD[dstRId + 3] = srcAlpha;
        }
    }
}

void replaceIfHigherAlpha(const int &x0, const int &y0,
                          const SkBitmap &dst,
                          const SkBitmap &src) {
    SkPixmap dstP;
    SkPixmap srcP;
    dst.peekPixels(&dstP);
    src.peekPixels(&srcP);
    uint8_t *dstD = static_cast<uint8_t*>(dstP.writable_addr());
    uint8_t *srcD = static_cast<uint8_t*>(srcP.writable_addr());
    for(int y = 0; y < src.height() && y + y0 < dst.height(); y++) {
        for(int x = 0; x < src.width() && x + x0 < dst.width(); x++) {
            int dstRId = ((y + y0) * dst.width() + (x + x0))*4;
            int srcRId = (y * src.width() + x)*4;
            uchar dstAlpha = dstD[dstRId + 3];
            uchar srcAlpha = srcD[srcRId + 3];
            if(dstAlpha >= srcAlpha) continue;
            dstD[dstRId] = srcD[srcRId];
            dstD[dstRId + 1] = srcD[srcRId + 1];
            dstD[dstRId + 2] = srcD[srcRId + 2];
            dstD[dstRId + 3] = srcAlpha;
        }
    }
}

void SampledMotionBlurEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                                       const qreal &scale) {
    Q_UNUSED(scale)
    SkBitmap motionBlur;
    motionBlur.allocPixels(bitmap.info());
    motionBlur.eraseColor(SK_ColorTRANSPARENT);
    //SkCanvas canvasSk(motionBlur);
    qreal opacityStepT = 1./(numberSamples + 1);
    qreal opacityT = opacityStepT*(1. - qCeil(numberSamples) + numberSamples);
    foreach(const stdsptr<BoundingBoxRenderData> &sample, samples) {
        qreal sampleAlpha = opacityT*opacityT*opacity;
        QPointF drawPosF = sample->fGlobalBoundingRect.topLeft() -
                boxData->fGlobalBoundingRect.topLeft();//QPointF(0., 0.);
        QPoint drawPos = drawPosF.toPoint();
        replaceIfHigherAlpha(drawPos.x(), drawPos.y(),
                             motionBlur, sample->renderedImage,
                             sampleAlpha);
        opacityT += opacityStepT;
    }

    replaceIfHigherAlpha(0, 0, bitmap, motionBlur);
//    SkCanvas canvasSk2(imgPtr);
//    SkPaint paintT;
//    paintT.setBlendMode(SkBlendMode::kDstOver);
//    canvasSk2.drawBitmap(motionBlur, 0.f, 0.f, &paintT);
}
