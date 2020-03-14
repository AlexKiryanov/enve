#ifndef QATOMICLIST_H
#define QATOMICLIST_H

#include <QList>
#include <mutex>
#include <condition_variable>

template <typename T>
class QAtomicList : private QList<T> {
public:
    int count() {
        std::lock_guard<std::mutex> lk(mMutex);
        return QList<T>::count();
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> lk(mMutex);
        return QList<T>::isEmpty();
    }

    void copy(QList<T>& list) {
        std::lock_guard<std::mutex> lk(mMutex);
        list = *this;
    }

    void append(const T& t) {
        std::lock_guard<std::mutex> lk(mMutex);
        QList<T>::append(t);
    }

    QList<T> get() {
        std::lock_guard<std::mutex> lk(mMutex);
        return *this;
    }

    QList<T> takeAll() {
        std::lock_guard<std::mutex> lk(mMutex);
        QList<T> result;
        QList<T>::swap(result);
        return result;
    }

    bool takeFirst(T& t) {
        std::lock_guard<std::mutex> lk(mMutex);
        if(QList<T>::isEmpty()) return false;
        t = QList<T>::takeFirst();
        return true;
    }

    void appendAndNotifyAll(const QList<T>& list) {
        std::lock_guard<std::mutex> lk(mMutex);
        QList<T>::append(list);
        mCv.notify_all();
    }

    void appendAndNotifyOne(const QList<T>& list) {
        std::lock_guard<std::mutex> lk(mMutex);
        QList<T>::append(list);
        mCv.notify_one();
    }

    void notifyAll() {
        std::lock_guard<std::mutex> lk(mMutex);
        mCv.notify_all();
    }

    void notifyOne() {
        std::lock_guard<std::mutex> lk(mMutex);
        mCv.notify_one();
    }

    void waitNotEmpty() {
        std::unique_lock<std::mutex> lk(mMutex);
        while(QList<T>::isEmpty()) mCv.wait(lk);
    }

    void waitTakeFirst(T& t) {
        std::unique_lock<std::mutex> lk(mMutex);
        while(QList<T>::isEmpty()) mCv.wait(lk);
        t = QList<T>::takeFirst();
    }
private:
    std::mutex mMutex;
    std::condition_variable mCv;
};

#endif // QATOMICLIST_H
