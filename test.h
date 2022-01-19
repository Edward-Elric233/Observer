/*
 * R X 收到朋友x的消息
 * S X 发送给朋友x的消息
 * W X 停留xs
 *
 * 你发送的唯一的消息是回复你已经收到的消息
 * 对于任意一个朋友的消息，你只回复一次
 * 你的朋友不发送一个后续消息直到你已经回复了以前的消息
 *
 * 一条消息的等待时间是：你收到它的时间到你回复它的时间
 *
 * 如果一个朋友X收到了他发送的所有消息的回复，那么他的等待时间就是所有消息的等待时间
 * 否则等待时间是-1

14
R 12
W 2
R 23
W 3
R 45
S 45
R 45
S 23
R 23
W 2
S 23
R 34
S 12
S 34

12 2 + 3 + 1 + 1 + 1 + 2 + 1 + 1 = 13


 */

#include <iostream>
#include <unordered_map>
#include <map>
#include <string>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int M, X, time = 0;
    bool flag = false;
    string cmd;
    map<int, int> msg_time;
    unordered_map<int, int> wait_time;

    cin >> M;
    while (M--) {
        cin >> cmd >> X;
        switch (cmd[0]) {
            case 'W':
                time += X;
                flag = true;
                break;
            case 'R':
                if (!flag) {
                    ++time;
                } else {
                    flag = false;
                }
                msg_time[X] = time;
                break;
            case 'S':
                if (!flag) {
                    ++time;
                } else {
                    flag = false;
                }
                wait_time[X] += time - msg_time[X];
                msg_time[X] = -1;
                break;
        }
    }
    for (auto iter : msg_time) {
        cout << iter.first << " ";
        if (~iter.second) cout << "-1";
        else cout << wait_time[iter.first];
        cout << "\n";
    }
    return 0;
}