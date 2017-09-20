#include "linkbox.h"
#include <QFileDialog>
#include "mainwindow.h"
#include "canvas.h"
#include "durationrectangle.h"

ExternalLinkBox::ExternalLinkBox() :
    BoxesGroup() {
    setType(TYPE_EXTERNAL_LINK);
    setName("Link Empty");
}

void ExternalLinkBox::reload() {


    scheduleUpdate();
}

void ExternalLinkBox::changeSrc() {
    QString src = QFileDialog::getOpenFileName(mMainWindow,
                                               "Link File",
                                               "",
                                               "AniVect Files (*.av)");
    if(!src.isEmpty()) {
        setSrc(src);
    }
}

void ExternalLinkBox::setSrc(const QString &src) {
    mSrc = src;
    setName("Link " + src);
    reload();
}

QPointF InternalLinkBox::getRelCenterPosition() {
    return mLinkTarget->getRelCenterPosition();
}

qreal InternalLinkBox::getEffectsMargin() {
    return mLinkTarget->getEffectsMargin();
}

BoundingBox *InternalLinkBox::getLinkTarget() {
    return mLinkTarget.data();
}

BoundingBox *InternalLinkBox::createLink() {
    return mLinkTarget->createLink();
}

BoundingBoxRenderData *InternalLinkBox::createRenderData() {
    BoundingBoxRenderData *renderData = mLinkTarget->createRenderData();
    renderData->parentBox = weakRef<BoundingBox>();
    return renderData;
}

void InternalLinkBox::setupBoundingBoxRenderDataForRelFrame(
        const int &relFrame, BoundingBoxRenderData *data) {
    mLinkTarget->setupBoundingBoxRenderDataForRelFrame(relFrame, data);
    BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame, data);
}

void InternalLinkBox::addSchedulersToProcess() {
    mLinkTarget->addSchedulersToProcess();
    BoundingBox::addSchedulersToProcess();
}

void InternalLinkBox::processSchedulers() {
    mLinkTarget->processSchedulers();
    BoundingBox::processSchedulers();
}

QRectF InternalLinkBox::getRelBoundingRectAtRelFrame(const int &relFrame) {
    int absFrame = prp_relFrameToAbsFrame(relFrame);
    int relFrameLT = mLinkTarget->prp_absFrameToRelFrame(absFrame);
    return mLinkTarget->getRelBoundingRectAtRelFrame(relFrameLT);
}

InternalLinkBox::InternalLinkBox(BoundingBox *linkTarget) :
    BoundingBox(TYPE_INTERNAL_LINK) {
    setLinkTarget(linkTarget);
}

bool InternalLinkBox::relPointInsidePath(const QPointF &point) {
    return mLinkTarget->relPointInsidePath(point);
}

void InternalLinkBox::prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                                        int *lastIdentical,
                                                        const int &relFrame) {
    int fIdLT;
    int lIdLT;
//    int relFrameLT = mLinkTarget->prp_absFrameToRelFrame(
//                prp_relFrameToAbsFrame(relFrame));
    mLinkTarget->prp_getFirstAndLastIdenticalRelFrame(&fIdLT,
                                                      &lIdLT,
                                                      relFrame);
    int fId;
    int lId;
    if(mVisible) {
        if(isRelFrameInVisibleDurationRect(relFrame)) {
            Animator::prp_getFirstAndLastIdenticalRelFrame(&fId,
                                                            &lId,
                                                            relFrame);
        } else {
            if(relFrame > mDurationRectangle->getMaxFrameAsRelFrame()) {
                fId = mDurationRectangle->getMaxFrameAsRelFrame();
                lId = INT_MAX;
            } else if(relFrame < mDurationRectangle->getMinFrameAsRelFrame()) {
                fId = INT_MIN;
                lId = mDurationRectangle->getMinFrameAsRelFrame();
            }
        }
    } else {
        fId = INT_MIN;
        lId = INT_MAX;
    }
    fId = qMax(fId, fIdLT);
    lId = qMin(lId, lIdLT);
    if(lId > fId) {
        *firstIdentical = fId;
        *lastIdentical = lId;
    } else {
        *firstIdentical = relFrame;
        *lastIdentical = relFrame;
    }
}

bool InternalLinkBox::prp_differencesBetweenRelFrames(const int &relFrame1,
                                                      const int &relFrame2) {
    int relFrame1LT = mLinkTarget->prp_absFrameToRelFrame(
                prp_relFrameToAbsFrame(relFrame1));
    int relFrame2LT = mLinkTarget->prp_absFrameToRelFrame(
                prp_relFrameToAbsFrame(relFrame2));
    bool differences =
            ComplexAnimator::prp_differencesBetweenRelFrames(relFrame1,
                                                             relFrame2) ||
            mLinkTarget->prp_differencesBetweenRelFrames(relFrame1LT,
                                                         relFrame2LT);
    if(differences || mDurationRectangle == NULL) return differences;
    return mDurationRectangle->hasAnimationFrameRange();
}

void InternalLinkCanvas::setClippedToCanvasSize(const bool &clipped) {
    mClipToCanvasSize->setValue(clipped);
    scheduleUpdate();
}

QRectF InternalLinkCanvas::getRelBoundingRectAtRelFrame(const int &relFrame) {
    if(mClipToCanvasSize) {
        return mLinkTarget->getRelBoundingRectAtRelFrame(relFrame);
    }
    return ((Canvas*)mLinkTarget.data())->BoxesGroup::
            getRelBoundingRectAtRelFrame(relFrame);
}


//void InternalLinkCanvas::draw(QPainter *p) {
//    p->save();
//    if(mClipToCanvasSize) {
//        p->setClipRect(mRelBoundingRect);
//    }
//    p->setTransform(QTransform(getCombinedTransform().inverted()), true);
//    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
//        box->drawPixmap(p);
//    }

//    p->restore();
//}
