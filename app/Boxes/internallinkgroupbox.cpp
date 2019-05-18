#include "internallinkgroupbox.h"
#include "Animators/transformanimator.h"
#include "durationrectangle.h"
#include "layerboxrenderdata.h"

InternalLinkGroupBox::InternalLinkGroupBox(LayerBox* linkTarget) :
    LayerBox(TYPE_INTERNAL_LINK_GROUP) {
    setLinkTarget(linkTarget);

    ca_prependChildAnimator(mTransformAnimator.data(), mBoxTarget);
    connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
            this, &InternalLinkGroupBox::setTargetSlot);
}

InternalLinkGroupBox::~InternalLinkGroupBox() {
    setLinkTarget(nullptr);
}

void InternalLinkGroupBox::writeBoundingBox(QIODevice * const target) {
    mBoxTarget->writeProperty(target);
}

void InternalLinkGroupBox::readBoundingBox(QIODevice * const target) {
    mBoxTarget->readProperty(target);
}

//bool InternalLinkGroupBox::relPointInsidePath(const QPointF &relPos) {
//    return mLinkTarget->relPointInsidePath(point);
//}

FrameRange InternalLinkGroupBox::prp_getIdenticalRelRange(
        const int &relFrame) const {
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
    if(mVisible) {
        if(isFrameInDurationRect(relFrame)) {
            range *= BoundingBox::prp_getIdenticalRelRange(relFrame);
        } else {
            if(relFrame > mDurationRectangle->getMaxFrameAsRelFrame()) {
                range = mDurationRectangle->getRelFrameRangeToTheRight();
            } else if(relFrame < mDurationRectangle->getMinFrameAsRelFrame()) {
                range = mDurationRectangle->getRelFrameRangeToTheLeft();
            }
        }
    }
    if(!getLinkTarget()) return range;
    auto targetRange = getLinkTarget()->prp_getIdenticalRelRange(relFrame);

    return range*targetRange;
}

bool InternalLinkGroupBox::SWT_isBoxesGroup() const { return false; }

QMatrix InternalLinkGroupBox::getRelativeTransformAtRelFrameF(
        const qreal &relFrame) {
    if(!getLinkTarget() || !mParentGroup)
        return BoundingBox::getRelativeTransformAtRelFrameF(relFrame);
    if(mParentGroup->SWT_isLinkBox()) {
        return getLinkTarget()->getRelativeTransformAtRelFrameF(relFrame);
    } else {
        return BoundingBox::getRelativeTransformAtRelFrameF(relFrame);
    }
}

void InternalLinkGroupBox::setupEffectsF(const qreal &relFrame,
                                         BoundingBoxRenderData * const data) {
    if(!getLinkTarget() || !mParentGroup)
        return BoundingBox::setupEffectsF(relFrame, data);
    if(mParentGroup->SWT_isLinkBox()) {
        getLinkTarget()->setupEffectsF(relFrame, data);
    } else {
        BoundingBox::setupEffectsF(relFrame, data);
    }
}

qreal InternalLinkGroupBox::getEffectsMarginAtRelFrameF(const qreal &relFrame) {
    if(!getLinkTarget()) return 0;
    if(mParentGroup->SWT_isLinkBox()) {
        return getLinkTarget()->getEffectsMarginAtRelFrameF(relFrame);
    }
    return LayerBox::getEffectsMarginAtRelFrameF(relFrame);
}

const SkBlendMode &InternalLinkGroupBox::getBlendMode() {
    if(!getLinkTarget()) return BoundingBox::getBlendMode();
    if(mParentGroup->SWT_isLinkBox()) {
        return getLinkTarget()->getBlendMode();
    }
    return BoundingBox::getBlendMode();
}

void InternalLinkGroupBox::setupRenderData(const qreal &relFrame,
                                           BoundingBoxRenderData * const data) {
    const auto linkTarget = getLinkTarget();
    if(linkTarget) {
        linkTarget->BoundingBox::
                setupRenderData(relFrame, data);
    }

    LayerBox::setupRenderData(relFrame, data);
    if(linkTarget) {
        const qreal targetMargin =
                linkTarget->getEffectsMarginAtRelFrameF(relFrame);
        data->fEffectsMargin += targetMargin*data->fResolution;
    }
}

