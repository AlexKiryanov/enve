#include "taskscheduler.h"
#include "Boxes/boundingboxrenderdata.h"
#include "GPUEffects/gpupostprocessor.h"
#include "canvas.h"
#include "taskexecutor.h"
#include <QThread>

TaskScheduler *TaskScheduler::sInstance;

TaskScheduler::TaskScheduler() {
    sInstance = this;
    int numberThreads = qMax(1, QThread::idealThreadCount());
    for(int i = 0; i < numberThreads; i++) {
        QThread *taskExecutorThread = new QThread(this);
        TaskExecutor *taskExecutor = new TaskExecutor(i);
        taskExecutor->moveToThread(taskExecutorThread);
        connect(taskExecutor, &TaskExecutor::finishedUpdating,
                this, &TaskScheduler::processNextQuedCPUTask);
        connect(this, &TaskScheduler::processCPUTask,
                taskExecutor, &TaskExecutor::updateUpdatable);

        taskExecutorThread->start();

        mCPUTaskExecutors << taskExecutor;
        mExecutorThreads << taskExecutorThread;

        mFreeCPUThreads << i;
    }

    mHDDExecutorThread = new QThread(this);
    mHDDExecutor = new TaskExecutor(numberThreads);
    mHDDExecutor->moveToThread(mHDDExecutorThread);
    connect(mHDDExecutor, &TaskExecutor::finishedUpdating,
            this, &TaskScheduler::processNextQuedHDDTask);
    connect(this, &TaskScheduler::processHDDTask,
            mHDDExecutor, &TaskExecutor::updateUpdatable);

    mHDDExecutorThread->start();
    mExecutorThreads << mHDDExecutorThread;
}

TaskScheduler::~TaskScheduler() {
    for(const auto& thread : mExecutorThreads) {
        thread->quit();
        thread->wait();
        delete thread;
    }
    for(const auto& taskExecutor : mCPUTaskExecutors) {
        delete taskExecutor;
    }
//    mFileControlerThread->quit();
//    mFileControlerThread->wait();
    delete mHDDExecutor;
}

void TaskScheduler::initializeGPU() {
    try {
        mGpuPostProcessor.initialize();
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e, "Failed to initialize gpu for post-processing.");
    }

    connect(&mGpuPostProcessor, &GpuPostProcessor::finished,
            this, &TaskScheduler::tryProcessingNextQuedCPUTask);
    connect(&mGpuPostProcessor, &GpuPostProcessor::processedAll,
            this, &TaskScheduler::finishedAllQuedTasks);
}

void TaskScheduler::scheduleCPUTask(const stdsptr<_ScheduledTask>& task) {
    mScheduledCPUTasks << task;
}

void TaskScheduler::scheduleHDDTask(const stdsptr<_ScheduledTask>& task) {
    mScheduledHDDTasks << task;
}

void TaskScheduler::queCPUTask(const stdsptr<_ScheduledTask>& task) {
    if(!task->isQued()) task->taskQued();

    mQuedCPUTasks << task;
    tryProcessingNextQuedCPUTask();
}

void TaskScheduler::queScheduledCPUTasks() {
    // if(mBusyCPUThreads.isEmpty() && mQuedCPUTasks.isEmpty()) { !!! failes when creating BoxesGroup
    if(mBusyCPUThreads.isEmpty()) {
        if(mCurrentCanvas) {
            mCurrentCanvas->scheduleWaitingTasks();
            mCurrentCanvas->queScheduledTasks();
        }
        for(const auto &task : mScheduledCPUTasks) {
            queCPUTask(task);
        }
        mScheduledCPUTasks.clear();
    }
}

void TaskScheduler::queScheduledHDDTasks() {
    if(!mHDDThreadBusy) {
        for(const auto &task : mScheduledHDDTasks) {
            if(!task->isQued()) task->taskQued();

            mQuedHDDTasks << task;
            tryProcessingNextQuedHDDTask();
        }
        mScheduledHDDTasks.clear();
    }
}

