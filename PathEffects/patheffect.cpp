#include "patheffect.h"
#include "pointhelpers.h"
#include "pathoperations.h"
#include "skqtconversions.h"
#include "patheffectanimators.h"
#include <QDrag>

PathEffect::PathEffect(const QString &name,
                       const PathEffectType &type,
                       const bool &outlinePathEffect) :
    ComplexAnimator(name) {
    mPathEffectType = type;
    setIsOutlineEffect(outlinePathEffect);
}

void PathEffect::prp_startDragging() {
    QMimeData *mimeData = new PathEffectMimeData(this);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec();
}

const PathEffectType &PathEffect::getEffectType() {
    return mPathEffectType;
}

bool PathEffect::applyBeforeThickness() {
    if(mApplyBeforeThickness == nullptr) return false;
    return mApplyBeforeThickness->getValue();
}

void PathEffect::setParentEffectAnimators(PathEffectAnimators *parent) {
    mParentEffectAnimators = parent;
}

PathEffectAnimators *PathEffect::getParentEffectAnimators() {
    return mParentEffectAnimators.data();
}

QMimeData *PathEffect::SWT_createMimeData() {
    return new PathEffectMimeData(this);
}

bool PathEffect::SWT_isPathEffect() { return true; }

void PathEffect::setIsOutlineEffect(const bool &bT) {
    if(bT == mOutlineEffect) return;
    mOutlineEffect = bT;
    if(mOutlineEffect) {
        mApplyBeforeThickness = SPtrCreate(BoolProperty)("pre-thickness");
        ca_addChildAnimator(mApplyBeforeThickness);
    } else if(mApplyBeforeThickness != nullptr) {
        ca_removeChildAnimator(mApplyBeforeThickness);
        mApplyBeforeThickness.reset();
    }
}

void PathEffect::switchVisible() {
    setVisible(!mVisible);
}

void PathEffect::setVisible(const bool &bT) {
    if(bT == mVisible) return;
    mVisible = bT;
    prp_updateInfluenceRangeAfterChanged();
}

const bool &PathEffect::isVisible() {
    return mVisible;
}

bool PathEffect::hasReasonsNotToApplyUglyTransform() { return false; }
