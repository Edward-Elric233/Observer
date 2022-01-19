//
// Created by Administrator on 2022/1/17.
//

#ifndef OBSERVER_OBSERVER_H
#define OBSERVER_OBSERVER_H

#include <vector>
#include <mutex>
#include <memory>

//前置声明
class Observer;

//被观察者：data
class Observable : public std::enable_shared_from_this<Observable>{
    using ObserverList = std::vector<std::weak_ptr<Observer>>;
public:
    Observable(): observers_(new ObserverList) {}
    //通知函数
    void notify();
    //注册函数
    void register_(const std::weak_ptr<Observer> &observer);

    void unregister();

private:
    //观察者列表
    std::shared_ptr<ObserverList> observers_;
    mutable std::mutex mutex_;
};

//观察者：图表
class Observer {
public:
    ~Observer() {
        //派生类析构结束
        auto p = observable_.lock();
        if (p) {
            p->unregister();
        }
    }

    //纯虚函数:展示数据的方法
    virtual void update() = 0;
    std::weak_ptr<Observable> observable_;
};

#endif //OBSERVER_OBSERVER_H
