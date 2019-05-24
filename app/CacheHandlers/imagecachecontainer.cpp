#include "imagecachecontainer.h"
#include "tmpfilehandlers.h"
#include "canvas.h"
#include "skia/skiahelpers.h"

ImageCacheContainer::ImageCacheContainer(const FrameRange &range,
                                         HDDCachableCacheHandler * const parent) :
    HDDCachableRangeContainer(range, parent) {}

ImageCacheContainer::ImageCacheContainer(const sk_sp<SkImage> &img,
                                         const FrameRange &range,
                                         HDDCachableCacheHandler * const parent) :
    ImageCacheContainer(range, parent) {
    replaceImageSk(img);
}

void ImageCacheContainer::replaceImageSk(const sk_sp<SkImage> &img) {
    mImageSk = img;
    afterDataReplaced();
}

int ImageCacheContainer::getByteCount() {
    if(!mImageSk) return 0;
    SkPixmap pixmap;
    if(mImageSk->peekPixels(&pixmap)) {
        return pixmap.width()*pixmap.height()*
                pixmap.info().bytesPerPixel();
    }
    return 0;
}

sk_sp<SkImage> ImageCacheContainer::getImageSk() {
    return mImageSk;
}

void ImageCacheContainer::setDataLoadedFromTmpFile(const sk_sp<SkImage> &img) {
    replaceImageSk(img);

    if(mTmpLoadTargetCanvas) {
        mTmpLoadTargetCanvas->setCurrentPreviewContainer(
                    ref<ImageCacheContainer>());
        mTmpLoadTargetCanvas = nullptr;
    }
    afterDataLoadedFromTmpFile();
}

void ImageCacheContainer::drawSk(SkCanvas * const canvas,
                                 GrContext * const grContext) {
    SkPaint paint;
    //paint.setAntiAlias(true);
    paint.setFilterQuality(BoundingBox::sDisplayQuality);
    SkiaHelpers::drawImageGPU(canvas, mImageSk, 0, 0, &paint, grContext);
    //canvas->drawImage(mImageSk, 0, 0/*, &paint*/);
}

int ImageCacheContainer::clearMemory() {
    const int bytes = getByteCount();
    mImageSk.reset();
    return bytes;
}

void ImageCacheContainer::setLoadTargetCanvas(Canvas *canvas) {
    mTmpLoadTargetCanvas = canvas;
}

stdsptr<HDDTask> ImageCacheContainer::createTmpFileDataSaver() {
    const ImgTmpFileDataSaver::Func func = [this](qsptr<QTemporaryFile> tmpFile) {
        setDataSavedToTmpFile(tmpFile);
    };
    return SPtrCreate(ImgTmpFileDataSaver)(mImageSk, func);
}

stdsptr<HDDTask> ImageCacheContainer::createTmpFileDataLoader() {
    const ImgTmpFileDataLoader::Func func = [this](sk_sp<SkImage> img) {
        setDataLoadedFromTmpFile(img);
    };
    return SPtrCreate(ImgTmpFileDataLoader)(mTmpFile, func);
}
