#ifndef BOXPATHPOINT_H
#define BOXPATHPOINT_H
#include "animatedpoint.h"
class BoxTransformAnimator;
class BoxPathPoint : public AnimatedPoint {
    friend class StdSelfRef;
protected:
    BoxPathPoint(QPointFAnimator * const associatedAnimator,
                 BoxTransformAnimator * const boxTrans);
public:
    void setRelativePos(const QPointF& relPos);

    void startTransform();
    void finishTransform();

    bool isVisible(const CanvasMode mode) const {
        return mode == MOVE_BOX;
    }

    void drawSk(SkCanvas * const canvas,
                const CanvasMode mode,
                const float invScale,
                const bool keyOnCurrent);
};

#endif // BOXPATHPOINT_H
