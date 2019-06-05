#ifndef RECTANGLE_H
#define RECTANGLE_H
#include "Boxes/pathbox.h"
class AnimatedPoint;
class QPointFAnimator;
class Rectangle : public PathBox {
    friend class SelfRef;
protected:
    Rectangle();
public:
    void moveSizePointByAbs(const QPointF &absTrans);
    void startAllPointsTransform();

    MovablePoint *getBottomRightPoint();
    void finishAllPointsTransform();

    bool SWT_isRectangle() const { return true; }
    SkPath getPathAtRelFrameF(const qreal relFrame);

    void setTopLeftPos(const QPointF &pos);
    void setBottomRightPos(const QPointF &pos);
    void setYRadius(const qreal radiusY);
    void setXRadius(const qreal radiusX);
    void writeBoundingBox(QIODevice * const target);
    void readBoundingBox(QIODevice * const target);
    bool differenceInEditPathBetweenFrames(
                const int frame1, const int frame2) const;
private:
    qsptr<QPointFAnimator> mTopLeftAnimator;
    qsptr<QPointFAnimator> mBottomRightAnimator;
    qsptr<QPointFAnimator> mRadiusAnimator;

    stdsptr<AnimatedPoint> mTopLeftPoint;
    stdsptr<AnimatedPoint> mBottomRightPoint;
    stdsptr<AnimatedPoint> mRadiusPoint;

    void getMotionBlurProperties(QList<Property*> &list) const;
};

#endif // RECTANGLE_H
