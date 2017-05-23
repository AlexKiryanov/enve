#include "boxscrollwidgetvisiblepart.h"
#include "boxsinglewidget.h"
#include <QPainter>
#include "Animators/qrealanimator.h"
#include "boxscrollwidget.h"
#include <QTimer>
#include <QMimeData>
#include "Boxes/boundingbox.h"
#include "Boxes/boxesgroup.h"
#include "mainwindow.h"
#include "global.h"
#include "BoxesList/OptimalScrollArea/singlewidgetabstraction.h"

BoxScrollWidgetVisiblePart::BoxScrollWidgetVisiblePart(
        ScrollWidget *parent) :
    ScrollWidgetVisiblePart(parent) {
    setAcceptDrops(true);
    mScrollTimer = new QTimer(this);
}

SingleWidget *BoxScrollWidgetVisiblePart::createNewSingleWidget() {
    return new BoxSingleWidget(this);
}

void BoxScrollWidgetVisiblePart::paintEvent(QPaintEvent *) {
    QPainter p(this);

//    p.fillRect(rect(), Qt::red);
    int currY = MIN_WIDGET_HEIGHT;
    p.setPen(QPen(QColor(40, 40, 40), 1.));
    while(currY < height()) {
        p.drawLine(0, currY, width(), currY);

        currY += MIN_WIDGET_HEIGHT;
    }

    if(mDragging) {
        p.setPen(QPen(Qt::white, 3.));
        p.drawLine(0, mCurrentDragPosId*MIN_WIDGET_HEIGHT,
                   width(), mCurrentDragPosId*MIN_WIDGET_HEIGHT);
    }

    p.end();
}

void BoxScrollWidgetVisiblePart::drawKeys(QPainter *p,
                                          const qreal &pixelsPerFrame,
                                          const int &minViewedFrame,
                                          const int &maxViewedFrame) {
    //p->setPen(QPen(Qt::black, 1.));
    p->setPen(Qt::NoPen);
    Q_FOREACH(SingleWidget *container, mSingleWidgets) {
        ((BoxSingleWidget*)container)->drawKeys(
                            p, pixelsPerFrame,
                            container->y(),
                            minViewedFrame, maxViewedFrame);
    }
}

Key *BoxScrollWidgetVisiblePart::getKeyAtPos(
        const int &pressX, const int &pressY,
        const qreal &pixelsPerFrame,
        const int &minViewedFrame) {
    int remaining = pressY % MIN_WIDGET_HEIGHT;
    if(remaining < (MIN_WIDGET_HEIGHT - KEY_RECT_SIZE)/2 ||
       remaining > (MIN_WIDGET_HEIGHT + KEY_RECT_SIZE)/2) return NULL;
    Q_FOREACH(SingleWidget *container, mSingleWidgets) {
        int containerTop = container->y();
        int containerBottom = containerTop + container->height();
        if(containerTop > pressY || containerBottom < pressY) continue;
        return ((BoxSingleWidget*)container)->
                getKeyAtPos(pressX, pixelsPerFrame,
                            minViewedFrame);
    }
    return NULL;
}

DurationRectangleMovable *BoxScrollWidgetVisiblePart::getRectangleMovableAtPos(
        const int &pressX,
        const int &pressY,
        const qreal &pixelsPerFrame,
        const int &minViewedFrame) {
    Q_FOREACH(SingleWidget *container, mSingleWidgets) {
        int containerTop = container->y();
        int containerBottom = containerTop + container->height();
        if(containerTop > pressY || containerBottom < pressY) continue;
        return ((BoxSingleWidget*)container)->
                getRectangleMovableAtPos(pressX, pixelsPerFrame,
                                         minViewedFrame);
    }
    return NULL;
}

void BoxScrollWidgetVisiblePart::getKeysInRect(QRectF selectionRect,
        qreal pixelsPerFrame,
        QList<Key *> *listKeys) {
    QList<SingleWidgetAbstraction*> abstractions;
//    selectionRect.adjust(-0.5, -(BOX_HEIGHT/* + KEY_RECT_SIZE*/)*0.5,
//                         0.5, (BOX_HEIGHT/* + KEY_RECT_SIZE*/)*0.5);
    selectionRect.adjust(0.5, 0., 0.5, 0.);
    int currY = 0;
    mMainAbstraction->getAbstractions(selectionRect.top() - MIN_WIDGET_HEIGHT/2,
                                      selectionRect.bottom() - MIN_WIDGET_HEIGHT/2,
                                      &currY, 0,
                                      &abstractions,
                                      mCurrentRulesCollection,
                                      true,
                                      false);

    Q_FOREACH(SingleWidgetAbstraction *abs, abstractions) {
        SingleWidgetTarget *target = abs->getTarget();
        if(target->SWT_isAnimator()) {
            Animator *anim_target = (Animator*)target;
            anim_target->prp_getKeysInRect(selectionRect,
                                     pixelsPerFrame,
                                     listKeys);
        }
    }
//    Q_FOREACH(SingleWidget *container, mSingleWidgets) {
//        int containerTop = container->y();
//        int containerBottom = containerTop + container->height();
//        if(containerTop > selectionRect.bottom() ||
//           containerBottom < selectionRect.top()) continue;
//        ((BoxSingleWidget*)container)->
//                getKeysInRect(selectionRect,
//                              pixelsPerFrame,
//                              listKeys);
//    }
}

