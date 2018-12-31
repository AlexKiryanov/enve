#ifndef CANVASWINDOW_H
#define CANVASWINDOW_H

#include <QWidget>
#include "glwindow.h"
#include "singlewidgettarget.h"
#include "keyfocustarget.h"
#include "smartPointers/sharedpointerdefs.h"
#include "GPUEffects/gpupostprocessor.h"
class Brush;
class WindowSingleWidgetTarget;
enum ColorMode : short;
enum CanvasMode : short;
class Gradient;
class BoundingBox;
class BoxesGroup;
class TaskExecutor;
class SoundComposition;
class PaintSetting;
class CanvasWidget;
class RenderInstanceSettings;
class _ScheduledTask;
class _Task;
class ImageBox;
class SingleSound;
class VideoBox;
class Canvas;
class PaintSettings;
class StrokeSettings;
#include <QAudioOutput>

class CanvasWindow : public GLWindow,
        public KeyFocusTarget {
    Q_OBJECT
public:
    explicit CanvasWindow(QWidget *parent);
    ~CanvasWindow();

    Canvas *getCurrentCanvas();
    const QList<qsptr<Canvas>> &getCanvasList() {
        return mCanvasList;
    }

    void setCurrentCanvas(Canvas * const canvas);
    void addCanvasToList(const qsptr<Canvas> &canvas);
    void removeCanvas(const int &id);
    void addCanvasToListAndSetAsCurrent(const qsptr<Canvas> &canvas);
    void renameCanvas(Canvas *canvas, const QString &newName);
    void renameCanvas(const int &id, const QString &newName);
    bool hasNoCanvas();
    void setCanvasMode(const CanvasMode &mode);

    void queScheduledTasksAndUpdate();
    bool KFT_handleKeyEventForTarget(QKeyEvent *event);
    void KFT_setFocusToWidget() {
        setFocus();
        requestUpdate();
    }

    void KFT_clearFocus() {
        clearFocus();
    }

    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();

    void strokeCapStyleChanged(const Qt::PenCapStyle &capStyle);
    void strokeJoinStyleChanged(const Qt::PenJoinStyle &joinStyle);
    void strokeWidthChanged(const qreal &strokeWidth);

    void setResolutionFraction(const qreal &percent);
    void updatePivotIfNeeded();
    void schedulePivotUpdate();

    BoxesGroup *getCurrentGroup();
    void applyPaintSettingToSelected(PaintSetting* setting);
    void setSelectedFillColorMode(const ColorMode &mode);
    void setSelectedStrokeColorMode(const ColorMode &mode);

    void updateAfterFrameChanged(const int &currentFrame);
    void clearAll();
    void createAnimationBoxForPaths(const QStringList &importPaths);
    void createLinkToFileWithPath(const QString &path);
    VideoBox *createVideoForPath(const QString &path);
    int getCurrentFrame();
    int getMaxFrame();
    void SWT_addChildrenAbstractions(
            SingleWidgetAbstraction *abstraction,
            const UpdateFuncs &updateFuncs,
            const int& visiblePartWidgetId);
    ImageBox *createImageForPath(const QString &path);
    SingleSound *createSoundForPath(const QString &path);
    void updateHoveredElements();

    void setLocalPivot(const bool &bT);
    void setBonesSelectionEnabled(const bool &bT);

    void importFile(const QString &path,
                    const QPointF &relDropPos = QPointF(0., 0.));

    QWidget *getCanvasWidget();

    void grabMouse();

    bool hasFocus() {
        return mHasFocus;
    }

    void setFocus() {
        mHasFocus = true;
    }

    void clearFocus() {
        mHasFocus = false;
    }

    QRect rect();

    void releaseMouse();

    bool isMouseGrabber();

    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void openSettingsWindowForCurrentCanvas();

    void rotate90CCW();
    void rotate90CW();

    void writeCanvases(QIODevice *target);
    void readCanvases(QIODevice *target);

    WindowSingleWidgetTarget *getWindowSWT() {
        return mWindowSWTTarget.get();
    }

    void startDurationRectPosTransformForAllSelected();
    void finishDurationRectPosTransformForAllSelected();
    void moveDurationRectForAllSelected(const int &dFrame);
    void startMinFramePosTransformForAllSelected();
    void finishMinFramePosTransformForAllSelected();
    void moveMinFrameForAllSelected(const int &dFrame);
    void startMaxFramePosTransformForAllSelected();
    void finishMaxFramePosTransformForAllSelected();
    void moveMaxFrameForAllSelected(const int &dFrame);
    void getDisplayedFillStrokeSettingsFromLastSelected(
            PaintSettings *&fillSetings, StrokeSettings *&strokeSettings);
private:
    void changeCurrentFrameAction(const int &frame);

    //! @brief true if preview is currently playing
    bool mPreviewing = false;
    //! @brief true if currently preview is being rendered
    bool mRenderingPreview = false;
    bool mMouseGrabber = false;
    bool mHasFocus = false;

    int mCurrentRenderFrame;
    int mMaxRenderFrame = 0;
    int mSavedCurrentFrame = 0;

    qreal mSavedResolutionFraction = 100.;

    qsptr<WindowSingleWidgetTarget> mWindowSWTTarget;
    RenderInstanceSettings *mCurrentRenderSettings = nullptr;

    QWidget *mCanvasWidget;
    void setRendering(const bool &bT);
    void setPreviewing(const bool &bT);

    QTimer *mPreviewFPSTimer = nullptr;

    qptr<Canvas> mCurrentCanvas;
    QList<qsptr<Canvas>> mCanvasList;

    //void paintEvent(QPaintEvent *);

    void nextCurrentRenderFrame();


    // AUDIO
    void initializeAudio();
    void startAudio();
    void stopAudio();
    void volumeChanged(int value);

    QAudioDeviceInfo mAudioDevice;
    qptr<SoundComposition> mCurrentSoundComposition;
    QAudioOutput *mAudioOutput;
    QIODevice *mAudioIOOutput; // not owned
    QAudioFormat mAudioFormat;

    QByteArray mAudioBuffer;
    // AUDIO

    void qRender(QPainter *p);
    void renderSk(SkCanvas * const canvas,
                  GrContext * const grContext);
    void tabletEvent(QTabletEvent *e);

    bool handleCanvasModeChangeKeyPress(QKeyEvent *event);
    bool handleCutCopyPasteKeyPress(QKeyEvent *event);
    bool handleTransformationKeyPress(QKeyEvent *event);
    bool handleZValueKeyPress(QKeyEvent *event);
    bool handleParentChangeKeyPress(QKeyEvent *event);
    bool handleGroupChangeKeyPress(QKeyEvent *event);
    bool handleResetTransformKeyPress(QKeyEvent *event);
    bool handleRevertPathKeyPress(QKeyEvent *event);
    bool handleStartTransformKeyPress(QKeyEvent *event);
    bool handleSelectAllKeyPress(QKeyEvent *event);
    bool handleShiftKeysKeyPress(QKeyEvent *event);
signals:
    void updateUpdatable(_ScheduledTask*, int);
    void updateFileUpdatable(_ScheduledTask*, int);

    void changeCurrentFrame(int);
    void changeCanvasFrameRange(int, int);
public slots:
    void setCurrentBrush(const Brush *brush);
    void replaceBrush(const Brush *oldBrush,
                      const Brush *newBrush);

    void setMovePathMode();
    void setMovePointMode();
    void setAddPointMode();
    void setPickPaintSettingsMode();
    void setRectangleMode();
    void setCircleMode();
    void setTextMode();
    void setParticleBoxMode();
    void setParticleEmitterMode();
    void setPaintBoxMode();
    void setPaintMode();

    void raiseAction();
    void lowerAction();
    void raiseToTopAction();
    void lowerToBottomAction();

    void objectsToPathAction();
    void strokeToPathAction();

    void setFontFamilyAndStyle(const QString& family,
                               const QString& style);
    void setFontSize(const qreal &size);

    void connectPointsSlot();
    void disconnectPointsSlot();
    void mergePointsSlot();

    void makePointCtrlsSymmetric();
    void makePointCtrlsSmooth();
    void makePointCtrlsCorner();

    void makeSegmentLine();
    void makeSegmentCurve();

    void pathsUnionAction();
    void pathsDifferenceAction();
    void pathsIntersectionAction();
    void pathsDivisionAction();
    void pathsExclusionAction();
    void pathsCombineAction();
    void pathsBreakApartAction();

    void renameCurrentCanvas(const QString &newName);
    void setCurrentCanvas(const int &id);

    void setClipToCanvas(const bool &bT);
    void setRasterEffectsVisible(const bool &bT);
    void setPathEffectsVisible(const bool &bT);

    void interruptPreview();
    void outOfMemory();
    void interruptPreviewRendering();
    void interruptOutputRendering();

    void playPreview();
    void stopPreview();
    void pausePreview();
    void resumePreview();
    void renderPreview();
    void renderFromSettings(RenderInstanceSettings *settings);

    void importFile();

    void startSelectedStrokeWidthTransform();

    void deleteAction();
    void copyAction();
    void pasteAction();
    void cutAction();
    void duplicateAction();
    void selectAllAction();
    void invertSelectionAction();
    void clearSelectionAction();

    void groupSelectedBoxes();
    void ungroupSelectedBoxes();
    void rotate90CWAction();
    void rotate90CCWAction();

    void flipHorizontalAction();
    void flipVerticalAction();
private slots:
    void nextSaveOutputFrame();
    void nextPreviewRenderFrame();

    void pushTimerExpired();
};

#endif // CANVASWINDOW_H
