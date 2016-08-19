#include "connectedtomainwindow.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include <QApplication>

ConnectedToMainWindow::ConnectedToMainWindow(ConnectedToMainWindow *parent)
{
    mMainWindow = parent->getMainWindow();
}

ConnectedToMainWindow::ConnectedToMainWindow(MainWindow *parent)
{
    mMainWindow = parent;
}

void ConnectedToMainWindow::addUndoRedo(UndoRedo *undoRedo)
{
    mMainWindow->getUndoRedoStack()->addUndoRedo(undoRedo);
}

void ConnectedToMainWindow::addUpdateScheduler(UpdateScheduler *scheduler)
{
    mMainWindow->addUpdateScheduler(scheduler);
}

void ConnectedToMainWindow::callUpdateSchedulers()
{
    mMainWindow->callUpdateSchedulers();
}

MainWindow *ConnectedToMainWindow::getMainWindow()
{
    return mMainWindow;
}

bool ConnectedToMainWindow::isShiftPressed() {
    return QApplication::keyboardModifiers() & Qt::ShiftModifier;
}

bool ConnectedToMainWindow::isCtrlPressed() {
    return (QApplication::keyboardModifiers() & Qt::ControlModifier);
}

void ConnectedToMainWindow::startNewUndoRedoSet()
{
     mMainWindow->getUndoRedoStack()->startNewSet();
}

void ConnectedToMainWindow::finishUndoRedoSet()
{
     mMainWindow->getUndoRedoStack()->finishSet();
}

void ConnectedToMainWindow::scheduleRepaint() {
    mMainWindow->scheduleRepaint();
}

void ConnectedToMainWindow::scheduleBoxesListRepaint() {
    mMainWindow->scheduleBoxesListRepaint();
}

void ConnectedToMainWindow::schedulePivotUpdate() {
    mMainWindow->schedulePivotUpdate();
}