BoxSingleWidget *BoxScrollWidgetVisiblePart::
            getClosestsSingleWidgetWithTargetTypeLookBelow(
                const SWT_TargetTypes &type,
                const int &yPos,
                bool *isBelow) {
    int idAtYPos = yPos / MIN_WIDGET_HEIGHT;
    int targetId = (yPos + MIN_WIDGET_HEIGHT/2) /
                    MIN_WIDGET_HEIGHT;
    if(idAtYPos < mSingleWidgets.count() &&
       idAtYPos >= 0) {
        BoxSingleWidget *singleWidgetUnderMouse = (BoxSingleWidget*)
                mSingleWidgets.at(
                    idAtYPos);
        while(singleWidgetUnderMouse->isHidden()) {
            idAtYPos++;
            if(idAtYPos >= mSingleWidgets.count()) return NULL;
            singleWidgetUnderMouse = (BoxSingleWidget*)
                    mSingleWidgets.at(
                        idAtYPos);
        }
        while(!type.isTargeted(singleWidgetUnderMouse->getTargetAbstraction()->
                                getTarget()) ) {
            idAtYPos++;
            if(idAtYPos >= mSingleWidgets.count()) return NULL;
            singleWidgetUnderMouse = (BoxSingleWidget*)
                    mSingleWidgets.at(
                        idAtYPos);
        }
        *isBelow = targetId > idAtYPos;
        return singleWidgetUnderMouse;
    }
    return NULL;
}

BoxSingleWidget *BoxScrollWidgetVisiblePart::
            getClosestsSingleWidgetWithTargetType(
                const SWT_TargetTypes &types,
                const int &yPos,
                bool *isBelow) {
    int idAtYPos = yPos / MIN_WIDGET_HEIGHT;
    int targetId = (yPos + MIN_WIDGET_HEIGHT/2)/
                    MIN_WIDGET_HEIGHT;
    if(idAtYPos < mSingleWidgets.count() &&
       idAtYPos >= 0) {
        BoxSingleWidget *singleWidgetUnderMouse = (BoxSingleWidget*)
                mSingleWidgets.at(
                    idAtYPos);
        while(singleWidgetUnderMouse->isHidden()) {
            idAtYPos--;
            if(idAtYPos < 0) {
                return getClosestsSingleWidgetWithTargetTypeLookBelow(
                            types, yPos, isBelow);
            }
            singleWidgetUnderMouse = (BoxSingleWidget*)
                    mSingleWidgets.at(
                        idAtYPos);
        }
        while(!types.isTargeted(singleWidgetUnderMouse->getTargetAbstraction()->
                                getTarget())) {
            idAtYPos--;
            if(idAtYPos < 0) {
                return getClosestsSingleWidgetWithTargetTypeLookBelow(
                            types, yPos, isBelow);
            }
            singleWidgetUnderMouse = (BoxSingleWidget*)
                    mSingleWidgets.at(
                        idAtYPos);
        }
        *isBelow = targetId > idAtYPos;
        return singleWidgetUnderMouse;
    }
    return NULL;
}

void BoxScrollWidgetVisiblePart::stopScrolling() {
    if(mScrollTimer->isActive()) {
        mScrollTimer->disconnect();
        mScrollTimer->stop();
    }
}

