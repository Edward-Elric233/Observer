# 线程安全的Observer模式
## 绪论
陈硕： 《C++多线程服务端编程》

## 什么是Observer模式
面向对象设计模式种的一种：
- Singleton模式 
- Factory模式...
- Observer模式：对象之间发送通知



### 例子

数据Data
- update()

可视化：折线图、柱状图、单元格
- display()

### 多态

```cpp
#include <string>
#include <iostream>

class Father {
public:
    explicit Father(const std::string &name)
    :name_(name) {}

    virtual void call() const {
        std::cout << "father's name:" << name_ << std::endl;
    }
    
    virtual ~Father() {}		//不使用智能指针的话是必须的

private:
    const std::string name_;
};

class Son : public Father {
public:
    explicit Son(const std::string &name)
    : Father(name + "'s father")
    , name_(name) {}

    void call() const override {
        std::cout << "son's name:" << name_ << std::endl;
    }
private:
    const std::string name_;
};


int main() {
    //Father *p = new Son("Mike");
    const Father &father = Son("Mike");
    //p->call();
    father.call();
    
    delete p;
    return 0;
}

```



#### 多态发生的条件

1. 使用一个指向派生类的**基类指针或者引用**
2. 访问**虚函数**



### **Observer 0.1**

**代码实现**

```cpp
//前置声明
class Observer;

//被观察者：data
class Observable {
public:
    //通知函数
    void notify();
    //注册函数
    void register_(Observer *observer);

private:
    //观察者列表
    std::vector<Observer *> observers;
};

//观察者：图表
class Observer {
public:
    //纯虚函数:展示数据的方法
    virtual void update() = 0;
};
void Observable::notify() {
    for (auto observer : observers) {
        observer->update();
    }
}

void Observable::register_(Observer *observer) {
    observers.push_back(observer);
}

```

**测试代码**

```cpp
#include <iostream>
#include <vector>
#include "Observer.h"

using namespace std;

class Data : public Observable {
public:
    explicit Data(const vector<int> &data):data_(data) {}
    explicit Data(vector<int> &&data):data_(data) {}

    void update(const vector<int> &data) {
        data_ = data;
        notify();
    }
    void update(vector<int> &&data) {
        data_ = data;
        notify();
    }
    vector<int> data_;
};

class Observer1 : public Observer {
public:
    Observer1(const Data &data):data_(data) {}

    void update() override {
        cout << "Observer1:";
        for (auto x : data_.data_) {
            cout << " " << x;
        }
        cout << "\n";
    }

private:
    const Data &data_;
};

class Observer2 : public Observer {
public:
    Observer2(const Data &data):data_(data) {}

    void update() override {
        cout << "Observer2:";
        for (auto x : data_.data_) {
            cout << " " << x;
        }
        cout << "\n";
    }

private:
    const Data &data_;
};

class Observer3 : public Observer {
public:
    Observer3(const Data &data):data_(data) {}

    void update() override {
        cout << "Observer3:";
        for (auto x : data_.data_) {
            cout << " " << x;
        }
        cout << "\n";
    }

private:
    const Data &data_;
};

int main() {
    Data data(vector<int>({1,2,3}));

    Observer1 observer1(data);
    Observer2 observer2(data);
    Observer3 observer3(data);

    data.register_(&observer1);
    data.register_(&observer2);
    data.register_(&observer3);

    int x;
    while (cin >> x) {
        auto arr = data.data_;
        arr.push_back(x);
        data.update(arr);
    }

    return 0;
}

```

## 改进

### 互斥锁操作

```cpp
#include <pthread.h>

class Mutex {
public:
    Mutex():mutex_(pthread_mutex_init(&mutex_, nullptr)) {}
    ~Mutex() {
        pthread_mutex_destroy(&mutex_);
    }
private:
    void lock() {
        pthread_mutex_lock(&mutex_);
    }
    void unlock() {
        pthread_mutex_unlock(&mutex_);
    }
    friend class MutexGuard;
    pthread_mutex_t mutex_;
};

class MutexGuard {
    explicit MutexGuard(Mutex &mutex)
    : mutex_(mutex) {
        mutex_.lock();
    }
    
    ~MutexGuard() {
        mutex_.unlock();
    }
    
private:
    Mutex &mutex_;
};
```



- `MutexGuard`是一个栈上对象
- `Mutex`是一个类的成员变量

### **Observer 0.2**

**0.1的问题**

- 不确定`observer`是否存活

**解决方案**

- `observer`在析构函数中解注册

**代码实现**