LayerBox *InternalLinkGroupBox::getFinalTarget() const {
    if(!getLinkTarget()) return nullptr;
    if(getLinkTarget()->SWT_isLinkBox()) {
        return GetAsPtr(getLinkTarget(), InternalLinkGroupBox)->getFinalTarget();
    }
    return getLinkTarget();
}

int InternalLinkGroupBox::prp_getRelFrameShift() const {
    if(!getLinkTarget()) return 0;
    if(getLinkTarget()->SWT_isLinkBox() ||
       (mParentGroup ? mParentGroup->SWT_isLinkBox() : false)) {
        return LayerBox::prp_getRelFrameShift() +
                getLinkTarget()->prp_getRelFrameShift();
    }
    return LayerBox::prp_getRelFrameShift();
}

bool InternalLinkGroupBox::relPointInsidePath(const QPointF &relPos) const {
    if(!getFinalTarget()) return false;
    return getFinalTarget()->relPointInsidePath(relPos);
}

void InternalLinkGroupBox::setTargetSlot(BoundingBox *target) {
    if(target->SWT_isBoxesGroup())
        setLinkTarget(GetAsPtr(target, LayerBox));
}

void InternalLinkGroupBox::setLinkTarget(LayerBox *linkTarget) {
    disconnect(mBoxTarget.data(), nullptr, this, nullptr);
    if(getLinkTarget()) {
        disconnect(getLinkTarget(), nullptr, this, nullptr);
        getLinkTarget()->removeLinkingBox(this);
        removeAllContainedBoxes();
    }
    if(linkTarget) {
        setName(linkTarget->getName() + " link");
        mBoxTarget->setTarget(linkTarget);
        linkTarget->addLinkingBox(this);
        connect(linkTarget, &BoundingBox::prp_absFrameRangeChanged,
                this, &BoundingBox::prp_afterChangedRelRange);

        const auto &boxesList = linkTarget->getContainedBoxesList();
        for(const auto& child : boxesList) {
            const auto newLink = child->createLinkForLinkGroup();
            addContainedBox(newLink);
        }
    } else {
        setName("empty link");
        mBoxTarget->setTarget(nullptr);
    }
    planScheduleUpdate(Animator::USER_CHANGE);
    connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
            this, &InternalLinkGroupBox::setTargetSlot);
}

QPointF InternalLinkGroupBox::getRelCenterPosition() {
    if(!getLinkTarget()) return QPointF();
    return getLinkTarget()->getRelCenterPosition();
}

LayerBox *InternalLinkGroupBox::getLinkTarget() const {
    return GetAsPtr(mBoxTarget->getTarget(), LayerBox);
}

qsptr<BoundingBox> InternalLinkGroupBox::createLink() {
    if(!getLinkTarget()) return createLink();
    return getLinkTarget()->createLink();
}

qsptr<BoundingBox> InternalLinkGroupBox::createLinkForLinkGroup() {
    if(!getLinkTarget()) return createLink();
    if(mParentGroup->SWT_isLinkBox()) {
        return getLinkTarget()->createLinkForLinkGroup();
    } else {
        return SPtrCreate(InternalLinkGroupBox)(this);
    }
}

bool InternalLinkGroupBox::SWT_isLinkBox() const { return true; }

bool InternalLinkGroupBox::isFrameInDurationRect(const int &relFrame) const {
    if(!getLinkTarget()) return false;
    return LayerBox::isFrameInDurationRect(relFrame) &&
            getLinkTarget()->isFrameInDurationRect(relFrame);
}

stdsptr<BoundingBoxRenderData> InternalLinkGroupBox::createRenderData() {
    if(!getLinkTarget()) return nullptr;
    auto renderData = getLinkTarget()->createRenderData();
    renderData->fParentBox = this;
    return renderData;
}

QRectF InternalLinkGroupBox::getRelBoundingRect(const qreal &relFrame) {
    if(!getLinkTarget()) return QRectF();
    return getLinkTarget()->getRelBoundingRect(relFrame);
}
