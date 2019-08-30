// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef STDSELFREF_H
#define STDSELFREF_H
#include <QtCore>
#include <memory>
#include "../exceptions.h"
#include "eobject.h"

template <class T> class StdPointer;
template <class T> using stdsptr = std::shared_ptr<T>;
template <class T> using stdptr = StdPointer<T>;

class StdSelfRef {
    template <class T> friend class StdPointer;
    e_PROHIBIT_HEAP
public:
    virtual ~StdSelfRef();

    template <class T, typename... Args>
    static inline std::shared_ptr<T> sCreate(Args && ...args) {
        return (new T(std::forward<Args>(args)...))->template iniRef<T>();
    }

    template<class T>
    inline std::shared_ptr<T> ref() const {
        if(mThisWeak.expired()) RuntimeThrow("Not initialized, or already deleting");
        return std::static_pointer_cast<T>(std::shared_ptr<StdSelfRef>(mThisWeak));
    }
private:
    template<class T>
    inline std::weak_ptr<T> weakRef() const {
        if(mThisWeak.expired()) return std::weak_ptr<T>();
        return ref<T>();
    }

    template<class T>
    std::shared_ptr<T> iniRef() {
        if(!mThisWeak.expired()) RuntimeThrow("Shared pointer reinitialization");
        std::shared_ptr<T> thisRef = std::shared_ptr<T>(static_cast<T*>(this));
        this->mThisWeak = std::static_pointer_cast<StdSelfRef>(thisRef);
        return thisRef;
    }
private:
    std::weak_ptr<StdSelfRef> mThisWeak;
};
#endif // STDSELFREF_H
