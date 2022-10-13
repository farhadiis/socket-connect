#ifndef TRACKER_DISPATCH_QUEUE_H
#define TRACKER_DISPATCH_QUEUE_H

#include <thread>
#include <functional>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <queue>
#include <mutex>
#include <string>
#include <condition_variable>

typedef std::function<void(void)> fp_t;

class DispatchQueue {

private:
    std::string name_;
    std::mutex lock_;
    std::vector<std::thread> threads_;
    std::queue<fp_t> q_;
    std::condition_variable cv_;
    bool quit_ = false;

    void dispatch_thread_handler();

public:
    explicit DispatchQueue(std::string name, size_t thread_cnt = 1);
    ~DispatchQueue();

    // dispatch and copy
    void dispatch(const fp_t &op);
    // dispatch and move
    void dispatch(fp_t &&op);

    // Deleted operations
    DispatchQueue(const DispatchQueue &rhs) = delete;
    DispatchQueue &operator=(const DispatchQueue &rhs) = delete;
    DispatchQueue(DispatchQueue &&rhs) = delete;
    DispatchQueue &operator=(DispatchQueue &&rhs) = delete;

};

#endif //TRACKER_DISPATCH_QUEUE_H
