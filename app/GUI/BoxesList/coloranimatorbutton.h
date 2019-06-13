#ifndef COLORANIMATORBUTTON_H
#define COLORANIMATORBUTTON_H
#include "boxeslistactionbutton.h"
#include "smartPointers/selfref.h"
class ColorAnimator;

class ColorAnimatorButton : public BoxesListActionButton {
public:
    ColorAnimatorButton(ColorAnimator * const colorTarget,
                        QWidget * const parent = nullptr);

    void setColorTarget(ColorAnimator * const target);
    void openColorSettingsDialog();

    QColor color() const;
protected:
    void paintEvent(QPaintEvent *);
private:
    QColor mColor;
    qptr<ColorAnimator> mColorTarget;
};

#endif // COLORANIMATORBUTTON_H
