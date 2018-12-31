#ifndef EFFECTANIMATORS_H
#define EFFECTANIMATORS_H
#include "Animators/complexanimator.h"
#include "smartPointers/sharedpointerdefs.h"

class PixmapEffect;
class BoundingBox;
class SkImage;
class SkCanvas;
class SkBitmap;
struct BoundingBoxRenderData;
struct PixmapEffectRenderData;


class EffectAnimators : public ComplexAnimator {
    friend class SelfRef;
public:
    void addEffect(const qsptr<PixmapEffect> &effect);

    qreal getEffectsMargin() const;

    void setParentBox(BoundingBox *box);
    BoundingBox *getParentBox() {
        return mParentBox_k;
    }

    bool hasEffects();

    bool SWT_isPixmapEffectAnimators() const { return true; }
    qreal getEffectsMarginAtRelFrame(const int &relFrame) const;
    qreal getEffectsMarginAtRelFrameF(const qreal &relFrame) const;

    void addEffectRenderDataToListF(const qreal &relFrame,
                                    BoundingBoxRenderData *data);

    void ca_removeAllChildAnimators();

    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
    void readPixmapEffect(QIODevice *target);
protected:
    EffectAnimators(BoundingBox *parentBox);
private:
    BoundingBox * const mParentBox_k;
};

#endif // EFFECTANIMATORS_H
