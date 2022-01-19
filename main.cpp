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
