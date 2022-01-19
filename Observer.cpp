//
// Created by Administrator on 2022/1/17.
//

#include "Observer.h"

void Observable::notify() {
    decltype(observers_) observers;
    {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        observers = observers_;
    }

    for (auto iter = observers->begin(); iter != observers->end(); ++iter) {
        auto p = iter->lock();
        if (p) {
            p->update();
        }
    }
}

//Observable().register_(Observer());

void Observable::register_(const std::weak_ptr<Observer> &observer) {
    {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        if (!observers_.unique()) {
            observers_.reset(new ObserverList(*observers_));
        }
        observers_->push_back(observer);
    }
    auto p = observer.lock();
    if (p) {
        //p->observable = std::shared_ptr<Observable>(this);        //FATAL
        p->observable_ = shared_from_this();
    }
}

void Observable::unregister() {
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (!observers_.unique()) {
        observers_.reset(new ObserverList(*observers_));
    }
    for (auto iter = observers_->begin(); iter != observers_->end(); ++iter) {
        if (iter->expired()) {
            iter = observers_->erase(iter);
        }
    }
}