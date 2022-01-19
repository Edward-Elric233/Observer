#include <memory>
#include <vector>
#include <mutex>
#include <iostream>

using namespace std;

class Father {
public:
    explicit Father(const std::string &name)
            :name_(name) {}

    virtual void call() const {
        std::cout << "father's name:" << name_ << std::endl;
    }

    ~Father() {//不使用智能指针的话是必须的
        cout << "Father dtor is called\n";
    }

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

    ~Son() {
        cout << "Son dtor is called\n";
    }
private:
    const std::string name_;
};

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

int main() {
    //Father *p = new Son("Mike");
    shared_ptr<Father> p(new Son("Mike"));

    //p->call();

    return 0;
}