void BoxScrollWidgetVisiblePart::dropEvent(
        QDropEvent *event) {
    stopScrolling();
    mDragging = false;
    if(event->mimeData()->hasFormat("boundingbox")) {
        int yPos = event->pos().y();
        bool below;

        SWT_TargetTypes type;
        type.targetsFunctionList =
                QList<SWT_Checker>({&SingleWidgetTarget::SWT_isBoundingBox});
        BoxSingleWidget *singleWidgetUnderMouse =
                getClosestsSingleWidgetWithTargetType(
                    type,
                    yPos,
                    &below);
        if(singleWidgetUnderMouse == NULL) return;

        BoundingBox *box = ((BoundingBoxMimeData*)event->mimeData())->
                getBoundingBox();
        BoundingBox *boxUnderMouse =
                ((BoundingBox*)singleWidgetUnderMouse->
                 getTargetAbstraction()->getTarget());

        BoxesGroup *parentGroup = boxUnderMouse->getParent();
        if(parentGroup == NULL ||
           boxUnderMouse->isAncestor(box)) return;
        if(parentGroup != box->getParent()) {
            box->getParent()->removeChild(box);
            parentGroup->addChild(box);
        }
        if(below) { // add box below
            parentGroup->moveChildAbove( // boxesgroup list is reversed
                        box,
                       boxUnderMouse);
        } else { // add box above
            parentGroup->moveChildBelow(
                        box,
                        boxUnderMouse);
        }
    } else if(event->mimeData()->hasFormat("pixmapeffect")) {
        int yPos = event->pos().y();
        bool below;

        SWT_TargetTypes type;
        type.targetsFunctionList =
                QList<SWT_Checker>({&SingleWidgetTarget::SWT_isPixmapEffect});
        BoxSingleWidget *singleWidgetUnderMouse =
                getClosestsSingleWidgetWithTargetType(
                    type,
                    yPos,
                    &below);
        if(singleWidgetUnderMouse == NULL) return;

        PixmapEffect *effect = ((PixmapEffectMimeData*)event->mimeData())->
                getPixmapEffect();
        PixmapEffect *effectUnderMouse =
                ((PixmapEffect*)singleWidgetUnderMouse->
                 getTargetAbstraction()->getTarget());

        if(effect != effectUnderMouse) {
            EffectAnimators *underMouseAnimator =
                                     effectUnderMouse->getParentEffectAnimators();
            EffectAnimators *draggedAnimator =
                                     effect->getParentEffectAnimators();
            if(draggedAnimator != underMouseAnimator) {
                draggedAnimator->getParentBox()->removeEffect(effect);
                underMouseAnimator->getParentBox()->addEffect(effect);
            }
            if(below) { // add box below
                underMouseAnimator->ca_moveChildAbove( // boxesgroup list is reversed
                            effect,
                            effectUnderMouse);
            } else { // add box above
                underMouseAnimator->ca_moveChildBelow(
                            effect,
                            effectUnderMouse);
            }
            underMouseAnimator->getParentBox()->clearAllCache();
        }
    }
    scheduledUpdateVisibleWidgetsContent();
    MainWindow::getInstance()->callUpdateSchedulers();
}

void BoxScrollWidgetVisiblePart::dragEnterEvent(
        QDragEnterEvent *event)
{
    //mDragging = true;
    if(event->mimeData()->hasFormat("boundingbox") ||
       event->mimeData()->hasFormat("pixmapeffect")) {
        event->acceptProposedAction();
    }
}

void BoxScrollWidgetVisiblePart::dragLeaveEvent(
        QDragLeaveEvent *) {
    mDragging = false;
    stopScrolling();
//    if(mScrollTimer->isActive()) {
//        QPoint mousePos = mapFromGlobal(QCursor::pos());
//        if(mousePos.x() < 20 || mousePos.x() > width() - 20) {
//            mScrollTimer->disconnect();
//            mScrollTimer->stop();
//        } else {
//            mScrollTimer->setInterval(100);
//        }
//    }
    update();
}

#include <QDebug>
void BoxScrollWidgetVisiblePart::dragMoveEvent(
        QDragMoveEvent *event) {    
    int yPos = event->pos().y();

    if(yPos < 30) {
        if(!mScrollTimer->isActive()) {
            connect(mScrollTimer, SIGNAL(timeout()),
                    this, SLOT(scrollUp()));
            mScrollTimer->start(300);
        }
    } else if(yPos > height() - 30) {
        if(!mScrollTimer->isActive()) {
            connect(mScrollTimer, SIGNAL(timeout()),
                    this, SLOT(scrollDown()));
            mScrollTimer->start(300);
        }
    } else {
        mScrollTimer->disconnect();
        mScrollTimer->stop();
    }
    mLastDragMoveY = yPos;
    if(event->mimeData()->hasFormat("boundingbox")) {
        mLastDragMoveTargetTypes.targetsFunctionList =
                QList<SWT_Checker>(
                        {&SingleWidgetTarget::SWT_isBoundingBox,
                         &SingleWidgetTarget::SWT_isBoxesGroup});
    } else if(event->mimeData()->hasFormat("pixmapeffect")) {
        mLastDragMoveTargetTypes.targetsFunctionList =
                QList<SWT_Checker>(
                        {&SingleWidgetTarget::SWT_isPixmapEffect});
    }

    updateDraggingHighlight();
}

void BoxScrollWidgetVisiblePart::updateDraggingHighlight() {
    mDragging = false;
    bool below;
    BoxSingleWidget *singleWidgetUnderMouse =
            getClosestsSingleWidgetWithTargetType(mLastDragMoveTargetTypes,
                                                  mLastDragMoveY,
                                                  &below);
    if(singleWidgetUnderMouse != NULL) {
        int currentDragPosId = singleWidgetUnderMouse->y()/MIN_WIDGET_HEIGHT;
        if(below) {
            currentDragPosId++;
        }
        mDragging = true;
        mCurrentDragPosId = currentDragPosId;
    }
    update();
}

void BoxScrollWidgetVisiblePart::scrollUp() {
    mParentWidget->scrollParentAreaBy(-MIN_WIDGET_HEIGHT);
    updateDraggingHighlight();
}

void BoxScrollWidgetVisiblePart::scrollDown() {
    mParentWidget->scrollParentAreaBy(MIN_WIDGET_HEIGHT);
    updateDraggingHighlight();
}
