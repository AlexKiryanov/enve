﻿#ifndef UPDATABLE_H
#define UPDATABLE_H
#include <QList>
#include <QEventLoop>
#include "smartPointers/sharedpointerdefs.h"
class TaskExecutor;

class Que;

class Task : public StdSelfRef {
    friend class TaskScheduler;
    friend class Que;
protected:
    Task() {}

    virtual void scheduleTaskNow() = 0;
    virtual void beforeProcessing() {}
    //! @brief For intermediate tasks called after
    //! all other tasks in the parent ContainerTask finished,
    //! for all others called just after finishing
    virtual void afterProcessing() {}
    virtual void afterCanceled() {}
public:
    enum State : char {
        CREATED,
        SCHEDULED,
        QUED,
        PROCESSING,
        FINISHED,
        CANCELED,
        FAILED
    };

    virtual void processTask() = 0;
    virtual bool needsGpuProcessing() const { return false; }
    virtual void taskQued() { mState = QUED; }
    //! @brief  For children tasks of a ContainerTask.
    //! Can pass results only to a single task.
    //! Do NOT use for passing results to main thread cache/handlers/boxes etc.
    virtual void afterProcessingAsContainerStep() {}

    bool scheduleTask();
    bool isQued() { return mState == QUED; }
    bool isScheduled() { return mState == SCHEDULED; }

    ~Task() {
        cancelDependent();
    }

    bool isActive() { return mState != CREATED && mState != FINISHED; }

    void aboutToProcess();
    void finishedProcessing();
    bool readyToBeProcessed();

    void addDependent(Task * const updatable);

    bool finished();

    void decDependencies();
    void incDependencies();

    void cancel() {
        mState = CANCELED;
        cancelDependent();
        afterCanceled();
    }

    State getState() const {
        return mState;
    }

    void setException(const std::exception_ptr& exception) {
        mUpdateException = exception;
    }

    bool unhandledException() const {
        return static_cast<bool>(mUpdateException);
    }

    std::exception_ptr handleException() {
        std::exception_ptr exc;
        mUpdateException.swap(exc);
        return exc;
    }
protected:
    State mState = CREATED;
private:
    void tellDependentThatFinished();
    void cancelDependent();

    int mNDependancies = 0;
    QList<stdptr<Task>> mDependent;
    std::exception_ptr mUpdateException;
};

class CPUTask : public Task {
    friend class StdSelfRef;
protected:
    void scheduleTaskNow() final;
};

class HDDTask : public Task {
    friend class StdSelfRef;
protected:
    void scheduleTaskNow() final;
};

class ContainerTask : public Task {
    friend class StdSelfRef;
protected:
    ContainerTask() {}
public:
    void scheduleTaskNow() final;

    void addCPUTask(const stdsptr<Task>& task) {
        mCPUTasks << task;
        incDependencies();
    }

    void addHDDTask(const stdsptr<Task>& task) {
        mHDDTasks << task;
        incDependencies();
    }
protected:
    void afterProcessing() {
        for(const auto& task : mProcessingTasks)
            task->finishedProcessing();
    }
private:
    void afterSubTaskFinished() {
        decDependencies();
        scheduleReadyChildren();
    }
    void scheduleReadyChildren();

    QList<stdsptr<Task>> mProcessingTasks;

    QList<stdsptr<Task>> mCPUTasks;
    QList<stdsptr<Task>> mHDDTasks;
};

class CustomCPUTask : public CPUTask {
    friend class StdSelfRef;
public:
    void beforeProcessing() final {
        if(mBefore) mBefore();
    }

    void processTask() final {
        if(mRun) mRun();
    }

protected:
    void afterProcessing() final {
        if(mAfter) mAfter();
    }

    CustomCPUTask(const std::function<void(void)>& before,
                  const std::function<void(void)>& run,
                  const std::function<void(void)>& after) :
        mBefore(before), mRun(run), mAfter(after) {}
private:
    const std::function<void(void)> mBefore;
    const std::function<void(void)> mRun;
    const std::function<void(void)> mAfter;
};

class CustomHDDTask : public HDDTask {
    friend class StdSelfRef;
public:
    void beforeProcessing() final {
        if(mBefore) mBefore();
    }

    void processTask() final {
        if(mRun) mRun();
    }

protected:
    void afterProcessing() final {
        if(mAfter) mAfter();
    }

    CustomHDDTask(const std::function<void(void)>& before,
                  const std::function<void(void)>& run,
                  const std::function<void(void)>& after) :
        mBefore(before), mRun(run), mAfter(after) {}
private:
    const std::function<void(void)> mBefore;
    const std::function<void(void)> mRun;
    const std::function<void(void)> mAfter;
};

#endif // UPDATABLE_H