```cpp
#include <vector>
#include <mutex>

//前置声明
class Observer;

//被观察者：data
class Observable {
public:
    //通知函数
    void notify();
    //注册函数
    void register_(Observer *observer);

    void unregister(Observer *observer);

private:
    //观察者列表
    std::vector<Observer *> observers;
    mutable std::mutex mutex_;
};

//观察者：图表
class Observer {
public:
    ~Observer() {
        observable->unregister(this);
    }

    //纯虚函数:展示数据的方法
    virtual void update() = 0;
    Observable *observable;
};



void Observable::notify() {
    std::lock_guard<std::mutex> lockGuard(mutex_);
    for (auto observer : observers) {
        observer->update();
    }
}

void Observable::register_(Observer *observer) {
    std::lock_guard<std::mutex> lockGuard(mutex_);
    observers.push_back(observer);
    observer->observable = this;
}

void Observable::unregister(Observer *observer) {
    std::lock_guard<std::mutex> lockGuard(mutex_);
    for (auto iter = observers.begin(); iter != observers.end(); ++iter) {
        if (*iter == observer) {
            iter = observers.erase(iter);
            break;
        }
    }
}
```

### Observer 1.0

**0.2的问题**

- `Observable`指针失效（或许可以通过将`Observable`设置成单例解决）
- 在`Observer`的析构函数中解注册没有意义：等到`Observer`析构的时候派生类已经析构完毕，可能会在`unregister`之前`Observable`调用`notify()`方法，通知这个析构到一半的对象，导致程序崩溃。
- 广泛加锁，效率低下
- 如果`update`函数调用了`register_`函数导致程序崩溃

**解决方案**

- 智能指针

**裸指针存在的问题**

没有办法通过指针判断所指向对象的状态

### **智能指针**

头文件：`<memory>`

- `shared_ptr`：带引用计数的智能指针
  - `explicit shared_ptr<T>(P *)`  `shared_ptr<T>(const shared_ptr<P> &p)`
  - `shared_ptr<T> ptr = p`是错误的
- `weak_ptr` ：指向`shared_ptr`，解决循环引用

​		p1->A  :p -> B

​		p2->B  :p |-> A

- `unique_ptr`：实现RAII
- 只在第一次创建的时候才能使用裸指针
- 在创建时捕获析构函数，析构函数不用是虚析构的

**常见操作**

```cpp
    //shared_ptr<Data> pData(new Data(vector<int>({1,2,3})));
    shared_ptr<Data> pData = make_shared<Data>(vector<int>({1,2,3}));
    pData.get();
    
    weak_ptr<Data> wkData = pData;
    shared_ptr<Data> pData1;
    if (pData1 = wkData.lock()) {
        pData1->update(vector<int>({1,2,3,4}));
    }
    if (wkData.expired()) {
        
    }

```

**代码实现**

```cpp
#include <vector>
#include <mutex>
#include <memory>

//前置声明
class Observer;

//被观察者：data
class Observable {
public:
    //通知函数
    void notify();
    //注册函数
    void register_(const std::weak_ptr<Observer> &observer);

    //void unregister(Observer *observer);

private:
    //观察者列表
    std::vector<std::weak_ptr<Observer>> observers;
    mutable std::mutex mutex_;
};

//观察者：图表
class Observer {
public:
    virtual ~Observer() {
        //派生类析构结束
        //observable->unregister(this);
    }

    //纯虚函数:展示数据的方法
    virtual void update() = 0;
    //Observable *observable;
};

void Observable::notify() {
    std::lock_guard<std::mutex> lockGuard(mutex_);

    for (auto iter = observers.begin(); iter != observers.end(); ++iter) {
        auto p = iter->lock();
        if (p) {
            p->update();
        } else {
            //weak_ptr失效，所指向的对象已经析构了
            //删除操作
            iter = observers.erase(iter);
        }
    }
}

//Observable().register_(Observer());

void Observable::register_(const std::weak_ptr<Observer> &observer) {
    std::lock_guard<std::mutex> lockGuard(mutex_);
    observers.push_back(observer);
}

/*
void Observable::unregister(Observer *observer) {
    std::lock_guard<std::mutex> lockGuard(mutex_);
    for (auto iter = observers.begin(); iter != observers.end(); ++iter) {
        if (*iter == observer) {
            iter = observers.erase(iter);
            break;
        }
    }
}
 */
```

**测试代码**