void TaskScheduler::tryProcessingNextQuedHDDTask() {
    if(!mHDDThreadBusy) {
        processNextQuedHDDTask(mCPUTaskExecutors.count(), nullptr);
    }
}

void TaskScheduler::tryProcessingNextQuedCPUTask() {
    if(!mFreeCPUThreads.isEmpty()) {
        processNextQuedCPUTask(mFreeCPUThreads.takeFirst(), nullptr);
    }
}

void TaskScheduler::processNextQuedHDDTask(
        const int &finishedThreadId,
        _ScheduledTask * const finishedTask) {
    Q_UNUSED(finishedThreadId);
    if(mHDDThreadBusy && !finishedTask) return;
    mHDDThreadBusy = false;
    if(finishedTask) finishedTask->finishedProcessing();
    if(!mFreeCPUThreads.isEmpty() && !mQuedCPUTasks.isEmpty()) {
        processNextQuedCPUTask(mFreeCPUThreads.takeFirst(), nullptr);
    }
    if(mQuedHDDTasks.isEmpty()) {
        callAllQuedHDDTasksFinishedFunc();
//        if(!mRenderingPreview) {
//            callUpdateSchedulers();
//        }
//        if(!mFreeCPUThreads.isEmpty() && !mQuedCPUTasks.isEmpty()) {
//            processNextQuedCPUTask(mFreeCPUThreads.takeFirst(), nullptr);
//        }
    } else {
        for(int i = 0; i < mQuedHDDTasks.count(); i++) {
            auto task = mQuedHDDTasks.at(i).get();
            if(task->readyToBeProcessed()) {
                task->setCurrentTaskExecutor(mHDDExecutor);
                task->beforeProcessingStarted();
                emit processHDDTask(task, mCPUTaskExecutors.count());
                mQuedHDDTasks.takeAt(i);
                i--;
                mHDDThreadBusy = true;
                return;
            }
        }
    }
}

void TaskScheduler::processNextQuedCPUTask(
        const int &finishedThreadId,
        _ScheduledTask *const finishedTask) {
    if(finishedTask) {
        mBusyCPUThreads.removeOne(finishedThreadId);
        if(finishedTask->needsGpuProcessing()) {
            auto gpuProcess =
                    SPtrCreate(BoxRenderDataScheduledPostProcess)(
                        GetAsSPtr(finishedTask, BoundingBoxRenderData));
            mGpuPostProcessor.addToProcess(gpuProcess);
        } else {
            finishedTask->finishedProcessing();
        }
    }
    if(mQuedCPUTasks.isEmpty()) {
        mFreeCPUThreads << finishedThreadId;
        callAllQuedCPUTasksFinishedFunc();
        if(mGpuPostProcessor.hasFinished()) {
            emit finishedAllQuedTasks();
        }
    } else {
        int threadId = finishedThreadId;
        for(int i = 0; i < mQuedCPUTasks.count(); i++) {
            auto updatablaT = mQuedCPUTasks.at(i).get();
            if(updatablaT->readyToBeProcessed()) {
                updatablaT->setCurrentTaskExecutor(
                            mCPUTaskExecutors.at(threadId));
                updatablaT->beforeProcessingStarted();
                mQuedCPUTasks.removeAt(i);
                emit processCPUTask(updatablaT, threadId);
                mBusyCPUThreads << threadId;
                i--;
                //return;
                if(mFreeCPUThreads.isEmpty() || mQuedCPUTasks.isEmpty()) {
//                    UsageWidget* usageWidget = MainWindow::getInstance()->getUsageWidget();
//                    if(usageWidget)
//                        usageWidget->setThreadsUsage(mThreadsUsed);
                    return;
                }
                threadId = mFreeCPUThreads.takeFirst();
            }
        }

        mFreeCPUThreads << threadId;
        //callAllQuedCPUTasksFinishedFunc();
    }
//    UsageWidget* usageWidget = MainWindow::getInstance()->getUsageWidget();
//    if(!usageWidget) return;
//    usageWidget->setThreadsUsage(mThreadsUsed);
}
