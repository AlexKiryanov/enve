#ifndef BRUSHWIDGET_H
#define BRUSHWIDGET_H

#include <QWidget>
#include "brushcontexedwrapper.h"

class BrushWidget : public QWidget {
    Q_OBJECT
public:
    explicit BrushWidget(BrushContexedWrapper * const brushCWrapper,
                         QWidget * const parent = nullptr);
protected:
    void paintEvent(QPaintEvent*);
    void enterEvent(QEvent*) {
        mHovered = true;
        update();
    }
    void leaveEvent(QEvent*) {
        mHovered = false;
        update();
    }

    void mousePressEvent(QMouseEvent* e);
signals:
    void selected(BrushContexedWrapper*);
public slots:
    void selectionChanged(bool sel) {
        if(sel) emit selected(mBrushCWrapper);
        update();
    }
private:
    bool isSelected() const {
        return mBrushCWrapper->selected();
    }

    void deselect() {
        mBrushCWrapper->setSelected(false);
    }

    const qptr<BrushContexedWrapper> mBrushCWrapper;
    bool mHovered = false;
};

#endif // BRUSHWIDGET_H