```cpp
#include <iostream>
#include <vector>
#include <memory>
#include "Observer.h"

using namespace std;

class Data : public Observable {
public:
    explicit Data(const vector<int> &data):data_(data) {}
    explicit Data(vector<int> &&data):data_(data) {}

    void update(const vector<int> &data) {
        data_ = data;
        notify();
    }
    void update(vector<int> &&data) {
        data_ = data;
        notify();
    }
    vector<int> data_;
};

class Observer1 : public Observer {
public:
    Observer1(const Data &data):data_(data) {}

    void update() override {
        cout << "Observer1:";
        for (auto x : data_.data_) {
            cout << " " << x;
        }
        cout << "\n";
    }

private:
    const Data &data_;
};

class Observer2 : public Observer {
public:
    Observer2(const Data &data):data_(data) {}

    void update() override {
        cout << "Observer2:";
        for (auto x : data_.data_) {
            cout << " " << x;
        }
        cout << "\n";
    }

private:
    const Data &data_;
};

class Observer3 : public Observer {
public:
    Observer3(const Data &data):data_(data) {}

    void update() override {
        cout << "Observer3:";
        for (auto x : data_.data_) {
            cout << " " << x;
        }
        cout << "\n";
    }

private:
    const Data &data_;
};

int main() {
    shared_ptr<Data> data = make_shared<Data>(vector<int>({1,2,3}));

    shared_ptr<Observer1> observer1 = make_shared<Observer1>(*data);
    shared_ptr<Observer2> observer2 = make_shared<Observer2>(*data);
    shared_ptr<Observer3> observer3 = make_shared<Observer3>(*data);

    data->register_(observer1);
    data->register_(observer2);
    data->register_(observer3);

    int x;
    while (cin >> x) {
        auto arr = data->data_;
        arr.push_back(x);
        data->update(arr);
    }

    return 0;
}

```

### Observer 1.1

**1.0存在的问题**

- 观察者`Observer`析构的时候不会主动地去被观察者`Observable`中解注册自己，导致轻微的内存泄漏

**解决方案**

`observer`在析构函数中解注册

**代码实现**

```cpp
#include <vector>
#include <mutex>
#include <memory>

//前置声明
class Observer;

//被观察者：data
class Observable : public std::enable_shared_from_this<Observable>{
public:
    //通知函数
    void notify();
    //注册函数
    void register_(const std::weak_ptr<Observer> &observer);

    void unregister();

private:
    //观察者列表
    std::vector<std::weak_ptr<Observer>> observers;
    mutable std::mutex mutex_;
};

//观察者：图表
class Observer {
public:
    ~Observer() {
        //派生类析构结束
        auto p = observable.lock();
        if (p) {
            p->unregister();
        }
    }

    //纯虚函数:展示数据的方法
    virtual void update() = 0;
    std::weak_ptr<Observable> observable;
};
void Observable::notify() {
    std::lock_guard<std::mutex> lockGuard(mutex_);

    for (auto iter = observers.begin(); iter != observers.end(); ++iter) {
        auto p = iter->lock();
        if (p) {
            p->update();
        } else {
            //weak_ptr失效，所指向的对象已经析构了
            //删除操作
            iter = observers.erase(iter);
        }
    }
}

//Observable().register_(Observer());

void Observable::register_(const std::weak_ptr<Observer> &observer) {
    std::lock_guard<std::mutex> lockGuard(mutex_);
    observers.push_back(observer);
    auto p = observer.lock();
    if (p) {
        //p->observable = std::shared_ptr<Observable>(this);        //FATAL
        p->observable = shared_from_this();
    }
}

void Observable::unregister() {
    std::lock_guard<std::mutex> lockGuard(mutex_);
    for (auto iter = observers.begin(); iter != observers.end(); ++iter) {
        if (iter->expired()) {
            iter = observers.erase(iter);
        }
    }
}
```

### Observer 2.0

**1.1存在的问题**

- 广泛加锁，效率低下
- 如果`update`函数调用了`register_`函数导致程序崩溃

**解决方案**

使用智能指针加互斥锁模拟的读写锁

不推荐在程序中直接使用读写锁：

- 读写锁的性能不如互斥锁
- 读锁中不小心修改了数据导致程序崩溃
- 读锁是可重入的，写锁是不可重入的。读锁在重入的过程中被写锁抢占可能会导致死锁

**COW代码实现**



```cpp
class COW { //copy on write
public:
    void read() const {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        for (auto x : arr_) {
            cout << x << " ";
        }
        cout << "\n";
    }
    void append(int x) {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        arr_.push_back(x);
    }
private:
    std::vector<int> arr_;
    mutable std::mutex mutex_;
};
```

```cpp
class COW { //copy on write
public:
    COW(): arr_(new std::vector<int>()) {}
    void read() const {
        decltype(arr_) arr;
        {
            std::lock_guard<std::mutex> lockGuard(mutex_);
            arr = arr_;
        }
        for (auto x : *arr) {
            cout << x << " ";
        }
        cout << "\n";
    }
    void append(int x) {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        if (!arr_.unique()) {
            //有其他地方正在使用
            arr_.reset(new std::vector<int>(*arr_));
        }
        arr_->push_back(x);
    }
private:
    std::shared_ptr<std::vector<int>> arr_;
    mutable std::mutex mutex_;
};
```

**代码实现**

```cpp
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
```

